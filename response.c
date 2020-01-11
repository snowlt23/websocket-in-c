#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

char response_buffer[1024*1024];
char* response_ptr;
void start_response() {
  response_ptr = response_buffer;
}

void response(char* s) {
  int len = strlen(s);
  memcpy(response_ptr, s, len);
  response_ptr += len;
}

void write_response(int fd) {
  //printf("%.*s\n", response_ptr - response_buffer, response_buffer);
  write(fd, response_buffer, response_ptr - response_buffer);
}
