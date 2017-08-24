#include"Client_API.h"
int BLOCK_API(char *filename, int client_identifier, char* result_data)
{

    // shared mem for size
    int shm_sz_id;
    key_t sh_sz_key = 2300;
    long max_size_shm;
    shm_data_struct_t* shm_ptr;
    if ((shm_sz_id = shmget(sh_sz_key, sizeof(shm_data_struct_t),0666)) < 0) {
        perror("shmget");
        exit(1);
    }

    // fprintf(stderr, "\nshmget of size server called and shmid returned is %d\n", shm_sz_id );

    if ((shm_ptr = shmat(shm_sz_id, NULL, 0)) == (char *) -1) {
        perror("shmat");
        exit(1);
    }


//shared memory parameter
    int shmid;
    key_t key;
    char *shm,*s;
    //printf("\nFIle name is %s\n",filename);
//message queue parameter ends here
    char client_name[100] = "/client";
    char str[15];
    sprintf(str, "%d", client_identifier);
    //unique client name
    strcat(client_name, str);

    mqd_t qd_server, qd_client;
    mqueue mq;
    strcpy(&mq.message,client_name);


    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MESSAGES;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;


    FILE *fp;
    struct stat buf;
    char save_file_name[30];

//message queue parameter ends here





//open the queues
    if ((qd_client = mq_open ( client_name, O_RDONLY | O_CREAT, QUEUE_PERMISSIONS, &attr)) == -1)
    {
        perror ("Client: mq_open (client)");
        exit (1);
    }
    fprintf(stderr, "\nclient queue opened with %d \n", qd_client);

    if ((qd_server = mq_open (SERVER_QUEUE_NAME, O_WRONLY)) == -1)
    {
        perror ("Client: mq_open (server)");
        exit (1);
    }
    fprintf(stderr, "\nserver queue opened with %d \n", qd_server);

//for loop MAIN=FOR LOOP
    char *token = strtok(filename, ",");

    printf("\nclient name is %s\n",client_name);

    int iter=0;

    while (token != NULL)
    {
        printf("\n \n");
        printf("\n \n");
        printf("\n \n");
        printf("\n \n");
        //printf("%s\n", token);
        srand(time(NULL));
        unsigned int unique_number = (unsigned int)rand();
        //printf("\nunique number is %u\n",unique_number);

        if((key=ftok(token,unique_number))==-1)
        {perror("cannot create key");exit(1);}
        //printf("\nkey number is %d\n",key);

        fp=fopen(token,"r");
        stat(token,&buf);
        long size = buf.st_size;
        printf("\n Size of the file is %ld \n",size);
        //file handler ends here


        mq.key=key;
        mq.size= size;


        while(size > shm_ptr->s_sz){
            perror("waiting/spining now");
        };

        pthread_mutex_lock(&shm_ptr->mutex);
        shm_ptr->s_sz -= size;
        fprintf(stderr,"\nSend: current size of the shared memory is %lu of process id %d\n"
                ,shm_ptr->s_sz, client_identifier);
        pthread_mutex_unlock(&shm_ptr->mutex);

//shared memory
        if ((shmid = shmget(mq.key, mq.size, IPC_CREAT | 0666)) < 0) {
            perror("shmget");
            exit(1);
        }
        //fprintf(stderr, "\nclient created with id %d \n", shmid);

        if ((shm = shmat(shmid, NULL, 0)) == (char *) -1) {
            perror("shmat");
            exit(1);
        }
        //fprintf(stderr, "\nclient mapped to the id %d\n", shmid);
        struct shmid_ds *shmid_ds;


        s=shm;
        //read data char by char
        do
        {
            *shm++=fgetc(fp);

            if(feof(fp))
                break;
            //printf("%c",*shm);
        }while(1);


        fclose(fp);

// send message to server
        if (mq_send (qd_server, (char *)&mq , sizeof(mq)  , 0) == -1)
        { perror ("Client: Not able to send message to server"); }

        char final_file_name[100];
        sprintf(final_file_name,"Blockcompressed%d",rand()%256);
        //strcat(&final_file_name,&save_file_name);
        //iter++;

//receive from server
        mqueue rq;
        while(1)
        {
            if (mq_receive (qd_client, (char *)&rq, attr.mq_msgsize, NULL) == -1)
            { perror ("Client: mq_receive");  }

                // if(key==rq.key)
            else{
                printf("\nReceived Compressed File with key %d and size %lu\n",rq.key,rq.size);
                FILE *fp;

                fp=fopen(final_file_name, "w+");
                char *saa;
                for(saa = s; *saa != NULL; saa++)
                {
                    fputc(*saa,fp);
                }
                fclose(fp);

                
                // uncompress
                char uncompressed[size];

                size_t ulength=0;
                int status2 = snappy_uncompress(s,rq.size,uncompressed);
                snappy_uncompressed_length(s, rq.size, &ulength);
                //fprintf(stderr, "\nuncompressed status is %d with uncompressed file length of %d\n", status2, ulength);

                char *sa;
                for(sa = uncompressed; *sa != NULL; sa++)
                {
                    putchar(*sa);
                }

                putchar('\n');
                

                sleep(sleep_time);

                //REMOVE THE SEGMENT
                int detach=0;
                if(detach = shmdt(s)==-1)
                {
                    perror("cannot detach from client");
                }

                int rtrn=0;
                if ((rtrn = shmctl(shmid, IPC_RMID, shmid_ds)) == -1)
                {perror("shmctl: shmctl failed");exit(1);}

                printf("\nremoved the segment\n");

                pthread_mutex_lock(&shm_ptr->mutex);
                shm_ptr->s_sz += size;
                fprintf(stderr,"\nReceive: current size of the shared memory is %lu of process id %d\n"
                        ,shm_ptr->s_sz, client_identifier);
                pthread_mutex_unlock(&shm_ptr->mutex);
                break;
            }

        }
        token = strtok(NULL, ",");

    }
    return 1;

}




int NONBLOCK_API(char *filename,  int client_identifier, int success_send) {


    // shared mem for size
    int shm_sz_id;
    key_t sh_sz_key = 2300;
    long max_size_shm;
    shm_data_struct_t *shm_ptr;
    if ((shm_sz_id = shmget(sh_sz_key, sizeof(shm_data_struct_t), 0666)) < 0) {
        perror("shmget");
        exit(1);
    }

    // fprintf(stderr, "\nshmget of size server called and shmid returned is %d\n", shm_sz_id);

    if ((shm_ptr = shmat(shm_sz_id, NULL, 0)) == (char *) -1) {
        perror("shmat");
        exit(1);
    }



//shared memory parameter
    int shmid;
    key_t key;
    char *shm, *s;

//shared memory parameter ends here



//message queue parameter ends here
    char client_name[] = "/client";
    char str[100];
    sprintf(str, "%d", client_identifier);
    //unique client name
    strcat(client_name, str);

    mqd_t qd_server;
    mqueue mq;
    strcpy(&mq.message, client_name);


    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MESSAGES;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;


    FILE *fp;
    struct stat buf;

//message queue parameter ends here


//open the queues
    if ((qd_clienta = mq_open(client_name, O_RDONLY | O_CREAT, QUEUE_PERMISSIONS, &attr)) == -1) {
        perror("Client: mq_open (client)");
        exit(1);
    }
    fprintf(stderr, "\nclient queue opened with %d \n", qd_clienta);

    if ((qd_server = mq_open(SERVER_QUEUE_NAME, O_WRONLY)) == -1) {
        perror("Client: mq_open (server)");
        exit(1);
    }
    fprintf(stderr, "\nserver queue opened with %d \n", qd_server);



    // long size = 0;
    char new_file_names[strlen(filename)];
    strcpy(new_file_names,filename);
    //know the number of messages
    char *tokenp = strtok(new_file_names, ",");

    int iter = 0;
    long size_total=0;
    while (tokenp != NULL) {
        iter++;
        //printf("token %s",tokenp);

        fp=fopen(tokenp,"r");
        stat(tokenp,&buf);
        size_total += buf.st_size;

        tokenp = strtok(NULL, ",");

    }
    printf("total files are: %d and total file size is %lu",iter,size_total);
    total_request=iter;


//bc array change
    size_array = (long *) calloc(iter, sizeof(long));

    shm_ptr_array = (long *) calloc(iter, sizeof(long));

    shm_shmid_array = (long *) calloc(iter, sizeof(long));

    shm_key_array = (key_t *) calloc(iter, sizeof(key_t));

    int rtrn = 0;
    struct shmid_ds *shmid_ds;


    char *tokena = strtok(filename, ",");

    int loop_val=0;
    srand(time(NULL));

    //chan here
    if(size_total > shm_ptr->s_sz){
        perror("CLIENT cannot be Processed: Server loaded. Please try after sometime ");
        return 0;
    }


    //  while(size_total> shm_ptr->s_sz){
//        perror("spining now");
    //};

    int temp_request=0;

    while (tokena != NULL) {

        printf("\n \n");
        printf("\n \n");
        printf("\n \n");
        printf("\n \n");
        //printf("\n%s\n", tokena);


        unsigned int unique_number = (unsigned int)rand();

        printf("\nclient name is %s\n",client_name);
        //printf("\nunique number is %u\n",unique_number);


        if((key=ftok(tokena,unique_number))==-1)
        {perror("cannot create key");exit(1);}

        //printf("\nkey number is %d\n",key);

        fp=fopen(tokena,"r");
//bc
        stat(tokena,&buf);
        long size = buf.st_size;



        size_array[loop_val]=size;

        printf("\n Size of the file is %ld \n",size_array[loop_val]);
        //file handler ends here

        mq.key = key;
        mq.size = size;
        shm_key_array[loop_val] = key;

        //fprintf(stderr, "\nkey is  %d \n", shm_key_array[loop_val]);





        //chan here


        pthread_mutex_lock(&shm_ptr->mutex);
        shm_ptr->s_sz -= size;
        fprintf(stderr,"\nsend: current size of the shared memory is %lu of process id %d\n"
                ,shm_ptr->s_sz, client_identifier);

        pthread_mutex_unlock(&shm_ptr->mutex);





//shared memory
        if ((shmid = shmget(mq.key, mq.size, IPC_CREAT | 0666)) < 0) {
            perror("shmget");
            exit(1);
        }
        shm_shmid_array[loop_val] = shmid;

        // fprintf(stderr, "\nshmid %d \n", shm_shmid_array[loop_val]);

        if ((shm = shmat(shmid, NULL, 0)) == (char *) -1) {
            perror("shmat");
            exit(1);
        }
        // fprintf(stderr, "\nclient mapped to the id %d\n", shmid);


        struct shmid_ds *shmid_ds;
        s = shm;

        shm_ptr_array[loop_val] = s;
        //fprintf(stderr, "\nshm pointer is  %d \n", shm_ptr_array[loop_val]);
        //read data char by char
        do {
            *shm++ = fgetc(fp);

            if (feof(fp))
                break;
            printf("%c", *shm);
        } while (1);


        fclose(fp);



// send message to server
        if (mq_send(qd_server, (char *) &mq, sizeof(mq), 0) == -1) {
            perror("Client: Not able to send message to server");
        }

        tokena = strtok(NULL, ",");
        loop_val++;
    }



    //printf("\nloop_val %d\n", loop_val);
    return 1;
}





int WAIT_NONBLOCK_RESULT() {

    //shared memory sz
    // shared mem for size
    int shm_sz_id;
    key_t sh_sz_key = 2300;
    long max_size_shm;
    shm_data_struct_t* shm_ptr;
    if ((shm_sz_id = shmget(sh_sz_key, sizeof(shm_data_struct_t),0666)) < 0) {
        perror("shmget");
        exit(1);
    }

    //fprintf(stderr, "\nshmget of size server called and shmid returned is %d\n", shm_sz_id );

    if ((shm_ptr = shmat(shm_sz_id, NULL, 0)) == (char *) -1) {
        perror("shmat");
        exit(1);
    }





    int num_response = 0;
    struct shmid_ds *shmid_ds;
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MESSAGES;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;




    char save_file_name[100];
    sprintf(save_file_name,"NonBlockcompressed%d.txt",rand()%256);
    //strcat(&final_file_name,&save_file_name);


//receive from server
    mqueue rq;
    //
    while (1) {

        if (mq_receive(qd_clienta, (char *) &rq, attr.mq_msgsize, NULL) == -1)
        { perror("Client: mq_receive"); }
        else {
            printf("\n \n");
            printf("\n \n");
            printf("\n \n");
            printf("\n \n");
            printf("\nReceived Compressed File with key %d and size %lu\n",rq.key,rq.size);
            //printf("\nnumresponse is %d\n", num_response);
            // if (shm_key_array[num_response] == rq.key)
            {



                FILE *fp;

                fp=fopen(save_file_name, "w+");
                char *saa;
                for(saa = shm_ptr_array[num_response]; *saa != NULL; saa++)
                {
                    fputc(*saa,fp);
                }
                fclose(fp);


                char uncompressed[size_array[num_response]];

                size_t ulength = 0;
                int status2 = snappy_uncompress(shm_ptr_array[num_response], rq.size, uncompressed);
                snappy_uncompressed_length(shm_ptr_array[num_response], rq.size, &ulength);
                fprintf(stderr, "\nuncompressed status is %d with uncompressed file length of %d\n", status2, ulength);

                char *sa;
                for (sa = uncompressed; *sa != NULL; sa++) {
                    putchar(*sa);
                }

                putchar('\n');

                //REMOVE THE SEGMENT
                int detach = 0;
                if (detach = shmdt(shm_ptr_array[num_response]) == -1) {
                    perror("cannot detach from client");
                }
                int rtrn =0 ;
                if ((rtrn = shmctl(shm_shmid_array[num_response], IPC_RMID, shmid_ds)) == -1) {
                    perror("shmctl: shmctl failed");
                    exit(1);
                }
                printf("\nremoved the segment\n");

                pthread_mutex_lock(&shm_ptr->mutex);
                shm_ptr->s_sz += size_array[num_response];
                fprintf(stderr,"\nReceive: current size of the shared memory is %lu of process id %d\n"
                        ,shm_ptr->s_sz, "lala");

                pthread_mutex_unlock(&shm_ptr->mutex);

            }
            num_response++;

        }


        if (total_request  == num_response) {
            perror("No of Requests and Responses Match: Back to APP");
            //return 1;
            //exit(0);
            break;
        }
    }

    free(size_array);
    free(shm_ptr_array);
    free(shm_shmid_array);
    free(shm_key_array);
    return 1;

}



