#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

#include "function.h"
using namespace std;

// ./client delay_count num_of_bucket
int main(int argc, char * argv[]) {
  if (argc != 3) {
    cerr << "Error: incorrect number of arguments" << endl;
    exit(EXIT_FAILURE);
  }

  int delay = atoi(argv[1]);
  int bucket = atoi(argv[2]);
  const char * hostname = "0.0.0.0";
  const char * port = "12345";

  srand((unsigned int)time(NULL));
  for (int i = 0; i < 100; i++) {
    //setup client
    int socket_fd = build_client(hostname, port);
    if (socket_fd == -1) {
      std::cout << "Error in build client!\n";
      return -1;
    }

    //send request
    int random = rand() % bucket;
    string l1 = to_string(delay) + "," + to_string(random) + "\n";
    const char * request = l1.c_str();
    cout << request;
    send(socket_fd, request, strlen(request), 0);

    //receive response
    char response[20];
    memset(response, 0, sizeof(request));
    recv(socket_fd, response, sizeof(response), 0);
    string l2 = response;
    double value = stoi(l2);
    cout << "new value in bucket[" << random << "]: " << value << endl;
    close(socket_fd);
  }

  //freeaddrinfo(host_info_list);
  //close(socket_fd);

  return 0;
}
