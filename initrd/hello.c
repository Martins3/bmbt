#include <assert.h>  // assert
#include <limits.h>  // INT_MAX
#include <math.h>    // sqrt
#include <stdbool.h> // bool false true
#include <stdio.h>
#include <stdlib.h> // malloc sort
#include <string.h> // strcmp ..
#include <unistd.h> // sleep

int main(int argc, char *argv[]) {
  for (int i = 0; i < 1000; ++i) {
    printf("huxueshi:%s loongson %d\n", __FUNCTION__, i);
    sleep(1);
  }
  exit(0);
  return 0;
}
