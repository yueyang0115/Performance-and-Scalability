#include <netdb.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <time.h>
#include <cstring>
#include <iostream>

#include "socket.h"
#include "thread_arg.h"

#define CREATE_PER_THREAD 0
#define PRE_CREATE 1

using namespace std;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void delayloop(double req_delay) {
  struct timeval start, check;
  double elapsed_seconds;
  gettimeofday(&start, NULL);
  do {
    gettimeofday(&check, NULL);
    elapsed_seconds = (check.tv_sec + (check.tv_usec / 1000000.0)) -
                      (start.tv_sec + (start.tv_usec / 1000000.0));
  } while (elapsed_seconds < req_delay);
}

double calc_time(struct timespec start, struct timespec end) {
  double start_sec = (double)start.tv_sec*1000000000.0 + (double)start.tv_nsec;
  double end_sec = (double)end.tv_sec*1000000000.0 + (double)end.tv_nsec;

  if (end_sec < start_sec) {
    return 0;
  } else {
    return end_sec - start_sec;
  }
}

//create a thread per request
void * procOneRequest(void * arg) {
  Thread_arg * thr_arg = (Thread_arg *)arg;
  int client_fd = thr_arg->client_fd;
  int * bucket = thr_arg->bucket;
  int * numRequest = thr_arg->numRequest;
  
  //receive request
  char request[20];
  memset(request, 0, sizeof(request));
  int len = recv(client_fd, request, sizeof(request), 0);
  if (len <= 0) {
    close(client_fd);
    return NULL;
  }
  //parse the request
  string l1 = request;
  double delay = stoi(l1);
  int num = stoi(l1.substr(l1.find(",") + 1));
  
  //delay loop
  delayloop(delay);

  //add delay count to certain bucket
  pthread_mutex_lock(&mutex);
  bucket[num] += delay;
  int currReq = ++(*numRequest);
  cout << "Request[" << *numRequest << "] received, "
       << "delay: " << delay << ", number of bucket: " << num << endl;
  pthread_mutex_unlock(&mutex);

  //send response back
  string l2 = to_string(bucket[num]) + "\n";
  const char * response = l2.c_str();
  cout << "bucket[" << num << "]: " << response;
  send(client_fd, response, strlen(response), 0);
  cout << "Request[" << currReq << "] processed" << endl;
  
  close(client_fd);
  return NULL;
}

//pre-create threads
void * procRequests(void * arg) {
  while (1) {
    Thread_arg * thr_arg = (Thread_arg *)arg;
    int * bucket = thr_arg->bucket;
    int socket_fd = thr_arg->socket_fd;
    int * numRequest = thr_arg->numRequest;
    string ip;
    int client_fd = server_accept(socket_fd, &ip);
    if (client_fd == -1) {
      cout << "Error in build server!\n";
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
    
    //delay loop
    delayloop(delay);

    //add delay count to certain bucket
    pthread_mutex_lock(&mutex);
    bucket[num] += delay;
    int currReq = ++(*numRequest);
    cout << "Request[" << *numRequest << "] received, "
         << "delay: " << delay << ", number of bucket: " << num << endl;
    pthread_mutex_unlock(&mutex);

    //send response back
    string l2 = to_string(bucket[num]) + "\n";
    const char * response = l2.c_str();
    cout << "bucket[" << num << "]: " << response;
    send(client_fd, response, strlen(response), 0);
    cout << "Request[" << currReq << "] processed" << endl;
    
    close(client_fd);
  }
}

// ./server threading_strategy
int main(int argc, char * argv[]) {
  if (argc != 2) {
    cerr << "Error: incorrect number of arguments" << endl;
    exit(EXIT_FAILURE);
  }

  int thrd = atoi(argv[1]);
  if (thrd != CREATE_PER_THREAD && thrd != PRE_CREATE) {
    cerr << "Error: invalid threading strategy" << endl;
    exit(EXIT_FAILURE);
  }
  //int size = atoi(argv[2]);

  //setup server
  int socket_fd = build_server("12345");
  string ip;
  int numThreads = 2000;
  int numRequest = 0;
  int bucket_fd = server_accept(socket_fd, &ip);
  int size;
  //receive bucket size
  recv(bucket_fd, &size, sizeof(size), 0);
  cout << size << endl;
  close(bucket_fd);
  int bucket[size] = {0};
  
  //start timing
  struct timespec start_time, end_time;
  clock_gettime(CLOCK_MONOTONIC, &start_time);
  
  if (thrd == CREATE_PER_THREAD) {
    while (1) {
      //connect with each client
      int client_fd = server_accept(socket_fd, &ip);
      if (client_fd == -1) {
        cout << "Error in build server!\n";
        continue;
      }
      //handle request
      pthread_t thread;
      Thread_arg * thr_arg = new Thread_arg();
      thr_arg->client_fd = client_fd;
      thr_arg->bucket = bucket;
      thr_arg->numRequest = &numRequest;
      pthread_create(&thread, NULL, procOneRequest, thr_arg);
      pthread_detach(thread);

      //check timing
      clock_gettime(CLOCK_MONOTONIC, &end_time);
      double elapsed = calc_time(start_time, end_time) / 1e9;
      if(elapsed > 60){
	cout << "Totally " << numRequest << " requests have been processed." << endl;
        return 0;
      }
    }
  }

  if (thrd == PRE_CREATE) {
    int numThreads = 1000;
    pthread_t * threads;
    threads = (pthread_t *)malloc(numThreads * sizeof(pthread_t));
    Thread_arg * thr_arg = new Thread_arg();
    thr_arg->bucket = bucket;
    thr_arg->socket_fd = socket_fd;
    thr_arg->numRequest = &numRequest;
    for (int i = 0; i < numThreads; i++) {
      pthread_create(&threads[i], NULL, procRequests, thr_arg);
    }
    while(1){
      //check timing
      clock_gettime(CLOCK_MONOTONIC, &end_time);
      double elapsed = calc_time(start_time, end_time) / 1e9;
      if(elapsed > 60){
	cout << "Totally " << numRequest << " requests have been processed." << endl;
        return 0;
      }
    }
    for (int i = 0; i < numThreads; i++) {
      pthread_join(threads[i], NULL);
    }
  }
  return 0;
}
