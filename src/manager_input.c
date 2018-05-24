#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<signal.h>
#include<sys/types.h>
#include<string.h>

// TODO: remove it from code when random is not needed anymore.
#include <time.h>

#include "manager_input.h"
#include "manager_io.h"
#include "utils.h"
#include "gpio.h"

#define INPUT_PIN_NUMBER 8

#define DEFAULT_INPUT_PIN1 17
#define DEFAULT_INPUT_PIN2 18
#define DEFAULT_INPUT_PIN3 27
#define DEFAULT_INPUT_PIN4 22
#define DEFAULT_INPUT_PIN5 23
#define DEFAULT_INPUT_PIN6 24
#define DEFAULT_INPUT_PIN7 25
#define DEFAULT_INPUT_PIN8 5

#define DEFAULT_INPUT_PINS_ARRAY DEFAULT_INPUT_PIN1, DEFAULT_INPUT_PIN2, DEFAULT_INPUT_PIN3, DEFAULT_INPUT_PIN4, DEFAULT_INPUT_PIN5, DEFAULT_INPUT_PIN6, DEFAULT_INPUT_PIN7, DEFAULT_INPUT_PIN8


#define MAX_PINS 8

typedef int pinstatus;

#define READ_PIPE 0
#define WRITE_PIPE 1

#define UPDATE_SIGNAL SIGUSR1
#define ON 1
#define OFF !ON
#define NO_STATUS 100
typedef sig_atomic_t pin_status_t;
typedef int pipe_t[2];

int input_pin[] = { DEFAULT_INPUT_PINS_ARRAY };

typedef struct pidpipe{
  pipe_t  pipe;
  pid_t pid;
} pidpipe;

enum okerr {
  ok=0,
  err=1
};

//this area of the code is used only by the child processes
pin_status_t to_send = NO_STATUS;

// Actually variable my_pipe is not a pipe, it's two different pipes' ends, one to read and one to write.
int *my_pipe;
//this is used by just the father

/*
//Kills all sons, used when a fatal error occurres or just
//when the process has to be terminated
void kill_all_sons(int limit, pidpipe pin_pid_status[MAX_PINS]){
  for(int i = 0; i < limit; i++){
    kill(pin_pid_status[i].pid,SIGKILL);
  }
}
*/ 

//Just reads the pin and saves it into to_send
void child_pin_reader(int who_am_i){
  for(;;){
    read_pin(who_am_i, &to_send);
  }
}

//1) Expects a message START_MSG from the pipe
//2) Writes output as a string in the pipe
//3) GOTO 1
//TODO: Change MAX_INFO_TO_SEND_SIZE to an actual resonable value
void input_manager(pidpipe pin_pid_status[MAX_PINS]){
  char msg[MAX_INFO_TO_SEND_SIZE];
  srand(time(NULL));
  for(;;){
    read(my_pipe[READ_PIPE], msg, MAX_INFO_TO_SEND_SIZE);   
    if(strcmp(msg, START_MSG) != 0){  // Unexpected message.
      PRINT("Manager is missbehaving, killing myself\n");
      exit(1);
    }
    int i;
    for(i = 0; i<MAX_PINS; i++){
      int res = NO_STATUS;
      while(res == NO_STATUS){
        kill(pin_pid_status[i].pid, UPDATE_SIGNAL);
        read(pin_pid_status[i].pipe[READ_PIPE], &res, sizeof(int));
      }
      PRINT("Read from %i > %i\n",i, res);
      msg[i] = (res + OFFSET_OUTPUT_MSG); //Even more easy to read "bbbbbbbbb" -> all eggs are present 
    }
    msg[MAX_PINS] = '\0'; //Make it easy to ready and parse

    // Generate 2 bytes out of the string.
    unsigned char char_to_send = make_one_byte_from_string(msg);

    unsigned char message_to_send[2];
    message_to_send[0] = char_to_send;
    message_to_send[1] = '\0';

    write(my_pipe[WRITE_PIPE], message_to_send, DIM_OF_MSG_PIPE);  
  }
}

void child_signal_handler(int n){
  //TODO read from GPIO PIN
  PRINT("Reading pin.\n");
  write(my_pipe[WRITE_PIPE], &to_send, sizeof(pin_status_t));
}

int create_process(int i, pidpipe pin_pid_status[MAX_PINS]){
  if(i == MAX_PINS){
    return ok; 
  }else{
    PRINT("Starting %i\n", i);
    pipe(pin_pid_status[i].pipe);
    int pid = fork();
    if(pid < 0){
      PRINT("Fatal error occurred in the creation of a childi\n");
      return err;
    }else if(pid == 0){
      //child process started here 
      //mypipe is used in signal function and other dark magic around
      my_pipe = pin_pid_status[i].pipe;
      //close the READ_PIPE first, to avoid being overrunned by bulshittery
      close(my_pipe[READ_PIPE]); 
      //initialize the signal handler used to know that an input has to be piped
      signal(UPDATE_SIGNAL, child_signal_handler); 
      //run the main child_pin_reader that just reads a pin and sleeps
      child_pin_reader(i);
      //unused return, just here for absolute security of termination
      //if the code reaches this point is pretty much certian something
      //went wrong
      return err;
    }else{
      close(pin_pid_status[i].pipe[WRITE_PIPE]);
      pin_pid_status[i].pid = pid;
      create_process(i+1, pin_pid_status);
      enable_pin(input_pin[i], IN);
      return ok;
    }
  }
}

void start_input(int inpipe, int outpipe){
  // Parameters passed are the ends of the two pipes that the method will use to communicate with the manager.
  // Other ends have already been close by the manager process.

  // Create the 8 child processes.
  pidpipe pin_pid_status[MAX_PINS];
  PRINT("Input reader started\n\n");
  int result = create_process(0, pin_pid_status);
  if(result != ok){
    // Some error has occurred.
    exit(1);
  }

  //If here, you are father
  pipe_t p = { outpipe, inpipe };
  my_pipe = p;
  input_manager(pin_pid_status);

}
