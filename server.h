#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <assert.h>

#define HOSTNAME_BUFLEN 256
#define DATA_BUFLEN 8192

#define DEBUG

#define error(...) {fprintf(stderr, __VA_ARGS__); exit(1);}
#ifdef DEBUG
  #define debug(...) {printf(__VA_ARGS__);}
#else
  #define debug(...)
#endif

typedef void (*Handler)(int sock);

// utils.c
uint64_t endian64(uint64_t x);
bool eq_string(char* a, char* b, int len);
char* search_param(char* s, char* p);
int length_at_char(char* s, char c);

// sha1.c
void sha1(char* input, uint32_t* result);
void sha1_to_chars(uint32_t* sha, char* result);
void sha1_endian(uint32_t* result);
// base64.c
void base64(char* input, char* result);

// response.c
void start_response();
void response(char* s);
void response_file(char* filename);
void write_response(int fd);

// http.c
void get_protocol(char* data, char* result, int bufsize);

// websocket.c
void encode_seckey(char* ckey, char* result);
void handshake(int sock, char* data);
uint64_t encode_frame(unsigned char* data, int opcode, char* payload, uint64_t payloadlen);
void write_data(int sock, char* msg, int msgsize);
void write_message(int sock, char* message);
void decode_frame(char* data, int* opcode, uint64_t* payloadlen, char** payload);

// server.c
int new_socket(uint16_t port);
void server_loop(int sock, Handler handler);
