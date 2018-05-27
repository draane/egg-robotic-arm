#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<signal.h>
#include<sys/types.h>
#include<string.h>

#include "manager_input.h"
#include "manager_io.h"
#include "utils.h"
#include "gpio.h"
#include "input_settings.h"

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
pidpipe pin_pid_status[MAX_PINS];

// Actually variable my_pipe is not a pipe, it's two different pipes' ends, one to read and one to write.
int *my_pipe;
//this is used by just the father as a global list of children's pids.
int children_pid[MAX_PINS];

static void update_input_pins_from_file(int* pins_from_file);
static void signal_term_handler_children(int sig_int);
static void child_pin_reader(int who_am_i);
static void input_manager();
static void child_signal_handler(int n);
static int create_process(int i);
static void update_input_pins_from_file(int* pins_from_file);


static void signal_term_handler_children(int sig_int){
  if (sig_int == SIGTERM || sig_int == SIGINT){
    kill(getppid(), SIGTERM);
    close(my_pipe[WRITE_PIPE]);
    exit(1);
  }
}

void signal_term_handler_parent(int sigint){
  if (sigint == SIGTERM || sigint == SIGINT){
    PRINT("Input kills all children and itself.\n");
    int i;
    for ( i = 0; i<INPUT_PIN_NUMBER; i++){
      kill(children_pid[i], SIGTERM);
      close(pin_pid_status[i].pipe[READ_PIPE]);
    }

    close(my_pipe[READ_PIPE]);
    close(my_pipe[WRITE_PIPE]);
    // Propagate kill message to parent process.
    kill(getppid(), SIGTERM);
    exit(1);
  }
}

//Just reads the pin and saves it into to_send
static void child_pin_reader(int who_am_i){
  for(;;){
    read_pin(input_pin[who_am_i], &to_send);
  }
}

//1) Expects a message START_MSG from the pipe
//2) Writes output as a string in the pipe
//3) GOTO 1
static void input_manager(){
  char msg[MAX_INFO_TO_SEND_SIZE];

  srand(time(NULL));
  for(;;){
    read(my_pipe[READ_PIPE], msg, MAX_INFO_TO_SEND_SIZE);
    if(strcmp(msg, START_MSG) != 0){  // Unexpected message.
      PRINT("Manager is missbehaving, killing myself\n");
      kill(getppid(), SIGTERM);
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

static void child_signal_handler(int n){
  PRINT("Reading pin.\n");
  write(my_pipe[WRITE_PIPE], &to_send, sizeof(pin_status_t));
}

static int create_process(int i){
  if(i == MAX_PINS){
    return ok;
  }else{
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
      signal(SIGTERM, signal_term_handler_children);
      signal(SIGINT, signal_term_handler_children);
      //run the main child_pin_reader that just reads a pin and sleeps
      child_pin_reader(i);
      //unused return, just here for absolute security of termination
      //if the code reaches this point is pretty much certian something
      //went wrong
      return err;
    }else{
      close(pin_pid_status[i].pipe[WRITE_PIPE]);
      pin_pid_status[i].pid = pid;
      children_pid[i] = pid;
      create_process(i+1);
      return ok;
    }
  }
}

static void update_input_pins_from_file(int* pins_from_file){
  if (pins_from_file[0] != -1){
    // if the first element != -1 then the option -if was specified, and the input_pins need to be updated.
    PRINT("Input process received pins from file.\n");
    int i;
    for (i = 0; i<NUM_PINS; i++){
      input_pin[i] = pins_from_file[i];
    }
  }
  else {
    PRINT("Input process didn't receive pins from file.\n");
  }
}

void start_input(int inpipe, int outpipe, int* pins_from_file){
  // Parameters passed are the ends of the two pipes that the method will use to communicate with the manager.
  // Other ends have already been close by the manager process.
  int i;
  for(i = 0; i< MAX_PINS; i++)
    enable_pin(input_pin[i], IN);
  // The third parameter is the list of pins read from the input_pin_file (when option -if is specified).
  // If no option is specified, then all elements of the list are -1.
  update_input_pins_from_file(pins_from_file);
  // Create the 8 child processes.
  PRINT("Input reader started\n\n");
  int result = create_process(0);
  if(result != ok){
    // Some error has occurred.
    kill(getppid(), SIGTERM);
  }

  //If here, you are father
  pipe_t p = { outpipe, inpipe };
  my_pipe = p;

  signal (SIGTERM, signal_term_handler_parent);
  signal (SIGINT, signal_term_handler_parent);

  input_manager();

}
