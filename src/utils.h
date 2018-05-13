#ifndef DEV_UTILS_H
  #define DEV_UTILS_H
  #ifndef DEBUG
    #define DEBUG 1
  #endif
  #if DEBUG
    #include<stdio.h>
    #define PRINT(...) printf(__VA_ARGS__)
  #else
    #define PRINT(x) ;
  #endif
  #define MAX_INFO_TO_SEND_SIZE 100
  #define START_MSG "start\0"
  #define OFFSET_OUTPUT_MSG 'a'
#endif
