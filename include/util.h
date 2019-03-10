#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <stdlib.h>

inline static void err_exit(const char *msg) {
  printf("[Fatal Error]: %s \nExiting... \n", msg);
  exit(1);
}

#endif