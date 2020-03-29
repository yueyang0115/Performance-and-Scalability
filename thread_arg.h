#include <pthread.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

class Thread_arg {
 public:
  int * bucket;
  int client_fd;
  int delay;
  int bucketID;
  int threadID;
};
