#include <pthread.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

class Thread_arg {
 public:
  double * bucket;
  int client_fd;
};
