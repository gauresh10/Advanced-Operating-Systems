//
// Created by gauresh on 3/4/17.
//

#ifndef CLIENT_API_H
#define CLIENT_API_H

#include"include.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <mqueue.h>
#include "snappy.h"

//blocking

#define sleep_time 1

//non block parameters

long *size_array;

long *shm_ptr_array;

long *shm_shmid_array;

key_t *shm_key_array;

int total_request;




mqd_t qd_clienta;



int BLOCK_API(char *filename,  int client_identifier, char* result_data);

int NONBLOCK_API(char *filename,  int client_identifier, int success_send);

int WAIT_NONBLOCK_RESULT();

#endif