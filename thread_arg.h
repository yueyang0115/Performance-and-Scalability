#include <pthread.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

class Thread_arg {
 public:
  int * bucket;
  int client_fd;
  int socket_fd;
  int delay_l;
  int delay_u;
  int size;
  int * numRequest;
  const char * hostname;
};
