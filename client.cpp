#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/time.h>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

#include "function.h"
#include "thread_arg.h"
using namespace std;
//pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

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

// ./client delay_count num_of_bucket
void * sendRequest(void * arg) {
  Thread_arg * thr_arg = (Thread_arg *)arg;
  int bucket = thr_arg->bucketID;
  int delay = thr_arg->delay;
  int threadID = thr_arg->threadID;
  int socket_fd = thr_arg->client_fd;

  //pthread_mutex_lock(&mutex);
  //send request
  int random = rand() % bucket;
  string l1 = to_string(delay) + "," + to_string(random) + "\n";
  const char * request = l1.c_str();
  cout << "thread " << threadID << " sends Request: " << request;
  send(socket_fd, request, strlen(request), 0);

  //receive response
  char response[20];
  memset(response, 0, sizeof(request));
  recv(socket_fd, response, sizeof(response), 0);
  string l2 = response;
  double value = stoi(l2);
  cout << "new value in bucket[" << random << "]: " << value << endl;
  close(socket_fd);

  //pthread_mutex_unlock(&mutex);
  return NULL;
}

int main(int argc, char * argv[]) {
  if (argc != 3) {
    cerr << "Error: incorrect number of arguments" << endl;
    exit(EXIT_FAILURE);
  }

  int delay = atoi(argv[1]);
  int bucket = atoi(argv[2]);

  srand((unsigned int)time(NULL));
  for (int i = 0; i < 100; i++) {
    //setup client
    const char * hostname = "0.0.0.0";
    const char * port = "12345";
    int socket_fd = build_client(hostname, port);
    if (socket_fd == -1) {
      std::cout << "Error in build client!\n";
    }

    cout << "create a thread, ID is : " << i << endl;
    pthread_t thread;
    Thread_arg * thr_arg = new Thread_arg();
    thr_arg->delay = delay;
    thr_arg->bucketID = bucket;
    thr_arg->threadID = i;
    thr_arg->client_fd = socket_fd;
    pthread_create(&thread, NULL, sendRequest, thr_arg);
  }

  delayloop(5);
  //freeaddrinfo(host_info_list);
  //close(socket_fd);

  return 0;
}
