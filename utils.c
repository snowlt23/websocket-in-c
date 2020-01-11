#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

uint64_t endian64(uint64_t x) {
  return
    x >> 56 & 0xFF +
    x >> 48 & 0xFF +
    x >> 32 & 0xFF +
    x >> 24 & 0xFF +
    x >> 16 & 0xFF +
    x >>  8 & 0xFF +
    x       & 0xFF;
}

//
// string utils
//

bool eq_string(char* a, char* b, int len) {
  for (int i=0; i<len; i++) {
    if (a[i] != b[i]) return false;
  }
  return true;
}

char* search_param(char* s, char* p) {
  int len = strlen(s);
  int plen = strlen(p);
  for (int i=0; i<len; i++) {
    if (i + plen >= len) break;
    if (eq_string(&s[i], p, plen)) return &s[i + plen];
  }
  return NULL;
}

int length_at_char(char* s, char c) {
  for (int i=0; ; i++) {
    if (s[i] == c) return i;
  }
  return -1;
}
