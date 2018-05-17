#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <string.h>
#include "manager_output.h"
#include "output_pin.h"
#include "output_settings.h"
#include "utils.h"
#define ever (;;)

static void kill_all_sons(int* childs_pid, const int len);
static int shutdown(const int pipe1, const int pipe2, pid_t* childs_pid, const int child_num, const pid_t father_pid);

static int get_data_from_manager (const int pipe, int* par1, int* par2, int* par3, int* par4);
static int write_data_to_manager (const int pipe, const int par);


void output_manager(int* childs_pid, int child_num, int pipe_write, int pipe_read, pid_t father_pid) {
/*
  Wait for information from the pipe,, calculate the output
  and then send a signal to each output_pin process.
*/
  PRINT("Output Manager process started...\n");

  for ever {
    int eggs_in_the_case, eggs_to_move, eggs_to_order;
    if (0 != get_data_from_manager(pipe_read, &eggs_in_the_case, &eggs_to_move, &eggs_to_order) ) {
      shutdown(pipe_read, pipe_write, output_pin_pid, childs_pid, child_num, father_pid);
    }

    int i; // just a counter
    // calculating values for pins representing number of egg in the case, done
    // with bitwise and
    for (i = 0; i < 3; i++) {
      int pin_value = (eggs_in_the_case >> i) & 1;
      if (pin_value == 0)
        kill(childs_pid[i], SIGNAL0);
      else
        kill(childs_pid[i], SIGNAL1);
    }

    // calculating values for pins representing number of egg to move, done
    // with bitwise and
    for (i = 0; i < 2; i++) {
      int pin_value = (eggs_to_move >> i) & 1;
      if (pin_value == 0)
        kill(childs_pid[i+3], SIGNAL0);
      else
        kill(childs_pid[i+3], SIGNAL1);
    }

    // calculating values for pins representing number of egg to order, done
    // with bitwise and
    for (i = 0; i < 3; i++) {
      int pin_value = (eggs_to_order >> i) & 1;
      if (pin_value == 0)
        kill(childs_pid[i+5], SIGNAL0);
      else
        kill(childs_pid[i+5], SIGNAL1);
    }

    if (0 != write_data_to_manager(pipe_write, 0) ) {
      shutdown(pipe_read, pipe_write, output_pin_pid, childs_pid, child_num, father_pid);
    }

  }

  sleep(8);
  kill_all_sons(childs_pid, OUTPUT_PIN_NUMBER);

}

void start_output(int pipe_write, int pipe_read, pid_t father_pid) {
/*
  Create all the output_pin process, giving each process its pin number,
  and then execute the output_manager process.
*/

  PRINT("Starting output processes...\n");
  int output_pin[] = { DEFAULT_OUTPUT_PINS_ARRAY };

  pid_t* output_pin_pid = malloc(sizeof(pid_t)*OUTPUT_PIN_NUMBER);

  int i;
  for(i = 0; i<OUTPUT_PIN_NUMBER; i++) {
    pid_t pid = fork();
    if (pid == -1) {
      PRINT("Error at spawning child, closing\n");
      shutdown(pipe_read, pipe_write, output_pin_pid, i+1, father_pid);
    }
    else if(pid == 0) {
      output_pin_controller(output_pin[i]);
    }
    else {
      enable_pin(output_pin[i]);
      output_pin_pid[i] = pid;
    }
  }


  //set handler for SIGTERM
  signal (SIGTERM, sig_term_handler);

  sleep(1); // just to make sure every process spawned


  output_manager(output_pin_pid, pipe_write, pipe_read, father_pid);
}


void kill_all_sons(pid_t* childs_pid, const int len) {
/*
  send SIGKILL segnal to all the child processes, which pids are in childs_pid
  then delete the array of pids
*/
  PRINT("KILLING ALL OUTPUT_PIN PROCESSES...\n");

  int i;
  for (i = 0; i < len; i++) {
    kill(childs_pid[i], SIGKILL);
  }

  free(childs_pid);
  PRINT("\tDONE\n");
}

static int get_data_from_manager (const int pipe, int* par1, int* par2, int* par3, int* par4) {
  char msg_received[MAX_INFO_TO_SEND_SIZE];
  strcpy(msg_received, "\0");

  read(pipe, msg_received, MAX_INFO_TO_SEND_SIZE);

  if (strcmp(msg_received, "start\0") != 0) {
    fprintf(stdout, "Output process didn't receive the start command as expected.\n");
    return 1;
  }
  else {
    write(pipe_output_write, "ack\0", MAX_INFO_TO_SEND_SIZE);
  }

  int parameters[NUM_PARAMETERS_RECEIVED];

  int i;
  for (i = 0; i< NUM_PARAMETERS_RECEIVED; i++) {
    read(pipe, msg_received, MAX_INFO_TO_SEND_SIZE);
    parameters[i] = atoi(msg_received);
    fprintf(stdout, "received %s\n", msg_received);
    write(pipe_output_write, "ack\0", MAX_INFO_TO_SEND_SIZE);
  }

  read(pipe, msg_received, MAX_INFO_TO_SEND_SIZE);
  if (strcmp(msg_received, "finish_output\0") != 0) {
    return 1;
  }

  fprintf(stdout, "received %s\n", msg_received);
  write(pipe_output_write, "ack\0", MAX_INFO_TO_SEND_SIZE);

  *par1 = parameters[0];
  *par2 = parameters[1];
  *par3 = parameters[2];

  return 0;
}

static int write_data_to_manager (const int pipe, const int par) {
  write(pipe, "finish_output\0", MAX_INFO_TO_SEND_SIZE);
  read(pipe_output_read, msg_received, MAX_INFO_TO_SEND_SIZE);
  if (strcmp(msg_received, "ack\0") != 0) {
    return 1;
  }
  return 0;
}

static int shutdown(const int pipe1, const int pipe2, pid_t* childs_pid, const int child_num, const pid_t father_pid) {
  //kill all child
  kill_all_sons(childs_pid, child_num);

  //close pipes
  close(pipe1);
  close(pipe2);

  //send END signal to father
  kill(SIGTERM, father_pid);

  exit 1;
}

void sig_term_handler(int par) {
  shutdown
}
