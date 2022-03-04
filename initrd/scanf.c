#include <assert.h>  // assert
#include <limits.h>  // INT_MAX
#include <math.h>    // sqrt
#include <stdbool.h> // bool false true
#include <stdio.h>
#include <stdlib.h> // malloc sort
#include <string.h> // strcmp ..
#include <unistd.h> // sleep

int main(int argc, char *argv[]) {
  char x[1000];
  printf("enter userspace :%s \n", __FUNCTION__);
  while (true) {
    scanf("%s", x);
    printf("huxueshi:%s %s\n", __FUNCTION__, x);
  }
  return 0;
}
