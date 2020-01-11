#include <stdio.h>
#include "server.h"

int main() {
  char result[100];
  base64("abcdefg", result);
  printf("%s\n", result);

  uint32_t sha1r[6] = {};
  char sha1s[21] = {};
  char base64r[100];
  char* key = "dGhlIHNhbXBsZSBub25jZQ==258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
  sha1(key, sha1r);
  sha1_endian(sha1r);
  //sha1_to_chars(sha1r, sha1s);
  //printf("%s\n", sha1s);
  base64((char*)sha1r, base64r);
  printf("%s\n", base64r);
}
