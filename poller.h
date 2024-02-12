#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>          // strerror
#include <sys/types.h>	     /* sockets */
#include <sys/wait.h>	     /* sockets */
#include <fcntl.h>           // oflag in open
#include <sys/stat.h>        // mode_t in open
#include <sys/socket.h>
#include <netinet/in.h>	     /* internet sockets */
#include <pthread.h>         // threads
#include <signal.h>          /* signal */
#include <unistd.h>          // write

#include "stats.h"

#define thread_exit() return (void *)0      // avoid still reachable leaks

typedef struct {
    int *data;
    int size;
    int start;
    int end;
}buffer;

typedef struct{
    char name[128];
    int nsize;
    char vote[128];
    int vsize;
}voter;

void my_mutex_lock(pthread_mutex_t*);
void my_mutex_unlock(pthread_mutex_t*);

// number of lines
int totalines(FILE*);

// for some line in the file, get the name, size of name, vote, size of vote, and save them in voter
void get_name_vote(voter*, FILE*);