#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

char response_buffer[1024*1024];
char* response_ptr;
void start_response() {
  response_ptr = response_buffer;
}

void response_data(char* data, size_t size) {
  memcpy(response_ptr, data, size);
  response_ptr += size;
}

void response(char* s) {
  response_data(s, strlen(s));
}

void write_response(int fd) {
  //printf("%.*s\n", response_ptr - response_buffer, response_buffer);
  write(fd, response_buffer, response_ptr - response_buffer);
}

int response_file(char* filename) {
  char buff[4096];
  int fd = open(filename, O_RDONLY);
  if (fd == -1) return 0;
  for (;;) {
    int size = read(fd, buff, 4096);
    if (size < 1) break;
    response_data(buff, size);
  }
  close(fd);
  return 1;
}
