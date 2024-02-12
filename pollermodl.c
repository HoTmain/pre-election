#include "poller.h"

void my_mutex_lock(pthread_mutex_t* mtx) {
    if (pthread_mutex_lock(mtx)) {
        perror("pthread_mutex_lock"); exit(1);
    }
}

void my_mutex_unlock(pthread_mutex_t* mtx) {
    if (pthread_mutex_unlock(mtx)) {
        perror("pthread_mutex_unlock"); exit(1);
    }
}

int totalines(FILE* fd) {      // after call, file position will be at the EOF
    int linesc=1;
    for (int c = getc(fd); c != EOF; c = getc(fd))
        if (c == '\n')
            linesc++;
    
    rewind(fd);             // ,so rewind the fp back at the beginning of file
    return linesc;
}



void get_name_vote(voter* client, FILE *fd) {
    int i= 0;
    char c;
    for(c = getc(fd); c != EOF && c != ' '; c = getc(fd)) {
        client->name[i]= c;
        i++;
    }
    client->name[i]= ' ';
    i++;

    for(c = getc(fd); c != EOF && c != ' '; c = getc(fd)) {
        client->name[i]= c;
        i++;
    }
    client->name[i]= '\0';
    client->nsize= ++i;


    i= 0;
    for(c = getc(fd); c != EOF && c != '\n'; c = getc(fd)) {
        client->vote[i]= c;
        i++;
    }
    client->vote[i]= '\0';
    client->vsize= ++i;
}