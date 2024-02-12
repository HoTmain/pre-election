#include "poller.h"

int numWorkerThread;
int bufsize;
int port;
int fd;

mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

pthread_mutex_t mtx;    // general mutex, for set/get from buffer
pthread_mutex_t votemtx;    // for inserting in stats data structure
pthread_mutex_t fileprot;   // for inserting in logFile
pthread_cond_t cond_nonempty;   // lock producer (master) when buffer is full
pthread_cond_t cond_nonfull;    // lock consumer (workers) when buffer is empty
buffer buf;

int flag= 1;    // while equal to 1, threads will run endlessly (will change to 0 once SIGINT is recieved)
char* logFile;
char* statsFile;

struct sigaction act;
void sigflag(int sig);

void *accept_connections(void *socket);
void *read_request(void* arg);
void init(buffer*);


int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_in server;

    if (argc != 6) {
        printf("Please give: [portnum] [numWorkerthreads] [bufferSize] [output-log] [output-stats]\n");exit(1);}

    sigset_t set1;
    sigfillset(&set1);
    sigprocmask(SIG_SETMASK,&set1,NULL);    // block signals, so only master thread recieves it

    port= atoi(argv[1]);
    numWorkerThread= atoi(argv[2]);
    bufsize= atoi(argv[3]);


    if((sock= socket(AF_INET, SOCK_STREAM, 0)) == -1) { perror("socket"); exit(1); }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port);
    if(bind(sock, (struct sockaddr*) &server, sizeof(server)) < 0) {perror("bind"); exit(1);}

    if(listen(sock, 128) < 0) { perror("listen"); exit(1); }

    logFile= argv[4];
    statsFile= argv[5];
    pthread_mutex_init(&mtx, 0);
    pthread_mutex_init(&votemtx, 0);
    pthread_mutex_init(&fileprot, 0);
    pthread_cond_init(&cond_nonempty, 0);
	pthread_cond_init(&cond_nonfull, 0);

    int err;
    pthread_t master;
    if (err= pthread_create(&master, NULL, accept_connections, (void*) &sock)) {   //  Create master thread
        fprintf(stderr, "pthread_create: %s\n", strerror(err));
        exit(1);
    }

    int status;
    if (err= pthread_join(master, (void **) &status)) {   // Wait for master thread
        fprintf(stderr, "pthread_join: %s\n", strerror(err));
        exit(1);
    }
    return 0;
}


/*  Master  */
void *accept_connections(void *arg) {
    init(&buf);
    buf.data= malloc(sizeof(int) * bufsize);
    stats has_voted;
    create_stats(&has_voted);

    int err;
    pthread_t threads[numWorkerThread];
    for(int i= 0; i< numWorkerThread; i++) {        // create worker threads
        if (err= pthread_create(&threads[i], NULL, read_request, (void*) &has_voted)) { 
            fprintf(stderr, "pthread_join: %s\n", strerror(err));
            exit(1);
        }
    }


    int newsock;
    struct sockaddr_in client;
    socklen_t clientlen= sizeof(client);
    printf("Listening for connections to port %d\n", port);
    
    sigset_t set2;
    sigemptyset(&set2);
    sigprocmask(SIG_SETMASK,&set2,NULL);

    act.sa_flags = 0;
    act.sa_handler= sigflag;    // sigflag function will be responsible for stopping the threads from looping infinitely
    sigaction(SIGINT, &act, NULL);
    //  master thread loop
    int *sock= arg;
    fd= open(logFile, O_CREAT | O_RDWR | O_TRUNC, mode);
    while(flag) {
        newsock= accept(*sock, (struct sockaddr*) &client, &clientlen);


        my_mutex_lock(&mtx);    // produce
            while(buf.size >= bufsize) {
                pthread_cond_wait(&cond_nonfull, &mtx);
            }
            if(!flag) {
                my_mutex_unlock(&mtx);
                break;
            }
            buf.end= (buf.end +1) % bufsize;
            buf.data[buf.end]= newsock;
            buf.size++;

            pthread_cond_signal(&cond_nonempty);
        my_mutex_unlock(&mtx);


    }
    for(int i= 0; i< numWorkerThread; i++) {
        if (err= pthread_join(threads[i], NULL)) {   // Wait for workers first, before master thread exit
            fprintf(stderr, "pthread_join: %s\n", strerror(err));
            exit(1);
        }
    }

    post_stats(&has_voted, statsFile);  //  after server is done, create the pollStats

    free(buf.data);
    stats_destroy(&has_voted);

    close(fd);
    thread_exit();
}

/*  Worker  */
void *read_request(void* arg) {
    int amount;
    int newsock;
    int amount2;
    char name[128];
    char party[128];

    stats* has_voted= arg;
    //  worker thread loop
    while(flag) {

        my_mutex_lock(&mtx);    // consume
            while(buf.size <= 0) {
                pthread_cond_wait(&cond_nonempty, &mtx);
                if(!flag) {     // after emptying buffer do workers quit (buf.size == 0)
                    my_mutex_unlock(&mtx);      // locked by pthread_cond_wait, so unlock before quitting!
                    thread_exit();
                }
            }

            newsock= buf.data[buf.start];
            buf.start= (buf.start+1) % bufsize;
            buf.size--;

            pthread_cond_signal(&cond_nonfull);
        my_mutex_unlock(&mtx);



        if(write(newsock, "SEND NAME PLEASE ", 18) < 0){ perror("write"); exit(1);}
        amount= read(newsock, name, sizeof(name));
        name[amount]= '\0';

        my_mutex_lock(&votemtx);
            if(already_voted(has_voted, name)){
                write(newsock, "\nALREADY VOTED\n", 16);
                close(newsock);

                my_mutex_unlock(&votemtx);

                continue;
            }
            name_insert(has_voted, name);
        my_mutex_unlock(&votemtx);

        name[amount-1]= ' ';    // space character to separate name from vote in pollLog file

        write(newsock, "\nSEND VOTE PLEASE ", 19);
        amount2= read(newsock, party, sizeof(party));
        party[amount2]= '\0';
        
        vote_insert(has_voted, party);      // record vote

        
        party[amount2-1]= '\n';     // newline, so each recorded vote in log file is on a separate line

        my_mutex_lock(&fileprot);
            write(fd, strcat(name, party), amount+amount2);     //mutex is necessary in the extreme case where we have multiple votes recorder simultaneously
        my_mutex_unlock(&fileprot);
        
        party[amount2-1]= '\0';     //remove newline character for sprintf
        char votefor[27+amount2-1];        //  \nVOTE for Party RECORDED\n (27 bytes) + party(amount2-1 bytes)
        sprintf(votefor, "\nVOTE for Party %s RECORDED\n", party);
        write(newsock, votefor, sizeof(votefor));


        close(newsock);
    }
    thread_exit();
}


void init(buffer *buf) {
    buf->start= 0;
    buf->end= -1;
    buf->size= 0;
}

void post_stats(stats *stat, char* file) {
    FILE *fd2= fopen(file, "w+");

    for(int i= 0; i< stat->vote_cap; i++)
        fprintf(fd2, "%s %d\n", stat->vote[i].party, stat->vote[i].amount);
    
    fprintf(fd2,"TOTAL  %d", stat->total_votes);

    fclose(fd2);
}

void sigflag(int sig){
    flag= 0;
    pthread_cond_broadcast(&cond_nonempty);
}