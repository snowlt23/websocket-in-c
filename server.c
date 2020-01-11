#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "server.h"

#define error(...) {fprintf(stderr, __VA_ARGS__); exit(1);}
#define debug(...) {printf(__VA_ARGS__);}

#define HOSTNAME_BUFLEN 256
#define DATA_BUFLEN 8192

char* handshake_response = "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\n";

//
// socket
//

void get_protocol(char* data, char* result) {
  int len = length_at_char(data + 4, ' ');
  memcpy(result, data + 4, len);
  result[len] = '\0';
}

int new_socket(uint16_t port) {
  char hostname[HOSTNAME_BUFLEN];
  if (gethostname(hostname, HOSTNAME_BUFLEN) == -1) {
    error("can't get a hostname.");
  }
  debug("hostname: %s\n", hostname);

  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock == -1) {
    error("can't create socket.");
  }
  
  int opt = 1;
  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int)) != 0) {
    error("can't set option to socket");
  }

  struct sockaddr_in saddr = {};
  saddr.sin_family = AF_INET;
  saddr.sin_addr.s_addr = INADDR_ANY;
  saddr.sin_port = htons(port);

  if (bind(sock, (struct sockaddr *)&saddr, sizeof(saddr)) == -1) {
    error("can't bind socket to address");
  }

  return sock;
}

void encode_seckey(char* ckey, char* result) {
    char akey[256] = {};
    memcpy(akey, ckey, length_at_char(ckey, '\r'));
    strcpy(akey + strlen(akey), "258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
    uint32_t sha1_result[6] = {};
    sha1(akey, sha1_result);
    sha1_endian(sha1_result);
    base64((char*)sha1_result, result);
}

void handshake(int sock, char* sec_key) {
  start_response();
  response(handshake_response);
  response("Sec-WebSocket-Accept: ");
  response(sec_key);
  response("\r\n");
  //response("Sec-WebSocket-Protocol: ");
  //response(protocol);
  //response("\r\n");
  response("\r\n");
  write_response(sock);
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

void read_message(bool* ishandshake, int sock) {
  char data[DATA_BUFLEN];
  int datalen = read(sock, data, DATA_BUFLEN);
  if (datalen < 1) {
    sleep(100);
    return;
  }

  printf("%s\n", data);

  if (*ishandshake) {
    //char protocol[100];
    //get_protocol(data, protocol);
    //printf("protocol: %s\n", protocol);

    char* ckey = search_param(data, "Sec-WebSocket-Key: ");
    char seckey[1024];
    encode_seckey(ckey, seckey);
    handshake(sock, seckey);
    *ishandshake = false;
    return;
  } else {
    int opcode;
    uint64_t payloadlen;
    char* payload;
    decode_frame(data, &opcode, &payloadlen, &payload);
    printf("%d %ld\n%.*s\n", opcode, payloadlen, payloadlen, payload);
    write_data(sock, payload, payloadlen);
  }
}

void child_connection(int sock) {
  bool ishandshake = true;
  for (;;) {
    read_message(&ishandshake, sock);
  }
}

void server_loop(int sock) {
  struct sockaddr_in caddr;
  
  if (listen(sock, SOMAXCONN) == -1) {
    error("can't listen on socket");
  }

  for (;;) {
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(sock, &readfds);
    int width = sock+1;

    struct timeval timeout = {};
    timeout.tv_sec = 5;

    int sresp = select(width, &readfds, NULL, NULL, &timeout);
    if (sresp && sresp != -1) {
      if (FD_ISSET(sock, &readfds)) {
        int len = sizeof(caddr);
        int fork_sock = accept(sock, (struct sockaddr *)&caddr, &len);
        if (fork_sock < 0) {
          fprintf(stderr, "can't accept connections.");
          continue;
        }

        int pid = fork();
        if (pid == 0) { // if child process
          close(sock);
          //transaction_number++;
          child_connection(fork_sock);
          exit(0);
        }

        // parent process
        setpgid(pid, getpid());
        close(fork_sock);
      }
    }
  }
}

int main() {
  int sock = new_socket(8080);
  server_loop(sock);
}
