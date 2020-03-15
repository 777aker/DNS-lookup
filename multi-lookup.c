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
  // set the top to 0 which is basically where in the names array we are
  resources.top = 0;
  // the file requesters write to
  resources.service = argv[3];
  // the file resolvers write to
  resources.results = argv[4];

  // intialize done to false since we are just starting
  resources.requesters_done = 0;

  // semaphores saying whats done
  sem_init(&resources.req_done, 0, 0);
  sem_init(&resources.res_done, 0, 0);

  // initializing some mutexes that protect certain resources
  pthread_mutex_init(&resources.req_m, NULL);
  pthread_mutex_init(&resources.req_ser, NULL);
  pthread_mutex_init(&resources.res_m, NULL);

  pthread_t req_tids[atoi(argv[1])];
  // create the threads
  for(int i = 0; i < atoi(argv[1]); ++i) {
    pthread_create(&req_tids[i],NULL, (void *) requesters_func, (void *) &resources);
  }

  pthread_t res_tids[atoi(argv[2])];
  // create the threds
  for(int i = 0; i < atoi(argv[2]); ++i) {
    pthread_create(&res_tids[i],NULL, (void *) resolvers_func, (void *) &resources);
  }

  // wait for all of them to finish
  for(int i = 0; i < atoi(argv[1]); ++i) {
    pthread_join(req_tids[i],NULL);
  }

  // make the boolean that the resolvers are done true
  resources.requesters_done = 1;

  // since requesters are done let the resolvers run as much as they want
  for(int i = 0; i < 11; i++) {
    fprintf(stderr, "go go go");
    sem_post(&resources.resolvers);
  }

  // wait for them to finish
  for(int i = 0; i < atoi(argv[2]); ++i) {
    pthread_join(res_tids[i],NULL);
  }

  return 0;
}

// everything above this line works as it supposed to with no warnings

void* requesters_func(struct shared *resources) {
  // the index in the file_names this thread is working on
  int file_index;
  // the line of the file we are reading
  char *line;
  // the length of that line
  size_t len;
  // length of the line we read
  ssize_t read;
  // file pointer to the file we are reading
  FILE *file_ptr;
  // how many files this guy serviced
  int files = 0;
  // store this threads id
  pid_t id = 5;
  // while there is work to be done kinda
  while(1) {
    // lock the variable that points to where we are in names files
    pthread_mutex_lock(&resources->req_m);
    // get the variable and increment it
    file_index = resources->top;
    resources->top++;
    // unlock it
    pthread_mutex_unlock(&resources->req_m);
    // if we have exceeded the number of files given then we are done
    if(file_index >= resources->total) {
      // there are no more files to be serviced so do the finishing actions for this thread
      // lock the serviced file
      pthread_mutex_lock(&resources->req_ser);
      // write Thread <threadid> serviced # files
      file_ptr = fopen(resources->service, "r+");
      fprintf(file_ptr, "Thread <%d> serviced %d files.\n", id, files);
      fclose(file_ptr);
      // unlock the serviced file
      pthread_mutex_unlock(&resources->req_ser);
      // return/kill this thread
      return 0;
    }
    // get our file
    file_ptr = fopen(resources->file_names[file_index], "r");
    if(file_ptr == NULL) {
      fprintf(stderr, "error opening %s", resources->file_names[file_index]);
    }
    fprintf(stderr, "fuckin ell mate");
    // while there are lines to read, store the line in variable line
    while((read = getline(&line, &len, file_ptr)) != -1) {
      // wait so if the shared buffer is full we don't try to overwrite it
      sem_wait(&resources->requesters);
      // lock our shared buffer
      pthread_mutex_lock(&resources->shared_m);
      fprintf(stderr, "%d", resources->filled);
      fprintf(stderr, "check in");
      strcpy(resources->shared_buffer[resources->filled], line);
      resources->filled++;
      // unlock our buffer
      pthread_mutex_unlock(&resources->shared_m);
      // signal that there is a resource for resolvers to use
      sem_post(&resources->resolvers);
    }
    // close the file we have been using
    fclose(file_ptr);
    //increment how many files this guy finished
    files++;
  }
  return 0;
}

void* resolvers_func(struct shared *resources) {
  // line we grab from the shared buffer
  char *line;
  // result of the line from the shared buffer
  char *ip;
  // results file we are writing to
  FILE *file_ptr;
  // repeat this forever
  while(1) {
    // wait until there is a resource available
    sem_wait(&resources->resolvers);
    // block the shared buffer
    pthread_mutex_lock(&resources->shared_m);
    // if done and filled = 0 then quit nothing left to do
    if(resources->requesters_done == 1 && resources->filled == 0) {
      return 0;
    }
    // decrement filled from the shared buffer
    resources->filled--;
    // grab a line from the shared buffer
    line = resources->shared_buffer[resources->filled];
    // unblock the shared buffer
    pthread_mutex_unlock(&resources->shared_m);
    // signal to requesters that there is another open slot
    sem_post(&resources->requesters);
    // take what we got from shared buffer and run dns from util
    if(dnslookup(line, ip, INET6_ADDRSTRLEN) == -1) {
      fprintf(stderr, "Converting to ip failed, filling in as blank for %s\n", line);
      ip = "";
    }
    // block the results file so we can write to it
    pthread_mutex_lock(&resources->res_m);
    // write line we grabbed from shared buffer, ip from dns to file
    file_ptr = fopen(resources->results, "r+");
    fprintf(file_ptr, "%s,%s\n", line, ip);
    fclose(file_ptr);
    // unblock the results file
    pthread_mutex_unlock(&resources->res_m);
  }
  return 0;
}
