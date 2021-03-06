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

  #define READ_PIPE 0
  #define WRITE_PIPE 1
  #define MAX_INFO_TO_SEND_SIZE 100
  #define DIM_OF_MSG_PIPE 2
  #define NUM_PARAMETERS_RECEIVED 3

  #define START_MSG "1"
  #define OFFSET_OUTPUT_MSG 'a'

  #define NUMBER_OF_OUTPUT_BYTE 1
  #define NUMBER_OF_EGGS_IN_THE_BOX 6

  // Comment when not compiling and executing on a Raspberry.
  #define ON_THE_RASPBERRY
  #define NUM_PINS 8

  // Comment when not using the robotic arm.
  // #define ARM_INSTALLED
#endif
