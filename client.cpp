#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string>
#include <cstdlib>

using namespace std;

// ./client delay_count num_of_bucket
int main(int argc, char *argv[]){
  if (argc != 3) {
    cerr << "Error: incorrect number of arguments" << endl;
    exit(EXIT_FAILURE);
  }

  int delay = atoi(argv[1]);
  int bucket = atoi(argv[2]);
  
  //setup client
  /////////////////////////////////////////////////////
  int status;
  int socket_fd;
  struct addrinfo host_info;
  struct addrinfo *host_info_list;
  const char *hostname = "0.0.0.0";
  const char *port     = "12345";
  
  memset(&host_info, 0, sizeof(host_info));
  host_info.ai_family   = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;

  status = getaddrinfo(hostname, port, &host_info, &host_info_list);
  if (status != 0) {
    cerr << "Error: cannot get address info for host" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    return -1;
  } //if

  socket_fd = socket(host_info_list->ai_family, 
		     host_info_list->ai_socktype, 
		     host_info_list->ai_protocol);
  if (socket_fd == -1) {
    cerr << "Error: cannot create socket" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    return -1;
  } //if
  
  cout << "Connecting to " << hostname << " on port " << port << "..." << endl;
  
  status = connect(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
  if (status == -1) {
    cerr << "Error: cannot connect to socket" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    return -1;
  } //if
  //////////////////////////////////////////////////////////

  srand((unsigned int)time(NULL));
  for(int i = 0; i < 100; i++){
    //send request
    int random = rand() % bucket;
    string l1 = to_string(delay) + "," + to_string(random) + "\n";
    const char *request = l1.c_str();
    cout << request << endl;
    send(socket_fd, request, strlen(request), 0);

    //receive response
    char response[20];
    memset(response, 0, sizeof(request));
    recv(socket_fd, response, sizeof(response), 0);
    string l2 = response;
    double value = stoi(l2);
    cout << "new value in bucket[" << random << "]: " << value << endl;
  }
  
  freeaddrinfo(host_info_list);
  close(socket_fd);

  return 0;
}
