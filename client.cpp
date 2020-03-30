#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

#include "function.h"
#include "thread_arg.h"
using namespace std;
//pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// ./client delay_count num_of_bucket
void * sendRequest(void * arg) {
  Thread_arg * thr_arg = (Thread_arg *)arg;
  int bucket = thr_arg->bucketID;
  int delay = thr_arg->delay;
  int threadID = thr_arg->threadID;
  int socket_fd = thr_arg->client_fd;

  
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

  
  return NULL;
}

int main(int argc, char * argv[]) {
  if (argc != 3) {
    cerr << "Error: incorrect number of arguments" << endl;
    exit(EXIT_FAILURE);
  }

  int delay = atoi(argv[1]);
  int bucket = atoi(argv[2]);

  pthread_t * threads;
  int numThreads = 500;
  threads = (pthread_t *)malloc(numThreads * sizeof(pthread_t));

  srand((unsigned int)time(NULL));
  for (int i = 0; i < numThreads; i++) {
    //setup client
    const char * hostname = "0.0.0.0";
    const char * port = "12345";
    int socket_fd = build_client(hostname, port);
    if (socket_fd == -1) {
      std::cout << "Error in build client!\n";
    }

    cout << "create a thread, ID is : " << i << endl;
    Thread_arg * thr_arg = new Thread_arg();
    thr_arg->delay = delay;
    thr_arg->bucketID = bucket;
    thr_arg->threadID = i;
    thr_arg->client_fd = socket_fd;
    pthread_create(&threads[i], NULL, sendRequest, thr_arg);
  }
  for (int i = 0; i < numThreads; i++) {
    pthread_join(threads[i], NULL);
  }

  //freeaddrinfo(host_info_list);
  //close(socket_fd);

  return 0;
}
