#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <string.h>
#include "manager_output.h"
#include "output_pin.h"
#include "output_settings.h"
#include "utils.h"
#include "gpio.h"

#define MAX_SEND_BUFFER_SIZE 2
#define MAX_RECIVE_BUFFER_SIZE 2
#define PARAMETERS_RECIVED_FROM_THE_PIPE 4
#define ever (;;)

//global variables used by shutdown. must be globak, cause shutdown is called also from SIGTERM handler;
int output_pin[] = { DEFAULT_OUTPUT_PINS_ARRAY };
pid_t* output_pin_pid;
pid_t output_father_pid;
int output_read_pipe, output_write_pipe;


static void kill_all_sons(void);
static int shutdown(const int exit_value);
static void output_manager_end_signal_handler (int signal);

static int get_data_from_manager (const int pipe, int* par1, int* par2, int* par3, int* par4);
static int write_data_to_manager (const int pipe, const int par);

void output_manager(int pipe_write, int pipe_read, pid_t father_pid) {

/*
  Wait for information from the pipe,, calculate the output
  and then send a signal to each output_pin process.
*/
  PRINT("Output Manager process started...\n");

  int eggs_in_the_case, eggs_to_move, eggs_to_order;
  int arduino_value;

  for ever {
    if (0 != get_data_from_manager(pipe_read, &eggs_in_the_case, &eggs_to_move, &eggs_to_order, &arduino_value) ) {
      shutdown(1);

    }

    int i; // just a counter


    // calculating values for pins representing number of egg in the case, done
    // with bitwise and
    for (i = 0; i < 3; i++) {
      int pin_value = (eggs_in_the_case >> i) & 1;
      if (pin_value == 0)
        kill(output_pin_pid[i], SIGNAL0);
      else
        kill(output_pin_pid[i], SIGNAL1);
    }

    // calculating values for pins representing number of egg to move, done
    // with bitwise and
    for (i = 0; i < 2; i++) {
      int pin_value = (eggs_to_move >> i) & 1;
      if (pin_value == 0)
        kill(output_pin_pid[i+3], SIGNAL0);
      else
        kill(output_pin_pid[i+3], SIGNAL1);
    } 

    // calculating values for pins representing number of egg to order, done
    // with bitwise and
    for (i = 0; i < 3; i++) {
      int pin_value = (eggs_to_order >> i) & 1;
      if (pin_value == 0)
        kill(output_pin_pid[i+5], SIGNAL0);
      else
        kill(output_pin_pid[i+5], SIGNAL1);
    }

    if (0 != write_data_to_manager(pipe_write, 0) ) {
      shutdown(1);
    }
    //shutdown never get here
    shutdown(1);
  }
}

void start_output(int pipe_write, int pipe_read) {
/*
  Create all the output_pin process, giving each process its pin number,
  enable the pins,
  and then execute the output_manager process.
*/

  PRINT("Starting output processes...\n");

  pid_t father_pid = getppid();

  output_pin_pid = malloc(sizeof(pid_t)*OUTPUT_PIN_NUMBER);

  int i;

  //sets all the pid to -1, so in kill_all_sons it doesnt send kill signal to
  //non valid pids
  for (i = 0; i<OUTPUT_PIN_NUMBER; i++)
    output_pin_pid[i] = -1;

  output_read_pipe = pipe_read;
  output_write_pipe = pipe_write;
  output_father_pid = father_pid;

  for(i = 0; i<OUTPUT_PIN_NUMBER; i++) {
    pid_t pid = fork();
    if (pid == -1) {
      PRINT("Error at spawning child, closing\n");
      shutdown(1);
    }
    else if(pid == 0) {
      output_pin_controller(output_pin[i]);
    }
    else {
      enable_pin(output_pin[i], OUT);
      output_pin_pid[i] = pid;
    }
  }

  //set handler for SIGTERM
  signal(SIGTERM, output_manager_end_signal_handler);
  //set handler for SIGINT, so it handles ^C
  signal(SIGINT, output_manager_end_signal_handler);

  sleep(1); // just to make sure every process spawned

  //execute the main task
  output_manager(pipe_write, pipe_read, father_pid);
}

static int get_data_from_manager (const int pipe, int* par1, int* par2, int* par3, int* par4) {
/*
retrive data from the pipe and puts the data recived into the four integer (parx)
returns 0 if everything went define, other values if errors appened
*/
  char buffer[MAX_RECIVE_BUFFER_SIZE];
  int parameters[PARAMETERS_RECIVED_FROM_THE_PIPE];
  int i;

  for (i = 0; i < PARAMETERS_RECIVED_FROM_THE_PIPE; i++) {
    read(pipe, buffer, 2);
    parameters[i] = atoi(buffer);
    PRINT("received from manager: %s\n", buffer);
  };

  *par1 = parameters[0];
  *par2 = parameters[1];
  *par3 = parameters[2];
  *par4 = parameters[2];

  return 0;
}

static int write_data_to_manager (const int pipe, const int par) {
/*
send data to manager, so it knows everything in the output went fine
*/
  char buffer[MAX_SEND_BUFFER_SIZE];
  sprintf(buffer, "%d", par);
  write(pipe, buffer, MAX_SEND_BUFFER_SIZE);
//TODO: Check that write doesnt return errors

  return 0;
}

void kill_all_sons(void) {
/*
  send SIGKILL segnal to all the child processes, which pids are in childs_pid,
  disable the pins, then delete the array of pids
*/
  PRINT("KILLING ALL OUTPUT_PIN PROCESSES...\n");

  int i;
  for (i = 0; i < OUTPUT_PIN_NUMBER && output_pin_pid[i] != -1; i++) {
    disable_pin(output_pin[i]);
    kill(output_pin_pid[i], SIGKILL);
  }

  free(output_pin_pid);
  PRINT("\tDONE\n");
}

static int shutdown(const int exit_value) {
/*
call kill_all_sons to close output_pin processes,
close the pipes,
send SIGTERM to the father_pid process,
and then close the process with the value of the param exit_value as return value
*/
  //kill all child
  kill_all_sons();

  //close pipes
  close(output_read_pipe);
  close(output_write_pipe);

  //send END signal to father
  kill(output_father_pid, SIGTERM);

  exit (exit_value);
}

static void output_manager_end_signal_handler (int signal) {
/*
just call shutdown with different param depending in the signal recived
*/
  if (signal == SIGTERM)
    shutdown(-1);
  else
    shutdown(-2);
}
