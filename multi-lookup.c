#include "multi-lookup.h"

int main(int argc, char *argv[]) {

  // if we didn't get the right number of arguments quit
  if(argc < 6) {
    fprintf(stderr, "Expected at least 6 arguments, got %d\n", argc);
    return 0;
  }

  // the shared buffer
  struct shared resources;
  // where the shared buffer is filled to
  resources.filled = 0;
  // a mutex to protect filled
  pthread_mutex_init(&resources.shared_m, NULL);
  // these are how many resources can be written to and read in the shared buffer
  sem_init(&resources.requesters, 0, 10);
  sem_init(&resources.resolvers, 0, 0);

  // get all the files they want us to process, unless there are more than 10
  if(argc - 5 > 10) {
    fprintf(stderr, "Expected at most 10 files to process, received %d\n", (argc - 5));
    return 0;
  }
  for(int i = 5; i < argc; i++) {
    resources.file_names[(i-5)] = argv[i];
  }

  // this is to find out how many files we are processing
  resources.total = (argc - 5);
  fprintf(stderr, "%d\n", resources.total);
  // set the top to 0 which is basically where in the names array we are
  resources.top = 0;
  // the file requesters write to
  resources.service = argv[3];
  // the file resolvers write to
  resources.results = argv[4];

  // semaphores saying whats done
  sem_init(&resources.req_done, 0, 0);
  sem_init(&resources.res_done, 0, 0);

  // initializing some mutexes that protect certain resources
  pthread_mutex_init(&resources.req_m, NULL);
  pthread_mutex_init(&resources.req_ser, NULL);
  pthread_mutex_init(&resources.res_m, NULL);

  // create the threads
  create_requesters(atoi(argv[1]), &resources);
  create_resolvers(atoi(argv[2]), &resources);
  // wait until the threads are done
  sem_wait(&resources.req_done);
  sem_wait(&resources.res_done);

  return 0;
}

void create_requesters(int number, struct shared *resources) {
  pthread_t tids[number];
  // create the threads
  for(int i = 0; i < number; ++i) {
    pthread_create(&tids[i],NULL, (void *) requesters_func, (void *) resources);
  }
  // wait for all of them to finish
  for(int i = 0; i < number; ++i) {
    pthread_join(tids[i],NULL);
  }
  // signal that requesters are done
  sem_post(&resources->req_done);
}

void create_resolvers(int number, struct shared *resources) {
  pthread_t tids[number];
  // create the threds
  for(int i = 0; i < number; ++i) {
    pthread_create(&tids[i],NULL, (void *) resolvers_func, (void *) resources);
  }
  // wait for them to finish
  for(int i = 0; i < number; ++i) {
    pthread_join(tids[i],NULL);
  }
  // signal that resolvers are done
  sem_post(&resources->res_done);
}

// everything above this line works as it supposed to with no warnings

void* requesters_func(struct shared *resources) {
  fprintf(stderr, "hello\n");
  fprintf(stderr, "%d\n", resources->total);
  return 0;
}

void* resolvers_func(struct shared *resources) {
  fprintf(stderr, "world\n");
  return 0;
}
