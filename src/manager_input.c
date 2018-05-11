#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<signal.h>
#include<sys/types.h>

#include"manager_input.h"
#include"utils.h"

#define MAX_PINS 8

typedef int pinstatus;

#define READ_PIPE 0
#define WRITE_PIPE 1

#define UPDATE_SIGNAL SIGUSR1
#define ON 1
#define OFF !ON
#define NO_STATUS 100
typedef int pin_status_t;
typedef int pipe_t[2];

typedef struct pidpipe{
  pipe_t  pipe;
  pid_t pid;
} pidpipe;

//this area of the code is used only by the child processes
pin_status_t to_send = NO_STATUS;
int *my_pipe;

//Kills all sons, used when a fatal error occurres or just
//when the process has to be terminated
void kill_all_sons(int limit, pidpipe PIN_PIPE_PID[MAX_PINS]){
  for(int i = 0; i < limit; i++){
    kill(PIN_PIPE_PID[i].pid,SIGKILL);
  }
}

pin_status_t read_pin(int n){
  return n;
}

void child_pin_reader(int n,int p[2]){
  for(;;){
    pin_status_t val = read_pin(n);
    to_send = val;
  }
}

void input_manager(pidpipe PIN_PIPE_PID[MAX_PINS]){
  for(int i = 0; i<MAX_PINS; i++){
    int res = NO_STATUS;
    while(res == NO_STATUS){
      kill(PIN_PIPE_PID[i].pid, UPDATE_SIGNAL);
      int bytes = read(PIN_PIPE_PID[i].pipe[READ_PIPE], &res, sizeof(int));
    }
    PRINT("Readed from i > %i\n", res);
  }
  PRINT("Started all process, destruction of the earth now being done\n");
  fflush(stdin);
}

void child_handler(int n){
  write(my_pipe[WRITE_PIPE], &to_send, sizeof(pin_status_t));
}

void create_process(int i, pidpipe PIN_PIPE_PID[MAX_PINS]){
  if(i == MAX_PINS){
    return; 
  }else{
    PRINT("Starting %i\n", i);
    pipe(PIN_PIPE_PID[i].pipe);
    int pid = fork();
    if(pid < 0){
      PRINT("Fatal error occurred in the creation of a childi\n");
      exit(1);
    }else if(pid == 0){
      //child process started here 
      //mypipe is used in signal function and other dark magic around
      my_pipe = PIN_PIPE_PID[i].pipe;
      //close the READ_PIPE first, to avoid being overrunned by bulshittery
      close(my_pipe[READ_PIPE]); 
      //initialize the signal handler used to know that an input has to be piped
      signal(UPDATE_SIGNAL, child_handler); 
      //run the main child_pin_reader that just reads a pin and sleeps
      child_pin_reader(i,PIN_PIPE_PID[i].pipe);
      return;
    }else{
      close(PIN_PIPE_PID[i].pipe[WRITE_PIPE]);
      PIN_PIPE_PID[i].pid = pid;
      create_process(i+1, PIN_PIPE_PID);
    }
  }
}

void start_input(void){
  pidpipe PIN_PIPE_PID[MAX_PINS];
  PRINT("Input reader started\n\n");
  create_process(0, PIN_PIPE_PID);
  input_manager(PIN_PIPE_PID); 
}
