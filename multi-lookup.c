#include "multi-lookup.h"

int main(int argc, char *argv[]) {

  time_t start_time = time(NULL);

  time_t finish_time;

  time_t total_time;

  struct shared resources;

  resources.top_of_shared_buffer = 0;
  resources.top_of_name_files = 0;

  resources.requesters_done = 0;

  pthread_mutex_init(&resources.name_files_m, NULL);
  pthread_mutex_init(&resources.results_file_m, NULL);
  pthread_mutex_init(&resources.service_file_m, NULL);

  sem_init(&resources.requesters, 0, 9);
  sem_init(&resources.resolvers, 0, 0);

  resources.results_file_name = argv[4];

  resources.serviced_file_name = argv[3];

  pthread_mutex_init(&resources.shared_buffer_m, NULL);

  resources.max_name_files = argc - 5;

  for(int i = 0; i < (argc-5); i++) {

    strcpy(resources.name_files[i], argv[5+i]);

  }

  for(int i = 0; i < atoi(argv[1]); i++) {
    pthread_create(&resources.requester_ids[i], NULL, (void *) requesters_func, (void *) &resources);
  }

  for(int i = 0; i < atoi(argv[2]); i++) {
    pthread_create(&resources.resolver_ids[i], NULL, (void *) resolvers_func, (void *) &resources);
  }

  for(int i = 0; i < atoi(argv[1]); i++) {
    pthread_join(resources.requester_ids[i], NULL);
  }

  resources.requesters_done = 1;
  for(int i = 0; i < atoi(argv[2]); i++) {
    sem_post(&resources.resolvers);
  }

  for(int i = 0; i < atoi(argv[2]); i++) {
    pthread_join(resources.resolver_ids[i], NULL);
  }

  finish_time = time(NULL);

  total_time = finish_time - start_time;

  fprintf(stderr, "Runtime = %ld seconds", total_time);

  return 0;
}

void* requesters_func(struct shared *resources) {

  char *line;
  size_t len;
  int done = 0;
  int files_serviced = 0;

  FILE *file_ptr;

  while(!done) {

    pthread_mutex_lock(&resources->name_files_m);
    if(resources->top_of_name_files >= resources->max_name_files) {

      pthread_mutex_unlock(&resources->name_files_m);
      done = 1;

    } else {

      file_ptr = fopen(resources->name_files[resources->top_of_name_files], "r");
      resources->top_of_name_files++;
      pthread_mutex_unlock(&resources->name_files_m);


      if(file_ptr == NULL) {
        fprintf(stderr, "Couldn't open file %s.", resources->name_files[0]);
      } else {
        files_serviced++;
        while(getline(&line, &len, file_ptr) != -1) {

          line[strcspn(line, "\n")] = 0;

          sem_wait(&resources->requesters);

          pthread_mutex_lock(&resources->shared_buffer_m);

          strcpy(resources->shared_buffer[resources->top_of_shared_buffer], line);

          resources->top_of_shared_buffer++;

          pthread_mutex_unlock(&resources->shared_buffer_m);

          sem_post(&resources->resolvers);

        }
      }

      fclose(file_ptr);
    }

  }

  pthread_mutex_lock(&resources->service_file_m);

  file_ptr = fopen(resources->serviced_file_name, "a");
  if(file_ptr == NULL) {
    fprintf(stderr, "Cannot use argument 4 as service file.\n");
  } else {
    fprintf(file_ptr, "Thread <%ld> serviced %d file(s).\n", (long) pthread_self(), files_serviced);
  }
  fclose(file_ptr);

  pthread_mutex_unlock(&resources->service_file_m);

  return 0;
}

void* resolvers_func(struct shared *resources) {

  char line[1025];
  char ip[INET6_ADDRSTRLEN];

  FILE *file_ptr;

  while(1) {

    sem_wait(&resources->resolvers);

    pthread_mutex_lock(&resources->shared_buffer_m);

    if(resources->requesters_done && (resources->top_of_shared_buffer-1) == -1) {
      pthread_mutex_unlock(&resources->shared_buffer_m);
      return 0;
    } else {

      resources->top_of_shared_buffer--;

      strcpy(line, resources->shared_buffer[resources->top_of_shared_buffer]);

      pthread_mutex_unlock(&resources->shared_buffer_m);

      sem_post(&resources->requesters);

      if(dnslookup(line, ip, INET6_ADDRSTRLEN) == -1) {

        pthread_mutex_lock(&resources->results_file_m);
        file_ptr = fopen(resources->results_file_name, "a");
        if(file_ptr == NULL) {
          file_ptr = fopen(resources->results_file_name, "w");
          if(file_ptr == NULL) {
            fprintf(stderr, "file: %s\n", resources->results_file_name);
          }
        }

        fprintf(file_ptr, "%s,\n", line);

        fclose(file_ptr);
        pthread_mutex_unlock(&resources->results_file_m);

      } else {

        pthread_mutex_lock(&resources->results_file_m);
        file_ptr = fopen(resources->results_file_name, "a");
        if(file_ptr == NULL) {
          file_ptr = fopen(resources->results_file_name, "w");
          if(file_ptr == NULL) {
            fprintf(stderr, "file: %s\n", resources->results_file_name);
          }
        }

        fprintf(file_ptr, "%s,%s\n", line, ip);

        fclose(file_ptr);
        pthread_mutex_unlock(&resources->results_file_m);

      }
    }

  }

  return 0;
}
