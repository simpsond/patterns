//
// Created by Dustin Simpson on 6/25/23.
//

#include <proto/exec.h>

int main() {
  printf("realloc_test\n");

  void* buffer = AllocMem(10, MEMF_CLEAR);
  printf("buffer address: %p\n", buffer);
//  buffer = "hello";
  strcat(buffer, "hello");
  printf("buffer: %s\n", buffer);

  ReAllocMem(&buffer, 10, 20);
  printf("------\n");
  strcat(buffer, " world");
  printf("buffer address: %p\n", buffer);
  printf("buffer: %s\n", buffer);

}