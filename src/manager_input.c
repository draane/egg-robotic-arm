#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<signal.h>
#include<sys/types.h>
#include<semaphore.h>

#include"manager_input.h"
#include"utils.h"

#define PRESENT 1
#define EMPTY !PRESENT 

typedef int status_t;

#define MAX_PINS 8

struct pin_status{
  status_t pin_status;
  sem_t ready;
};

pid_t PIN_PID[MAX_PINS];

//Kills all sons, used when a fatal error occurres or just
//when the process has to be terminated
void kill_all_sons(int limit){
  for(int i = 0; i < limit; i++){
    kill(PIN_PID[i],SIGKILL);
  }
}

void child_pin_reader(int i){
  PRINT("I'm process %i, just started\n");
  pause();
}

void input_manager(void){
  PRINT("Started all process, destruction of the earth now being done\n");
  kill_all_sons(MAX_PINS);
}

void start_input(void){
  PRINT("Input reader started\n\n");
  for(int i = 0; i<MAX_PINS; i++){
    PRINT("Starting process %i\n", i + 1);
    int loc_i = i;
    int l_pid = fork();
    if(l_pid == 0){  //Son process
      child_pin_reader(loc_i); 
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
