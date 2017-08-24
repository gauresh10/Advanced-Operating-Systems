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
        printf("\nUsage : ./App2 <filenames with ',' between them example: gauresh.txt,sanjeev.txt,gauresh.txt> \n");
        exit(0);
    }
    //for client name
    srand(time(NULL));
    unsigned int client_number = (unsigned int)rand()%1000;

    char *result_data=NULL;

    char file_names[100];

    strcpy(file_names,argv[1]);

    int res = BLOCK_API(file_names,  client_number, result_data);

    if(res==1)
    {
        printf("\nAPP2: Check Compressed files\n");
    }
    else{
        printf("\nAPP2: Error\n");
    }
    return 0;
}
