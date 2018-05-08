#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include "manager_output.h"
#include "output_settings.h"

#define DEBUG 1

#if DEBUG
#include<stdio.h>
#define PRINT(...) printf(__VA_ARGS__)
#else
#define PRINT(x) ;
#endif

void kill_all_sons(int* childs_pid, int max_index) {
  PRINT("KILLING ALL OUTPUT_PIN PROCESSES...");
  int i = 0;
  for (; i < max_index; i++)
    kill(childs_pid[i], SIGKILL);
  free(childs_pid);
  PRINT("DONE\n");
}

void output_manager(int* childs_pid){
  PRINT("Starting Output Manager process...DONE\n");
  kill_all_sons(childs_pid, OUTPUT_PIN_NUMBER);
}

void start_output(void) {
  PRINT("Starting output processes...\n");
  int output_pin[] = {DEFAULT_OUTPUT_PIN1, DEFAULT_OUTPUT_PIN2, DEFAULT_OUTPUT_PIN3, DEFAULT_OUTPUT_PIN4, DEFAULT_OUTPUT_PIN5, DEFAULT_OUTPUT_PIN6, DEFAULT_OUTPUT_PIN7, DEFAULT_OUTPUT_PIN8};
  int* output_pin_pid = malloc(sizeof(int)*OUTPUT_PIN_NUMBER);

  for(int i = 0; i<OUTPUT_PIN_NUMBER; i++){
    PRINT("Starting child process %i...", i + 1);
    int pid = fork();
    if (pid == -1) {
      PRINT("Error at spawning child, closing\n");
      kill_all_sons(output_pin_pid, i-1);
      exit(1);
    }
    else if(pid == 0){  //Son process
      PRINT("HELLOOOO");
      pause();
    }else{
      output_pin_pid[i] = pid;
      PRINT("DONE\n");
    }
  }
  output_manager(output_pin_pid);
}
