#include "server.h"
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

char* handshake_response = "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\n";
char* handshake_constant = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

void encode_seckey(char* ckey, char* result) {
  char akey[256] = {};
  int ckeylen = length_at_char(ckey, '\r');
  assert(ckeylen + strlen(handshake_constant) < 255);
  memcpy(akey, ckey, ckeylen);
  strcpy(akey + strlen(akey), handshake_constant);
  uint32_t sha1_result[6] = {};
  sha1(akey, sha1_result);
  sha1_endian(sha1_result);
  base64((char*)sha1_result, result);
}

void handshake_with_seckey(int sock, char* sec_key) {
  start_response();
  response(handshake_response);
  response("Sec-WebSocket-Accept: ");
  response(sec_key);
  response("\r\n");
  response("\r\n");
  write_response(sock);
}

void handshake(int sock, char* data) {
  char* ckey = search_param(data, "Sec-WebSocket-Key: ");
  if (ckey == NULL) return;
  char seckey[1024];
  encode_seckey(ckey, seckey);
  handshake_with_seckey(sock, seckey);
}

uint64_t encode_frame(unsigned char* data, int opcode, char* payload, uint64_t payloadlen) {
  unsigned char* startdata = data;
  *data = 0b10000000 | (uint8_t)opcode;
  data += 1;
  if (payloadlen <= 125) {
    *data = (uint8_t)payloadlen;
    data += 1;
  } else if (payloadlen <= 65535) {
    *data = 126;
    data += 1;
    *(uint16_t*)data = htons((uint16_t)payloadlen);
    data += 2;
  } else {
    *data = 127;
    data += 1;
    *(uint64_t*)data = endian64(payloadlen);
    data += 8;
  }
  memcpy(data, payload, payloadlen);
  data += payloadlen;
  return (uint64_t)(data - startdata);
}

void write_data(int sock, char* msg, int msgsize) {
  unsigned char data[DATA_BUFLEN];
  uint64_t datasize = encode_frame(data, 1, msg, msgsize);
  write(sock, data, datasize);
}

void write_message(int sock, char* message) {
  unsigned char data[DATA_BUFLEN];
  uint64_t datasize = encode_frame(data, 1, message, strlen(message));
  write(sock, data, datasize);
}

void decode_frame(char* data, int* opcode, uint64_t* payloadlen, char** payload) {
  *opcode = (*(uint8_t*)data) & 0b1111;
  data += 1;

  bool hasmask = (*(uint8_t*)data) >> 7;
  *payloadlen = (*(uint8_t*)data) & 0b1111111;
  data += 1;

  if (*payloadlen == 126) { // <= 65535
    *payloadlen = ntohs(*(uint16_t*)data);
    data += 2;
  } else if (*payloadlen == 127) { // > 65535
    *payloadlen = endian64(*(uint64_t*)data);
    data += 8;
  }

  uint8_t* mask;
  if (hasmask) {
    mask = data;
    data += 4;
  }

  *payload = data;
  if (hasmask) {
    for (int i=0; i<*payloadlen; i++) {
      ((uint8_t*)*payload)[i] ^= mask[i % 4];
    }
  }
}
