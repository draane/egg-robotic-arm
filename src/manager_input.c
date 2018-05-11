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

#define MAX_PINS 8

typedef int pinstatus;

#define READ_PIPE 0
#define WRITE_PIPE 1

#define ON 1
#define OFF !ON

typedef struct pidpipe{
  int  pipe[2];
  pid_t pid;
  int value;
} pidpipe;

pidpipe PIN_PIPE_PID[MAX_PINS];

//Kills all sons, used when a fatal error occurres or just
//when the process has to be terminated
void kill_all_sons(int limit){
  for(int i = 0; i < limit; i++){
    kill(PIN_PIPE_PID[i].pid,SIGKILL);
  }
}

int read_pin(int n){
  return n; 
}

void child_pin_reader(int n,int p[2]){
  for(;;){
    close(p[READ_PIPE]);
    int val = read_pin(n);
    write(p[WRITE_PIPE],&val,sizeof(int));
    sleep(5);
  }
}

void input_manager(void){
  for(int i = 0; i<MAX_PINS; i++){
    int res = -1;
    int bytes = read(PIN_PIPE_PID[i].pipe[READ_PIPE], &res, sizeof(int));
    PRINT("Readed from i > %i\n", i);
  }
  PRINT("Started all process, destruction of the earth now being done\n");
  fflush(stdin);
  kill_all_sons(MAX_PINS);
}

void create_process(int i){
  if(i == MAX_PINS){
    return; 
  }else{
    PRINT("Starting %i\n", i);
    pipe(PIN_PIPE_PID[i].pipe);
    int pid = fork();
    if(pid < 0){
      PRINT("Fatal error occurred in the creation of a childi\n");
      exit(1);
    }else if(pid == 0){ // child;
      child_pin_reader(i,PIN_PIPE_PID[i].pipe);
      return;
    }else{
      close(PIN_PIPE_PID[i].pipe[WRITE_PIPE]);
      PIN_PIPE_PID[i].pid = pid;
      create_process(i+1);
    }
  }
}

void start_input(void){
  PRINT("Input reader started\n\n");
  create_process(0);
  input_manager(); 
}
