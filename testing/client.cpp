#include <netdb.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

#include "socket.h"
#include "thread_arg.h"
using namespace std;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

double calc_time(struct timespec start, struct timespec end) {
  double start_sec = (double)start.tv_sec * 1000000000.0 + (double)start.tv_nsec;
  double end_sec = (double)end.tv_sec * 1000000000.0 + (double)end.tv_nsec;

  if (end_sec < start_sec) {
    return 0;
  }
  else {
    return end_sec - start_sec;
  }
}

void * sendRequest(void * arg) {
  Thread_arg * thr_arg = (Thread_arg *)arg;
  int size = thr_arg->size;
  int delay_l = thr_arg->delay_l;
  int delay_u = thr_arg->delay_u;
  int * numRequest = thr_arg->numRequest;
  const char * hostname = thr_arg->hostname;

  while (1) {
    //setup client
    // const char * hostname = "0.0.0.0";
    const char * port = "12345";
    int socket_fd = build_client(hostname, port);
    if (socket_fd == -1) {
      std::cout << "Error in build client!\n";
    }

    //send request
    int random = rand() % size;
    int delay = rand() % (delay_u - delay_l + 1) + delay_l;
    string l1 = to_string(delay) + "," + to_string(random) + "\n";
    const char * request = l1.c_str();
    send(socket_fd, request, strlen(request), 0);
    pthread_mutex_lock(&mutex);
    int currReq = ++(*numRequest);
    cout << "send Request[" << currReq << "]: " << request;
    pthread_mutex_unlock(&mutex);

    //receive response
    char response[20];
    memset(response, 0, sizeof(request));
    int len = recv(socket_fd, response, sizeof(response), 0);
    if (len < 0) {
      return NULL;
    }

    string l2 = response;
    double value = stoi(l2);
    //cout << "Request[" << currReq << "] processed" << endl;
    cout << "new value in bucket[" << random << "]: " << value << endl;
    close(socket_fd);
  }

  return NULL;
}

// ./client hostname delay_l delay_u num_of_buckets num_of_threads
int main(int argc, char * argv[]) {
  if (argc != 6) {
    cerr << "Error: incorrect number of arguments" << endl;
    exit(EXIT_FAILURE);
  }

  const char * hostname = argv[1];
  int delay_l = atoi(argv[2]);
  int delay_u = atoi(argv[3]);
  int size = atoi(argv[4]);

  //setup client
  //const char * hostname = "0.0.0.0";
  const char * port = "12345";
  int socket_fd = build_client(hostname, port);
  if (socket_fd == -1) {
    std::cout << "Error in build client!\n";
  }

  //send bucket size
  send(socket_fd, &size, sizeof(size), 0);
  close(socket_fd);

  pthread_t * threads;
  int numThreads = atoi(argv[5]);
  threads = (pthread_t *)malloc(numThreads * sizeof(pthread_t));

  int numRequest = 0;
  srand((unsigned int)time(NULL));

  //start timing
  struct timespec start_time, end_time;
  clock_gettime(CLOCK_MONOTONIC, &start_time);

  for (int i = 0; i < numThreads; i++) {
    Thread_arg * thr_arg = new Thread_arg();
    thr_arg->hostname = hostname;
    thr_arg->delay_l = delay_l;
    thr_arg->delay_u = delay_u;
    thr_arg->size = size;
    thr_arg->numRequest = &numRequest;
    pthread_create(&threads[i], NULL, sendRequest, thr_arg);
  }
  while (1) {
    //check timing
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    double elapsed = calc_time(start_time, end_time) / 1e9;
    if (elapsed > 60) {
      cout << "Totally " << numRequest << " requests have been sent." << endl;
      return 0;
    }
  }
  for (int i = 0; i < numThreads; i++) {
    cout << i << endl;
    pthread_join(threads[i], NULL);
  }

  return 0;
}
