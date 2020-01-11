#include <stdint.h>
#include <stdbool.h>

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
void write_response(int fd);
