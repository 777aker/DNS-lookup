#ifndef MULTILOOKUP_H
#define MULTILOOKUP_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include "util.c"

int main(int argc, char *argv[]);

struct shared {
  // shared between requesters and resolvers
  // the shared buffer between requesters and resolvers
  char *shared_buffer[10];
  // how much of the shared buffer is filled
  int filled;
  // a semaphore for limiting requesters
  sem_t requesters;
  // a semaphore for limiting resolvers
  sem_t resolvers;
  // a mutex shared between requesters and solvers used to protect when filled is changed
  pthread_mutex_t shared_m;
  // shared between requesters
  // the service file requesters write to
  char *service;
  // the list of files that need to be processed by requesters
  char *file_names[10];
  // the index of how many files have been taken/are being processed
  int top;
  // how many total files are in names
  int total;
  // used to protect top
  pthread_mutex_t req_m;
  // mutex used to protect service file
  pthread_mutex_t req_ser;
  // shared between resolvers
  // file resolvers write to
  char *results;
  // mutex protecting the results file
  pthread_mutex_t res_m;
  // semaphore saying requesters are done
  sem_t req_done;
  // semaphore saying resolvers are done
  sem_t res_done;
};

void create_requesters(int number, struct shared *resources);
void* requesters_func(struct shared *resources);

void create_resolvers(int number, struct shared *resources);
void* resolvers_func(struct shared *resources);

#endif
