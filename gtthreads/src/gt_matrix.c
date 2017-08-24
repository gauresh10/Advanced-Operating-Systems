#include <stdio.h>
#include <unistd.h>
#include <linux/unistd.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sched.h>
#include <signal.h>
#include <setjmp.h>
#include <errno.h>
#include <assert.h>

#include "gt_include.h"


#define ROWS 512
#define COLS ROWS
#define SIZE COLS

#define NUM_CPUS 2
#define NUM_GROUPS NUM_CPUS
#define PER_GROUP_COLS (SIZE/NUM_GROUPS)

#define NUM_THREADS 32


#define PER_THREAD_ROWS (SIZE/NUM_THREADS)


unsigned long int total_exe_time=0;

unsigned int sched_policy;
extern unsigned int kthread_create_flag;


/* A[SIZE][SIZE] X B[SIZE][SIZE] = C[SIZE][SIZE]
 * Let T(g, t) be thread 't' in group 'g'.
 * T(g, t) is responsible for multiplication :
 * A(rows)[(t-1)*SIZE -> (t*SIZE - 1)] X B(cols)[(g-1)*SIZE -> (g*SIZE - 1)] */

typedef struct matrix
{
	int m[SIZE][SIZE];

	int rows;
	int cols;
	unsigned int reserved[2];
} matrix_t;

unsigned long int thread_data[NUM_THREADS_MAX];




typedef struct __uthread_arg
{
	matrix_t *_A, *_B, *_C;
	unsigned int reserved0;

	unsigned int tid;
	unsigned int gid;
	int start_row; /* start_row -> (start_row + PER_THREAD_ROWS) */
	int start_col; /* start_col -> (start_col + PER_GROUP_COLS) */
    int end_row;
    int end_col;
    unsigned int credits_assigned;
}uthread_arg_t;

struct timeval tv1;

static void generate_matrix(matrix_t *mat, int val)
{

	int i,j;
	mat->rows = SIZE;
	mat->cols = SIZE;
	for(i = 0; i < mat->rows;i++)
		for( j = 0; j < mat->cols; j++ )
		{
			mat->m[i][j] = val;
		}
	return;
}

static void print_matrix(matrix_t *mat)
{
	int i, j;

	for(i=0;i<SIZE;i++)
	{
		for(j=0;j<SIZE;j++)
			printf(" %d ",mat->m[i][j]);
		printf("\n");
	}

	return;
}


static void * uthread_mulmat1(void *p) {
    int i, j, k;
    int start_row, end_row;
    int start_col, end_col;
    unsigned int cpuid;
    struct timeval tv2;

#define ptr ((uthread_arg_t *)p)

    i = 0;
    j = 0;
    k = 0;

    start_row = 0;
    end_row = ptr->end_row;//(ptr->start_row + PER_THREAD_ROWS);

#ifdef GT_GROUP_SPLIT
    start_col = 0;
    end_col = ptr->end_col; //(ptr->start_col + PER_THREAD_ROWS);
#else
    start_col = 0;
    end_col = ptr->end_col;
#endif

#ifdef GT_THREADS
    cpuid = kthread_cpu_map[kthread_apic_id()]->cpuid;
    fprintf(stderr, "\nThread(id:%d, group:%d, cpu:%d) started",ptr->tid, ptr->gid, cpuid);
#else
    fprintf(stderr, "\nThread(id:%d, group:%d) started", ptr->tid, ptr->gid);
#endif

    for (i = start_row; i < end_row; i++)
        for (j = start_col; j < end_col; j++)
            for (k = 0; k < end_col; k++) {
                if(ptr->tid== 123 && i==30 && j ==30 && k==30){

                }
                ptr->_C->m[i][j] += ptr->_A->m[i][k] * ptr->_B->m[k][j];
                }
#ifdef GT_THREADS
    fprintf(stderr, "\nThread(id:%d, group:%d, cpu:%d) finished (TIME : %lu s and %lu us)",
			ptr->tid, ptr->gid, cpuid, (tv2.tv_sec - tv1.tv_sec), (tv2.tv_usec - tv1.tv_usec));
#else
    gettimeofday(&tv2,NULL);
    fprintf(stderr, "\nThread(id:%d, group:%d) finished (TIME : %lu s and %lu us)",
            ptr->tid, ptr->gid, (tv2.tv_sec - tv1.tv_sec), (tv2.tv_usec - tv1.tv_usec));
#endif
    thread_exec_time[ptr->tid].exe_time= (tv2.tv_sec * 1000000 + tv2.tv_usec) - (tv1.tv_sec * 1000000 + tv1.tv_usec);


#undef ptr
    return 0;
}

static void * uthread_mulmat(void *p)
{
	int i, j, k;
	int start_row, end_row;
	int start_col, end_col;
	unsigned int cpuid;
	struct timeval tv2;

#define ptr ((uthread_arg_t *)p)

	i=0; j= 0; k=0;

	start_row = 0;//ptr->start_row;
	end_row = SIZE;//(ptr->start_row + PER_THREAD_ROWS);

#ifdef GT_GROUP_SPLIT
	start_col = ptr->start_col;
	end_col = (ptr->start_col + PER_THREAD_ROWS);
#else
	start_col = 0;
	end_col = SIZE;
#endif

#ifdef GT_THREADS
	cpuid = kthread_cpu_map[kthread_apic_id()]->cpuid;
	fprintf(stderr, "\nThread(id:%d, group:%d, cpu:%d) started",ptr->tid, ptr->gid, cpuid);
#else
	fprintf(stderr, "\nThread(id:%d, group:%d) started",ptr->tid, ptr->gid);
#endif

	for(i = start_row; i < end_row; i++)
		for(j = start_col; j < end_col; j++)
			for(k = 0; k < SIZE; k++)
				ptr->_C->m[i][j] += ptr->_A->m[i][k] * ptr->_B->m[k][j];

#ifdef GT_THREADS
	fprintf(stderr, "\nThread(id:%d, group:%d, cpu:%d) finished (TIME : %lu s and %lu us)",
			ptr->tid, ptr->gid, cpuid, (tv2.tv_sec - tv1.tv_sec), (tv2.tv_usec - tv1.tv_usec));
#else
	gettimeofday(&tv2,NULL);
	fprintf(stderr, "\nThread(id:%d, group:%d) finished (TIME : %lu s and %lu us)",
			ptr->tid, ptr->gid, (tv2.tv_sec - tv1.tv_sec), (tv2.tv_usec - tv1.tv_usec));
#endif
   // thread_exec_time[ptr->tid]= (tv2.tv_sec * 1000000 + tv2.tv_usec) - (tv1.tv_sec * 1000000 + tv1.tv_usec);


#undef ptr
	return 0;
}

matrix_t A, B, C;

static void init_matrices()
{
	generate_matrix(&A, 1);
	generate_matrix(&B, 1);
	generate_matrix(&C, 0);

	return;
}


//uthread_arg_t uargs[NUM_THREADS];
//uthread_t utids[NUM_THREADS];


uthread_arg_t uargs1[NUM_THREADS_MAX];
uthread_t utids1[NUM_THREADS_MAX];

int main(int argc, char **argv) {
    if (argc < 2) {
        //default O(1) selection
        printf("please input a valid number");
        //sched_policy = 0;
        return 0;
    } else {
        //select O(1) or CBS
        sched_policy = atoi(argv[1]);
    }

    uthread_arg_t *uarg;
    int inx;
    int credits[4] = {25, 50, 75, 100};
    int matrix_size[4] = {32, 64, 128, 256};

    gtthread_app_init();

    if (sched_policy == 0) {

        init_matrices();

        gettimeofday(&tv1, NULL);

        for (inx = 0; inx < NUM_THREADS; inx++) {

            uarg = &uargs1[inx];

            uarg->_A = &A;
            uarg->_B = &B;
            uarg->_C = &C;

            uarg->tid = inx;

            uarg->gid = (inx % NUM_GROUPS);

            uarg->start_row = 0;
            uarg->end_col = SIZE;
            uarg->end_row = SIZE;
#ifdef GT_GROUP_SPLIT
            /* Wanted to split the columns by groups !!! */
            uarg->start_col = 0;
#endif

            //default credit of 100 changes
            uthread_create(&utids1[inx], uthread_mulmat1, uarg, uarg->gid, 0);
        }

    } else if (sched_policy == 1) {

        unsigned int i = 0;
        unsigned int t = 0;
        unsigned int thread_id = 0;

        init_matrices();

        gettimeofday(&tv1, NULL);
        //  for (int i = 0; i < 16; i++) {
        int k = 0, j = 0;
        for (j = 0; j < 4; j++) { //credits

            for (k = 0; k < 4; k++) {//matrix size


                t = 8 * i;

                for (inx = t; inx < t + 8; inx++) {


                    uarg = &uargs1[inx];
                    uarg->_A = &A;
                    uarg->_B = &B;
                    uarg->_C = &C;

                    uarg->tid = inx;

                    uarg->gid = (inx % NUM_GROUPS);


                    uarg->end_col = matrix_size[k];
                    uarg->end_row = matrix_size[k];


                    uthread_create(&utids1[inx], uthread_mulmat1, uarg, uarg->gid, credits[j]);
                    //thread_id++;

                }

                i++;

                //printf("ksdfvdjfvjl\n\t%d",i);
            }
        }
    }
    gtthread_app_exit();
    if (sched_policy == 1) {
        int i = 0,j=0,k=0,th_id=0;
        unsigned int long sumval_cpu,sumval_exec;

        for(i=0;i<4;i++)
        {
            for(j=0;j<4;j++){
                sumval_cpu=0L;
                sumval_exec=0L;
                for(k=0;k<8;k++)
                {
                    sumval_cpu+=thread_cpu_time[th_id].cpu_time;
                    sumval_exec+=thread_exec_time[th_id].exe_time;
                    th_id++;
                }
                printf("\n\tCredit %d and matrix %d has CPU TIME %lu us and EXEC Time %lu\n\t",credits[i],matrix_size[j],sumval_cpu/8,sumval_exec/8);

            }
        }


    }

    return (0);

}