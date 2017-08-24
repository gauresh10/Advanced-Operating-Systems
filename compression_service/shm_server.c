#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <mqueue.h>
#include "snappy.h"
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "include.h"

#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include"shm_server.h"

void sigint_handler(int sig)
{
    write(0, "Ahhh! Ctrl+C!\n", 14);

    struct shmid_ds *shmid_ds;
    int rtrn =0 ;
    if ((rtrn = shmctl(shm_sz_id, IPC_RMID, shmid_ds)) == -1) {
        perror("shmctl: shmctl failed");
        exit(1);
    }
    printf("\nremoved the shm_size segment\n");

    exit(0);
}

main(int argc, char **argv)
{

    if(argc<2)
    {
        perror("Minimum 2 parameters are needed : ./servermodule <CAP_SIZE>");
        exit(0);
    }

    //signal handler

    void sigint_handler(int sig); /* prototype */
    struct sigaction sa_sig;

    sa_sig.sa_handler = sigint_handler;
    sa_sig.sa_flags = 0; // or SA_RESTART
    sigemptyset(&sa_sig.sa_mask);

    if (sigaction(SIGINT, &sa_sig, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }



    // shared mem for size

    shm_data_struct_t* shm_ptr;
    //int shm_sz_id;
    key_t sh_sz_key = 2300;
    // long *s_sz;


    long max_size_of_shm = atoi(argv[1]);
    //fprintf(stderr, "\n %ld max size \n",max_size_of_shm);

    if ((shm_sz_id = shmget(sh_sz_key, sizeof(shm_data_struct_t), IPC_CREAT| 0666)) < 0) {
        perror("shmget");
        exit(1);
    }

    //fprintf(stderr, "\nshmget of size server called and shmid returned is %d\n", shm_sz_id );

    if ((shm_ptr = shmat(shm_sz_id, NULL, 0)) == (char *) -1) {
        perror("shmat");
        exit(1);
    }

    //shared across processes : use lock
    pthread_mutexattr_t m_attr;
    pthread_mutexattr_setpshared(&m_attr,PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&(shm_ptr->mutex),&m_attr);
    shm_ptr->s_sz = max_size_of_shm;


    fprintf(stderr, "\n Server Initialized with Cap Size of %ld \n",shm_ptr->s_sz);


    fprintf(stderr, "\n SAFE TO Start Clients \n");

//shared memory parameters
    int shmid;
    key_t key;
    char *shm,*s;

//snappy c
    struct snappy_env env;

//queue
    char clname[MAX_CLIENT_NAME];
    long size;
    mqd_t qd_server, qd_client; // queue descriptors

    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MESSAGES;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;
    snappy_init_env(&env);

    mqueue mq;
    mqueue rq;



//queue operation
    if ((qd_server = mq_open (SERVER_QUEUE_NAME, O_RDONLY | O_CREAT, QUEUE_PERMISSIONS, &attr)) == -1)
    { perror ("Server: mq_open (server)"); exit (1);}

    fprintf(stderr, "\nserver queue opened and waiting for client messages (server ID)%d\n", qd_server);

    while(1){

        printf("\n \n");
        printf("\n \n");
        printf("\n \n");
        printf("\n \n");
        //unsigned* buff_prio=(unsigned*)calloc(0,sizeof(unsigned));
        if (mq_receive (qd_server, (char *)&rq, attr.mq_msgsize, NULL) == -1)
        { perror ("Server: mq_receive");exit (1);}
        //fprintf (stderr,"buffer priority %u\n",*buff_prio);
        strcpy(&clname,rq.message);
        key=(int)rq.key;
        size = rq.size;

        printf ("Server: message received from Client %s with key %ld and size of file %ld.\n",clname, key, size);


//shared memory
        if ((shmid = shmget(key, size, 0666)) < 0) {
            perror("shmget");
            exit(1);
        }
       // fprintf(stderr, "\nshmget of server called and shmid returned is %d\n", shmid);

        if ((shm = shmat(shmid, NULL, 0)) == (char *) -1) {
            perror("shmat");
            exit(1);
        }
        //fprintf(stderr, "\nshmat of server  called \n");
        s=shm;
//lock the shared memory
        int rtrn;
        struct shmid_ds *shmid_ds;

        fprintf(stderr, "\nserver is about to compress and write data on the shared memeory \n");

        //compression and put data in shared memory
        char compressed[size];
        size_t length =0;
        size_t ulength =0;
        int status;
        status = snappy_compress(&env, shm, size, compressed, &length);
        fprintf(stderr, "\nstatus of compression is %d with length of compressed file : %d\n", status, length);

        int i =0;
        for(i = 0 ; i < length ; i ++)
        {
            *shm++ = compressed[i];

        }
        *shm=NULL;


        fprintf(stderr, "\nserver is done writing data on the shared memory \n");


//send to client
        if ((qd_client = mq_open (clname, O_WRONLY)) == 1)
        {
            perror ("Server: Not able to open client queue");

        }

        rq.size =length;
        printf ("\nsend message to client %s with key %ld and size of compressed file %ld.\n",clname, key, rq.size);

        if (mq_send (qd_client, (char *)&rq, sizeof(rq), 0) == -1)
        {
            perror ("Server: Not able to send message to client");
            //continue;
        }
        int detach=0;
        if(detach = shmdt(s)==-1)
        {
            perror("cannot detach from server");
        }
    }
//exit
    fprintf(stderr, "\nserver waits till client has read the data\n");
    fprintf(stderr, "\nserver exits\n");

    exit(0);
}