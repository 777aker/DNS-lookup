#ifndef MULTILOOKUP_H
#define MULTILOOKUP_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <time.h>

#include "util.c"
#include "util.h"

struct shared {

  int requesters_done;

  char shared_buffer[10][1025];
  int top_of_shared_buffer;
  pthread_mutex_t shared_buffer_m;

  char name_files[10][1025];
  int top_of_name_files;
  pthread_mutex_t name_files_m;
  int max_name_files;

  pthread_t requester_ids[20];
  pthread_t resolver_ids[20];

  char *results_file_name;
  pthread_mutex_t results_file_m;

  sem_t requesters;
  sem_t resolvers;

  char *serviced_file_name;
  pthread_mutex_t service_file_m;

};

int main(int argc, char *argv[]);

void* requesters_func(struct shared *resources);

void* resolvers_func(struct shared *resources);

#endif
