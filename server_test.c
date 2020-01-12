#include "server.h"

void read_message(bool* ishandshake, int sock) {
  char data[DATA_BUFLEN];
  int datalen = read(sock, data, DATA_BUFLEN);
  if (datalen < 1) {
    sleep(100);
    return;
  }

  debug("%s\n", data);

  char protocol[100];
  get_protocol(data, protocol, 100);
  debug("protocol: %s\n", protocol);

  if (strcmp(protocol, "/ws") == 0) { // WebSocket handshake
    if (*ishandshake) {
      handshake(sock, data);
      *ishandshake = false;
    } else {
    }
  } else if (!*ishandshake) { // WebSocket messaging
    int opcode;
    uint64_t payloadlen;
    char* payload;
    decode_frame(data, &opcode, &payloadlen, &payload);

    write_data(sock, payload, payloadlen);
  } else if (strcmp(protocol, "/") == 0 || strcmp(protocol, "/index.html")) { // /index.html
    start_response();
    response("HTTP/1.1 200 OK\r\n");
    response("Content-Type: text/html\r\n");
    response("Connection: Close\r\n");
    response("\r\n");
    response_file("index.html");
    write_response(sock);
  }
}

void handler(int sock) {
  bool ishandshake = true;
  for (;;) {
    read_message(&ishandshake, sock);
  }
}

int main() {
  int sock = new_socket(8080);
  server_loop(sock, handler);
}
