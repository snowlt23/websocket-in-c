#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

#define CIRCULAR_SHIFT(bits, word) (((word)<<(bits)) | ((word)>>(32-bits)))

uint32_t f(int t, uint32_t b, uint32_t c, uint32_t d) {
	if (t<20) return (b & c) | ((~ b) & d);
	else if (t<40) return b ^ c ^ d;
	else if (t<60) return (b & c) | (b & d) | (c & d);
	else return b ^ c ^ d;
}

uint32_t k(int t) {
	if (t<20) return 0x5A827999;
	else if (t<40) return 0x6ED9EBA1;
	else if (t<60) return 0x8F1BBCDC;
	else return 0xCA62C1D6;
}

void sha1_block(uint32_t* w, uint32_t* h) {
  uint32_t a, b, c, d, e;
  
  for (int t=16; t<80; t++) {
    w[t] = CIRCULAR_SHIFT(1, w[t-3] ^ w[t-8] ^ w[t-14] ^ w[t-16]);
  }
  a = h[0]; b = h[1]; c = h[2]; d = h[3]; e = h[4];
  for (int t=0; t<80; t++) {
    uint32_t tmp = CIRCULAR_SHIFT(5, a) + f(t, b, c, d) + e + w[t] + k(t);
    e = d; d = c; c = CIRCULAR_SHIFT(30, b); b = a; a = tmp;
  }
  h[0] += a; h[1] += b; h[2] += c; h[3] += d; h[4] += e;
}

void sha1_magic(uint32_t* result) {
	result[0] = 0x67452301;
	result[1] = 0xEFCDAB89;
	result[2] = 0x98BADCFE;
	result[3] = 0x10325476;
	result[4] = 0xC3D2E1F0;
}

void sha1(char* input, uint32_t* result) {
  uint32_t block[80];
  unsigned char lastblock[64] = {};
  uint32_t len = strlen(input);
  int blockn = len/64;
  int lastn = len-64*blockn;
  sha1_magic(result);

  for (int i=0; i<blockn; i++) {
    for (int t=0; t<16; t++) {
      block[t] =  input[i*64+t*4] << 24;
      block[t] |= input[i*64+t*4+1] << 16;
      block[t] |= input[i*64+t*4+2] << 8;
      block[t] |= input[i*64+t*4+3];
    }
    sha1_block(block, result);
  }

  memcpy(lastblock, input+blockn*64, lastn);
  if (lastn > 55) {
    lastblock[lastn] = 0x80;
    for (int t=0; t<16; t++) {
      block[t] =  lastblock[t*4] << 24;
      block[t] |= lastblock[t*4+1] << 16;
      block[t] |= lastblock[t*4+2] << 8;
      block[t] |= lastblock[t*4+3];
    }
    sha1_block(block, result);
    for (int i=0; i<60; i++) lastblock[i] = 0;
  } else {
    lastblock[lastn] = 0x80;
  }

	lastblock[60] = len*8>>24;
	lastblock[61] = len*8>>16;
	lastblock[62] = len*8>>8;
	lastblock[63] = len*8;

  for (int t=0; t<16; t++) {
    block[t] =  lastblock[t*4] << 24;
    block[t] |= lastblock[t*4+1] << 16;
    block[t] |= lastblock[t*4+2] << 8;
    block[t] |= lastblock[t*4+3];
  }
  sha1_block(block, result);
}

void sha1_endian(uint32_t* result) {
  result[0] = htonl(result[0]);
  result[1] = htonl(result[1]);
  result[2] = htonl(result[2]);
  result[3] = htonl(result[3]);
  result[4] = htonl(result[4]);
}

void sha1_to_chars(uint32_t* sha, char* result) {
  for (int i=0; i<5; i++) {
    sprintf(result+i*8, "%x", sha[i]);
  }
}
