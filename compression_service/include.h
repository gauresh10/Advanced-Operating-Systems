
#include<pthread.h>
#include <time.h>
#include<string.h>
#define SERVER_QUEUE_NAME "/sp-example-server"
 #define QUEUE_PERMISSIONS 0660 
 #define MAX_MESSAGES 10 
 #define MAX_MSG_SIZE 256 
 #define MSG_BUFFER_SIZE MAX_MSG_SIZE + 10
#define SEM_SHM_NAME "/SEMAPHORE"
#define MAX_CLIENT_NAME 100


//message queue struct
struct msgbuffer
{
    long  mtype;
    char key;
};
typedef struct msgbuffer msgbuf;

typedef struct
{
    pthread_mutex_t mutex;
    long s_sz;
}shm_data_struct_t;



typedef struct mqueue
{
    long key;
    long size;	
    char message[100];
} mqueue;


