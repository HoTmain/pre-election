#include "poller.h"
#include <netdb.h>	         /* gethostbyname */

mode_t mode = S_IRUSR | S_IRGRP | S_IROTH;


pthread_mutex_t mtx;

char* hname;
int port;
FILE *fd;


void* send_vote(void* arg);

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Please give: [serverName] [portNum] [input-file]\n");exit(1);}

    hname= argv[1];
    port= atoi(argv[2]);

    char* fname= argv[3];
    fd= fopen(fname, "r");
    if(fd == NULL) {
        perror("fopen");
        exit(1);
    }

    pthread_mutex_init(&mtx, 0);


    int lines= totalines(fd);
    int i;
    int err;
    pthread_t client[lines];
    for(i= 0; i< lines; i++) {
        if(err= pthread_create(&client[i], 0, send_vote, 0)) {
            fprintf(stderr, "pthread_create: %s\n", strerror(err));
            exit(1);
        }
    }

    for(; i; i--) {
        if (err= pthread_join(client[lines-i], NULL)) {   // Wait for clients (NULL for thread_return, otherwise it messes with variable values (int lines becomes 0 ??))
            fprintf(stderr, "pthread_join: %s\n", strerror(err));
            exit(1);
        }
    }
    fclose(fd);

    return 0;
}

// client threads
void* send_vote(void* arg) {
    voter votr;
    my_mutex_lock(&mtx);
        get_name_vote(&votr, fd);
    my_mutex_unlock(&mtx);

    struct sockaddr_in server;
    struct hostent *rem;

    int sock;
    if ((rem= gethostbyname(hname)) == NULL) { herror("gethostbyname"); exit(1); }

    if((sock= socket(AF_INET, SOCK_STREAM, 0)) == -1) { perror("socket"); exit(1); }
    
    server.sin_family = AF_INET;
    memcpy(&server.sin_addr, rem->h_addr, rem->h_length);
    server.sin_port = htons(port);
    if (connect(sock, (struct sockaddr*) &server, sizeof(server)) < 0) { perror("connect"); exit(1);}


    char rd[64];
    int amount;

    amount= read(sock, rd, sizeof(rd));
    if(amount == -1) { thread_exit();}
    rd[amount]= '\0';
    write(1, rd, amount);
    write(sock, votr.name, votr.nsize);     // send name
    
    amount= read(sock, rd, sizeof(rd));
    if(amount == -1) { thread_exit();}
    rd[amount]= '\0';
    write(1, rd, amount);
    write(sock, votr.vote, votr.vsize);     // send vote

    
    amount= read(sock, rd, sizeof(rd));     //  "VOTE for Party XYZ RECORDED"
    if(amount == -1) { thread_exit();}
    rd[amount]= '\0';
    write(1, rd, amount);


    thread_exit();
}