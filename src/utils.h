#ifndef DEBUG
#define DEBUG 1
#endif

#if DEBUG
#include<stdio.h>
#define PRINT(...) printf(__VA_ARGS__)
#else
#define PRINT(x) ;
#endif

#define READ_PIPE 0
#define WRITE_PIPE 1
#define MAX_INFO_TO_SEND_SIZE 100
