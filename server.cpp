#include <netdb.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

#include "function.h"
#include "thread_arg.h"
using namespace std;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void delayloop(double req_delay) {
  struct timeval start, check, end;
  double elapsed_seconds;
  gettimeofday(&start, NULL);
  do {
    gettimeofday(&check, NULL);
    elapsed_seconds = (check.tv_sec + (check.tv_usec / 1000000.0)) -
                      (start.tv_sec + (start.tv_usec / 1000000.0));
  } while (elapsed_seconds < req_delay);
}

void * processRequest(void * arg) {
  Thread_arg * thr_arg = (Thread_arg *)arg;
  int client_fd = thr_arg->client_fd;
  int * bucket = thr_arg->bucket;

  pthread_mutex_lock(&mutex);

  //receive request
  char request[20];
  memset(request, 0, sizeof(request));
  recv(client_fd, request, sizeof(request), 0);
  cout << "request: " << request;

  //parse the request
  string l1 = request;
  double delay = stoi(l1);
  cout << "delay: " << delay << endl;
  int num = stoi(l1.substr(l1.find(",") + 1));
  cout << "number of bucket: " << num << endl;

  //delay loop
  delayloop(delay);

  //add delay count to certain bucket
  bucket[num] += delay;

  //send response back
  string l2 = to_string(bucket[num]) + "\n";
  const char * response = l2.c_str();
  cout << "response: " << response;
  send(client_fd, response, strlen(response), 0);

  pthread_mutex_unlock(&mutex);
  return NULL;
}

// ./server num_of_cores threading_strategy num_of_buckets

int main(int argc, char * argv[]) {
  if (argc != 4) {
    cerr << "Error: incorrect number of arguments" << endl;
    exit(EXIT_FAILURE);
  }

  int cores = atoi(argv[1]);
  int threads = atoi(argv[2]);
  int size = atoi(argv[3]);
  int bucket[size] = {0};

  //setup server
  int socket_fd = build_server("12345");
  string ip;

  //handle request
  for (int i = 0; i < 10; i++) {
    //connect with each client
    int client_fd = server_accept(socket_fd, &ip);
    if (client_fd == -1) {
      std::cout << "Error in build server!\n";
      return -1;
    }
    cout << "socket " << i << " created" << endl;
    //create a thread per request
    pthread_t thread;
    Thread_arg * thr_arg = new Thread_arg();
    thr_arg->client_fd = client_fd;
    thr_arg->bucket = bucket;
    pthread_create(&thread, NULL, processRequest, thr_arg);
  }

  //freeaddrinfo(host_info_list);
  //close(socket_fd);

  return 0;
}
