#include <stdio.h>
#include "sha1.c"

int main() {
  uint32_t result[5];
  char* input = "fafafa";
  sha1(input, result);
  for (int i=0; i<5; i++) {
    printf("%04x", result[i]);
  }
}
