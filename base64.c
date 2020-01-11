#include <stdint.h>
#include <string.h>

char* base64_table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void base64(char* input, char* result) {
  int bit = 0;
  int bitsi = 0;
  int bits[1000] = {};
  for (int i=0; i<strlen(input); i++) {
    char c = input[i];
    for (int j=0; j<8; j++) {
      bits[bitsi] |= (c >> (7-j) & 0b1) << (5-bit);
      bit++;
      if (bit == 6) {
        bit = 0;
        bitsi++;
      }
    }
  }
  if (bit != 0) bitsi++;

  for (int i=0; i<bitsi; i++) {
    result[i] = base64_table[bits[i]];
  }

  int eqn = 4 - (bitsi%4);
  for (int i=bitsi; i<bitsi + eqn; i++) {
    result[i] = '=';
  }
  result[bitsi+eqn] = '\0';
}
