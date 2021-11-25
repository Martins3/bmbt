#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  while (1) {
    printf("Hello world!\n");
    sleep(1);
  }
  return 0;
}
