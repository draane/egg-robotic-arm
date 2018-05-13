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

  // Hubi&Costa's stuff here.
  #define READ_PIPE 0
  #define WRITE_PIPE 1
  #define MAX_INFO_TO_SEND_SIZE 100
  #define NUM_PARAMETERS_RECEIVED 3

  // Max's stuff here
  #define START_MSG "start\0"
  #define OFFSET_OUTPUT_MSG 'a'
#endif