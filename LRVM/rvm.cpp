#include "rvm.h"

//#define debug 1

static int transac_complete = 0;
static  int transid =0;
static int map_count = 0;
struct File_Info file_info[num_log_file];
struct log_t redo_log[num_log_file];
struct log_t undo_log[num_log_file];
struct is_mapped seg_mapped[num_log_file];
static int count1 =0;
static int num_mapped_file=0;
static int trans_no = 0;
static int prev_trans_no = 0;

using namespace std;

void verbose(int enable){
    if(enable==1) {
#define debug 1
    }
}
rvm_t rvm_init(char* directory)
{
    cout<<endl;
    cout<<"RVM INIT"<<endl;
    rvm_t t_rvm;
    struct stat st = {0};
    int len = strlen(directory);
    t_rvm.direc_name =(char*) malloc(len);
    strcpy(t_rvm.direc_name, directory);
    if (stat(t_rvm.direc_name, &st) == -1) {
        mkdir(t_rvm.direc_name, 0777);
        cout<<" New Directory Created "<<endl;
    }
    else
    {
        cout<<" Directory Already present "<<endl;
    }
    return t_rvm;
}

void rvm_destroy(rvm_t t_rvm, char* filename)
{
    cout<<endl;
    cout<<"RVM DESTROY"<<endl;
    int len = strlen(filename);
    char* file_name =(char*) malloc(len);
    strcpy(file_name, filename);
    char path[strlen(t_rvm.direc_name)+strlen(file_name)+5];
    sprintf(path,"%s/%s.txt",t_rvm.direc_name,file_name);
    int ret = remove(path);
    if(ret == 0)
    {
        cout<<" Testseg File Deleted "<<endl;
    }
    else
    {
        printf("Unable to Delete Testseg: File not present on DISK\n");
    }
    free(file_name);
}

void* rvm_map(rvm_t t_rvm, const char* segname, int size_to_create)
{
    cout<<endl;
    cout<<"RVM MAP"<<endl;
    void* map;

    char rvm_dir_format[strlen(t_rvm.direc_name)+2];
    strcpy(rvm_dir_format,"./");
    strcat(rvm_dir_format,t_rvm.direc_name);

    char* token;
    bool flag = false;
    char file_found[20];
    //copy file name
    int len = strlen(segname);
    char *file_name = (char*) malloc(len);
    strcpy(file_name, segname);
    //create the file
    char path[strlen(t_rvm.direc_name)+strlen(file_name)+5];
    sprintf(path,"%s/%s.txt",t_rvm.direc_name,segname);
    //check if the file exists
    struct stat   buffer;
    if(stat (path, &buffer) == 0)
    {
        size_t size = buffer.st_size;
        // File already exists, size to create is larger than existing size-> extend

        if (transac_complete==0 && transid !=0)
        {
            //changed
            char* token;
            string entry;
            char file_log[20];
            char file_original[20];
            sprintf(token,"%s",file_name);
            sprintf(file_log, "rvm_segments/%s$log.txt", token);

            sprintf(file_original, "rvm_segments/%s.txt", token);
            fstream fdd(file_log);
            fstream fp(file_original);

            while(getline(fdd,entry))
            {
                int offset;
                int size;
                string data;
                stringstream ss(entry);
                ss >> offset >> size;
                getline(ss,data,'\n');
                data.erase(remove(data.begin(),data.end(),'\t'),data.end());

                fp.seekp(offset);
                fp.write(data.c_str(),data.size());


            }
            fdd.close();
            fp.close();
            int ret = remove(file_log);
            if(ret == 0)
            {
                printf("File Deleted\n"); // file exists
            }
            else
            {
                printf("Unable to Delete: File not present\n");
            }
            //changed
            //  cout<<"path is "<<path<<endl;
            transid++;
            FILE* fd = fopen(path, "w+");
            truncate(path, size_to_create);
            strcpy(file_info[map_count].filename, file_name);
            //map the file to memory
            map = malloc(size_to_create);
            file_info[map_count].map_addr = map;
            seg_mapped[map_count].segs=map;
            num_mapped_file++;
#ifdef debug
            printf("\n New Map 2:: Map address: %d, with file name: %s, and file size: %d \n",map,file_info[map_count].filename,size_to_create);
#endif
            fclose(fd);
            map_count++;
        }


        else if(size < size_to_create)
        {



            int i=0;
            for (i=0; i<map_count;i++) {
                if (strcmp(file_info[i].filename, file_name) == 0) {
                    FILE *fd = fopen(path, "w+");
                    truncate(path, size_to_create);
                    //remap the file to memory
                    map = realloc(file_info[i].map_addr, size_to_create);
                    file_info[i].map_addr = map;
                    seg_mapped[i].segs=map;
#ifdef debug
                    printf("\n Updated Map:: Map address: %d, with file name: %s, and file size: %d \n",map,file_info[i].filename,size_to_create);
#endif
                    fclose(fd);
                }
            }
        }

        else if (size == size_to_create && getppid()!= (getpid()+1))
        {
            cout<< " Reading the File and Mapping into the virtual memory"<<endl;
            map = malloc(size_to_create);
            DIR *dp;
            struct dirent *ep;
            dp = opendir(rvm_dir_format);
            if(dp!=NULL)
            {
                while(ep = readdir(dp))
                {
                    token = strtok(ep->d_name,"$");
                    if(!strcmp(token, segname))
                    {
                        flag = true;
                        sprintf(file_found,"%s.txt",token);
                    }
                }
                (void) closedir(dp);
            }
            if(flag == true)
            {
                char l_path[40];
                sprintf(l_path,"%s/%s$log.txt",t_rvm.direc_name,segname);
                fstream fd(l_path);
                string entry;
                while(getline(fd,entry)){
                    int offset;
                    int size;
                    string data;
                    stringstream ss(entry);
                    ss >> offset >> size;
                    getline(ss,data,'\n');
                    data.erase(remove(data.begin(),data.end(),'\t'),data.end());

//                    strncpy((char*)(map+offset),data.c_str(),size);
                    memcpy((map+offset),data.c_str(),size);
                }
                int ret = remove(l_path);
                if(ret == 0)
                {
                    printf("lOG FILE Deleted\n");
                }
                else
                {
                    printf("Unable to Delete: LOG File not present\n");
                }
                flag = false;
            }
        }

        else
        {
            printf("\n Error to map the same segment with same size or less size\n");
            exit(0);
        }
    }
        //Creates new file and maps it
    else
    {
        transid++;
        FILE* fd = fopen(path, "w+");
        truncate(path, size_to_create);
        strcpy(file_info[map_count].filename, file_name);
        //map the file to memory
        map = malloc(size_to_create);
        file_info[map_count].map_addr = map;
        seg_mapped[map_count].segs=map;
        num_mapped_file++;

#ifdef debug
        printf("\n New Map:: Map address: %d, with file name: %s, and file size: %d \n",map,file_info[map_count].filename,size_to_create);
#endif

        fclose(fd);
        map_count++;

    }
    free(file_name);
    return map;
}

trans_t rvm_begin_trans(rvm_t rvm, int d, void **segs)
{
    cout<<endl;
    cout<<"RVM BEGIN TRANSACTION"<<endl;
    trans_t trans;
    trans.segs_trans = (char**)malloc (sizeof(char*)*d);
    int i;

    if(prev_trans_no == trans_no)
    {
        printf("current transaction %d", trans.trans_id);

        for(i = 0 ; i < d; i++)
        {
            file_info[i].total_files = d;
            file_info[i].dir_name=(char*)malloc(strlen(rvm.direc_name));
            strcpy(file_info[i].dir_name,rvm.direc_name);
            trans.num_fds = d;
            trans.segs_trans[i] = (char*)segs[i];
#ifdef debug
            printf("\nBegin transaction for %d  \n",trans.segs_trans[i]);
#endif
        }
    }
    else
    {
        printf("\n ERROR: transaction in progress: \n");

        abort();
    }
    trans_no = trans_no +1;
    return trans;
}


void  rvm_about_to_modify(trans_t tid, void *segbase, int offset, int size)
{
    cout<<endl;
    cout<<"RVM ABOUT TO MODIFY"<<endl;
    int i = 0;
    int j = 0;
    for(i = 0; i < tid.num_fds;i++)
    {
        if(tid.segs_trans[i] == segbase )
        {
            for(j = 0; j < count1 ; j++)
            {
                if((redo_log[j].offset+ redo_log[j].size+ redo_log[j].seg_base == offset+ size+ segbase) && (transac_complete==1))
                {
                    redo_log[j].offset = offset;
                    redo_log[j].size = size;
                    redo_log[j].seg_base = segbase;

                    goto label;
                }
            }
            transac_complete=1;
            undo_log[count1].data = (char*) malloc(size);
            undo_log[count1].seg_base = segbase;
            //change
            strncpy(undo_log[count1].data, (char*)segbase+offset,size);


#ifdef debug
            printf("\n New Undo log with log number: %d, with range: %d, segbase: %d, offset %d and data %s \n",count1,(size),segbase, offset, undo_log[count1].data);
#endif
            undo_log[count1].seg_base = segbase;
            undo_log[count1].offset = offset;
            undo_log[count1].size =  size;
            redo_log[count1].offset = offset;
            redo_log[count1].size = size;
            redo_log[count1].data=(char*)malloc(size);
            redo_log[count1].seg_base = segbase;

            count1++;
            label:break;
        }
    }
}

void rvm_commit_trans(trans_t trans)
{
    cout<<endl;
    cout<<"RVM COMMIT"<<endl;
    int i = 0,j=0;
    trans_no = 0;
    prev_trans_no = 0;
    for(j =0; j < count1; j++)
    {
        for(i =0; i < trans.num_fds; i++)
        {
            char path[60];
            sprintf(path,"%s/%s$log.txt",file_info[i].dir_name,file_info[i].filename);
            if(trans.segs_trans[i]==redo_log[j].seg_base)
            {

                strncpy(redo_log[j].data,trans.segs_trans[i]+redo_log[j].offset,redo_log[j].size);
#ifdef debug
                printf("\n Redo log with log number: %d, with range: %d, segbase: %d, offset %d and data %s \n",j,(redo_log[j].size),trans.segs_trans[i], redo_log[j].offset, redo_log[j].data);
#endif
                ofstream fds(path, ios::app);
                fds<<redo_log[j].offset<<'\t'<<redo_log[j].size<<'\t'<<redo_log[j].data<<'\n';
                fds.close();

            }
        }
    }
#ifdef debug
    int k;
    for(k=0;k<count1;k++)
    {

        free(redo_log[k].data);
    }
#endif
    i=0;
    for(i=0;i<num_mapped_file;i++){
//        free(trans.segs_trans[i]);
        free(undo_log[i].data);
    }
    //free(trans.segs_trans);
    transac_complete=0;
    count1=0;
    //map_count=0;

}


void rvm_abort_trans(trans_t trans){
    cout<<endl;
    cout<<"RVM ABORT TRANSACTION"<<endl;
    int i = 0,j=0;
    trans_no = 0;
    prev_trans_no = 0;
    for(j =0; j < count1; j++)
    {
        for(i =0; i < trans.num_fds; i++)
        {
            if(trans.segs_trans[i]==undo_log[j].seg_base)
            {
                strncpy((char*)(undo_log[j].seg_base+undo_log[j].offset),(undo_log[j].data),undo_log[j].size);

                strncpy((char*)(trans.segs_trans[i]+undo_log[j].offset),(undo_log[j].data),undo_log[j].size);
#ifdef debug
                cout<<"IN MEMORY DATA SEGMENT is "<<trans.segs_trans[i]+undo_log[j].offset<<endl;
                cout<<"UNDO LOG is "<<undo_log[j].data<<endl;
#endif
            }
        }
    }
    i=0;
    int k=0;
    for(i=0;i<num_mapped_file;i++){
        free(undo_log[i].data);
    }

    for(k=0;k<count1;k++)
    {
        free(redo_log[k].data);
    }
    count1=0;
//    map_count=0;
    transac_complete=0;
}


void rvm_unmap(rvm_t t_rvm,void *segbase ){
    cout<<endl;
    cout<<"RVM UNMMAP"<<endl;
    int i=0;
    bool no_segs=false;
    for(i=0;i<num_mapped_file;i++) {
        if(seg_mapped[i].segs==segbase) {
            free(segbase);
            no_segs=true;
            cout<<" Segment Unmapped Succesfully"<<endl;
        }

    }


    if(!no_segs)
    {
        cout<<" File not mapped: Cannot unmap before mapping"<<endl;
    }
    count1=0;
    map_count=0;
    num_mapped_file=0;
    transac_complete=0;
}


void rvm_truncate_log(rvm_t t_rvm){
    cout<<endl;
    cout<<"RVM TRUNCATE"<<endl;
    DIR *dp;
    struct dirent *ep;
    dp = opendir("./rvm_segments");
    char* token;
    int i =0;
    int value;
    string entry;
    bool flag = false;
    char file_log[20];
    char file_original[20];
    if(dp!=NULL)
    {
        while(ep =readdir(dp))
        {
            token = strtok(ep->d_name,"$");
            for(i = 0; i< num_mapped_file ; i++)
            {
                value = strcmp(token, file_info[i].filename);

                if (value == 0)
                {
                    sprintf(file_log, "rvm_segments/%s$log.txt", token);

                    sprintf(file_original, "rvm_segments/%s.txt", token);
                    fstream fd(file_log);
                    fstream fp(file_original);

                    while(getline(fd,entry))
                    {
                        int offset;
                        int size;
                        string data;
                        stringstream ss(entry);
                        ss >> offset >> size;
                        getline(ss,data,'\n');
                        data.erase(remove(data.begin(),data.end(),'\t'),data.end());

                        fp.seekp(offset);
                        fp.write(data.c_str(),data.size());


                    }
                    fd.close();
                    fp.close();
                    int ret = remove(file_log);
                    if(ret == 0)
                    {
                        printf(" LOG File Deleted\n"); // file exists
                    }
                    else
                    {
                        printf("LOG Unable to Delete: File not present\n");
                    }

                }
            }
        }
    }
}