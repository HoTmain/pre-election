#pragma once
#include "poller.h"

typedef struct{
    char party[128];
    int amount;
}voteinfo;

typedef struct{
    char **name;
    voteinfo *vote;
    int size;
    int name_cap;
    int vote_cap;
    int total_votes;
}stats;

//  responisble for malloc etc.
void create_stats(stats*);

// reutrn 1 if name has already voted, 0 therwise
int already_voted(stats* stat, char* name);

// insert name to stats
// if name array reches maximum capacity, reallocate by calling increase_size(stat)
void name_insert(stats* stat, char* name);

// insert vote to stats
// increasing vote array size will most likely not be necessary, so it isn't checked
void vote_insert(stats *stat, char* vote);

// incrases name/vote array size
void increase_size(stats*);

// write poll statistics to stats file
void post_stats(stats*, char* pollStats);

// destroy the structure (deallocate)
void stats_destroy(stats*);