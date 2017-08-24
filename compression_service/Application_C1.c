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
        printf("\nUsage : ./App1 <filenames with ',' between them example: gauresh.txt,sanjeev.txt,gauresh.txt> \n");
        exit(0);
    }
    //Generate A unique client number
    srand(time(NULL));
    unsigned int client_number = (unsigned int)rand()%100;

    //not used
    char *result_data=NULL;


    //store the file names
    char file_names[100];

    strcpy(file_names,argv[1]);

    int res = BLOCK_API(file_names,  client_number, result_data);

    if(res==1)
    {
        printf("\nAPP1: Check Compressed files\n");
    }
    else{
        printf("\nAPP1: Error\n");
    }
    return 0;
}
