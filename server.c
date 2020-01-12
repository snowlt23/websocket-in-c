#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "server.h"

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

void server_loop(int sock, Handler handler) {
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
          handler(fork_sock);
          exit(0);
        }

        // parent process
        setpgid(pid, getpid());
        close(fork_sock);
      }
    }
  }
}
