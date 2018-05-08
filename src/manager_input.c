#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<signal.h>
#include<sys/types.h>
#include"manager_input.h"
#include"utils.h"

#define MAX_PINS 8

pid_t PIN_PID[MAX_PINS];

//Kills all sons, used when a fatal error occurres or just
//when the process has to be terminated
void kill_all_sons(int limit){
  for(int i = 0; i < limit; i++){
    kill(PIN_PID[i],SIGKILL);
  }
}

void input_manager(void){
  PRINT("Started all process, destruction of the earth now being done\n");
  kill_all_sons(MAX_PINS);
}

void start_input(void){
  PRINT("Input reader started\n\n");
  for(int i = 0; i<MAX_PINS; i++){
    PRINT("Starting process %i\n", i + 1);
    int l_pid = fork();
    if(l_pid == 0){  //Son process
     pause(); 
    }else if(l_pid == -1){
      PRINT("Fatal error occurred in the creation of the %i process, aborting\n");
      kill_all_sons(i);      
      return;
    }else{
      PIN_PID[i] = l_pid;
      PRINT("Process %i created successfully\n", l_pid);
    }
  }
  input_manager(); 
}
