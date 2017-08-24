//
// Created by gauresh on 3/4/17.
//
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<limits.h>
#include <time.h>
#include"Client_API.h"



int main(int argc, char** argv)
{
    if(argc<2)
    {
        printf("\nPlease input 2 parameters\n ");
        printf("\nUsage : ./App3 <filenames with ',' between them example: gauresh.txt,sanjeev.txt,gauresh.txt> \n");
        exit(0);
    }

    //for client name
    srand(time(NULL));
    unsigned int client_number = (unsigned int)rand()%10000;

    char *result_data=NULL;

    char file_names[100];

    strcpy(file_names,argv[1]);

    int res = NONBLOCK_API(file_names,  client_number, result_data);

    printf("\nSleeping on App side for 3 seconds \n");
    printf("\nEmulating Useful work\n");
    sleep(3);

    if(res==1) {

        int results = WAIT_NONBLOCK_RESULT();

        if (results == 1) {
            printf("\nAPP3: Compression Success\n");
        }
        else {
            printf("\nAPP3: Compression Failed\n");
        }
    }
    return 0;
}
