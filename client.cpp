#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

#include "socket.h"
#include "thread_arg.h"
using namespace std;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void * sendRequest(void * arg) {
  Thread_arg * thr_arg = (Thread_arg *)arg;
  int size = thr_arg->size;
  int delay_l = thr_arg->delay_l;
  int delay_u = thr_arg->delay_u;
  int * numRequest = thr_arg->numRequest;
  //int socket_fd = thr_arg->client_fd;

  while (1) {
    //setup client
    const char * hostname = "0.0.0.0";
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

// ./client delay_l delay_u num_of_buckets num_of_threads
int main(int argc, char * argv[]) {
  if (argc != 5) {
    cerr << "Error: incorrect number of arguments" << endl;
    exit(EXIT_FAILURE);
  }

  int delay_l = atoi(argv[1]);
  int delay_u = atoi(argv[2]);
  int size = atoi(argv[3]);

  pthread_t * threads;
  int numThreads = atoi(argv[4]);
  threads = (pthread_t *)malloc(numThreads * sizeof(pthread_t));

  int numRequest = 0;
  srand((unsigned int)time(NULL));
  for (int i = 0; i < numThreads; i++) {
    Thread_arg * thr_arg = new Thread_arg();
    thr_arg->delay_l = delay_l;
    thr_arg->delay_u = delay_u;
    thr_arg->size = size;
    thr_arg->numRequest = &numRequest;
    pthread_create(&threads[i], NULL, sendRequest, thr_arg);
  }
  for (int i = 0; i < numThreads; i++) {
    pthread_join(threads[i], NULL);
  }

  return 0;
}
