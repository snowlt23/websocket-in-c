#include "server.h"

void get_protocol(char* data, char* result, int bufsize) {
  int len = length_at_char(data + 4, ' ');
  assert(len < bufsize);
  memcpy(result, data + 4, len);
  result[len] = '\0';
}
