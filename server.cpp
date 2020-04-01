#include <netdb.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

#include "function.h"
#include "thread_arg.h"

#define CREATE_PER_THREAD 0
#define PRE_CREATE 1

using namespace std;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
// ./server num_of_cores threading_strategy num_of_buckets

void * procRequests(void * arg) {
  while (1) {
    Thread_arg * thr_arg = (Thread_arg *)arg;
    int * bucket = thr_arg->bucket;
    int socket_fd = thr_arg->socket_fd;
    string ip;
    int client_fd = server_accept(socket_fd, &ip);
    if (client_fd == -1) {
      std::cout << "Error in build server!\n";
      exit(EXIT_FAILURE);
    }

    //receive request
    char request[20];
    memset(request, 0, sizeof(request));
    int len = recv(client_fd, request, sizeof(request), 0);
    if (len < 0) {
      close(client_fd);
      return NULL;
    }
    //parse the request
    string l1 = request;
    double delay = stoi(l1);
    int num = stoi(l1.substr(l1.find(",") + 1));
    cout << "request received, "
         << "delay: " << delay << ", number of bucket: " << num << endl;

    //delay loop
    delayloop(delay);

    //add delay count to certain bucket
    pthread_mutex_lock(&mutex);
    bucket[num] += delay;
    pthread_mutex_unlock(&mutex);

    //send response back
    string l2 = to_string(bucket[num]) + "\n";
    const char * response = l2.c_str();
    cout << "response: [" << num << "]" << response;
    send(client_fd, response, strlen(response), 0);

    close(client_fd);
  }
}

int main(int argc, char * argv[]) {
  if (argc != 4) {
    cerr << "Error: incorrect number of arguments" << endl;
    exit(EXIT_FAILURE);
  }

  int cores = atoi(argv[1]);
  int thrd = atoi(argv[2]);
  if (thrd != CREATE_PER_THREAD && thrd != PRE_CREATE) {
    cerr << "Error: invalid threading strategy" << endl;
    exit(EXIT_FAILURE);
  }
  int size = atoi(argv[3]);
  int bucket[size] = {0};

  //setup server
  int socket_fd = build_server("12345");
  string ip;
  int numThreads = 2000;

  if (thrd == CREATE_PER_THREAD) {
    //pthread_t * threads;
    //int numThreads = 2000;
    //threads = (pthread_t *)malloc(numThreads * sizeof(pthread_t));

    for (int i = 0; i < numThreads; i++) {
      //connect with each client
      int client_fd = server_accept(socket_fd, &ip);
      if (client_fd == -1) {
        std::cout << "Error in build server!\n";
        return -1;
      }
      cout << "socket " << i << " created" << endl;

      //handle request
      pthread_t thread;
      Thread_arg * thr_arg = new Thread_arg();
      thr_arg->client_fd = client_fd;
      thr_arg->bucket = bucket;
      pthread_create(&thread, NULL, processRequest, thr_arg);
      pthread_detach(thread);
    }
  }

  if (thrd == PRE_CREATE) {
    int numThreads = 500;
    pthread_t *threads;
    threads = (pthread_t *) malloc(numThreads * sizeof(pthread_t));
    Thread_arg * thr_arg = new Thread_arg();
    thr_arg->bucket = bucket;
    thr_arg->socket_fd = socket_fd;
    for (int i = 0; i < numThreads; i++) {
      pthread_create(&threads[i], NULL, procRequests, thr_arg);
    }
    for(int i = 0; i < numThreads; i++){
      pthread_join(threads[i], NULL);
    }
  }
  return 0;
}
