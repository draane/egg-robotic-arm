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

  }

  //shutdown never get here
  shutdown(1);

}

void start_output(int pipe_write, int pipe_read, pid_t father_pid) {
/*
  Create all the output_pin process, giving each process its pin number,
  and then execute the output_manager process.
*/

  PRINT("Starting output processes...\n");

  output_pin_pid = malloc(sizeof(pid_t)*OUTPUT_PIN_NUMBER);

  int i;

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

  output_manager(pipe_write, pipe_read, father_pid);
}

static int get_data_from_manager (const int pipe, int* par1, int* par2, int* par3, int* par4) {
  unsigned char msg_received[MAX_INFO_TO_SEND_SIZE];
  int parameters[NUM_PARAMETERS_RECEIVED];
  int i;

  for (i = 0; i< NUM_PARAMETERS_RECEIVED; i++) {
    read(pipe, msg_received, MAX_INFO_TO_SEND_SIZE);
    parameters[i] = atoi(msg_received);
    PRINT("received %s\n", msg_received);
  };

  *par1 = parameters[0];
  *par2 = parameters[1];
  *par3 = parameters[2];
  *par4 = parameters[2];

  return 0;
}

static int write_data_to_manager (const int pipe, const int par) {
  char buffer[MAX_INFO_TO_SEND_SIZE];
  sprintf(buffer, "%d", par);
  write(pipe, buffer, MAX_INFO_TO_SEND_SIZE);
  return 0;
}

void kill_all_sons(void) {
/*
  send SIGKILL segnal to all the child processes, which pids are in childs_pid
  then delete the array of pids
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
  //kill all child
  kill_all_sons();

  //close pipes
  close(output_read_pipe);
  close(output_write_pipe);

  //send END signal to father
  kill(SIGTERM, output_father_pid);

  exit (exit_value);
}

static void output_manager_end_signal_handler (int signal) {
  if (signal == SIGTERM)
    shutdown(-1);
  else
    shutdown(-2);
}
