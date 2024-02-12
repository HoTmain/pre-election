#include "stats.h"

void create_stats(stats* stat) {
    stat->size= 128;
    stat->name_cap= 0;
    stat->vote_cap= 0;
    stat->total_votes= 0;
    stat->name= malloc(128 * sizeof(char*));
    stat->vote= malloc(128 * sizeof(voteinfo));
    for(int i=0; i< 128; i++) {
        stat->name[i]= malloc(sizeof(char) * 128);

        stat->vote[i].amount=0;
    }
}

int already_voted(stats* stat, char* name) {
    for(int i= 0; i < stat->name_cap; i++) {
        if(!strcmp(stat->name[i], name))
            return 1;
    }
    return 0;
}

void name_insert(stats* stat, char* name) {
    if(stat->name_cap == stat->size) {
        increase_size(stat);
    }
    strcpy(stat->name[stat->name_cap], name);
    stat->name_cap++;
}

void vote_insert(stats *stat, char* vote) {
    for(int i= 0; i< stat->vote_cap; i++) {
        if(!strcmp(stat->vote[i].party, vote)) {
            stat->vote[i].amount++;
            stat->total_votes++;
            return;
        }
    }
    strcpy(stat->vote[stat->vote_cap].party, vote);
    stat->vote[stat->vote_cap].amount++;
    stat->vote_cap++;
    stat->total_votes++;
}

void increase_size(stats* stat) {
    stat->size*=2;
    stat->name= realloc(stat->name, sizeof(char*) * stat->size);
    for(int i= stat->name_cap; i < stat->size; i++)
        stat->name[i]= malloc(sizeof(char) * 128);
}

void stats_destroy(stats *stat) {
    for(int i=0; i< stat->size; i++)
        free(stat->name[i]);
    free(stat->name);
    free(stat->vote);
}