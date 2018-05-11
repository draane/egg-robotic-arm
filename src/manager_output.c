#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include "manager_output.h"
#include "output_pin.h"
#include "output_settings.h"
#include "utils.h"

#define fuckoff_and_kill_yourself(c, n) kill_all_sons(c, n); exit(1);

void kill_all_sons(int* childs_pid, const int len);

void output_manager(int* childs_pid){
/*
  Wait for information from the pipe,, calculate the output
  and then send a signal to each output_pin process.
*/
  PRINT("Output Manager process started...\n");

  //TODO: wait for pipe...

  int eggs_in_the_case, eggs_to_move, eggs_to_order;

  //TODO: assign values from the pipe to the previus variables ^^
  //for now I'll assign it here:
  eggs_in_the_case = 0;
  eggs_to_move = 3;
  eggs_to_order = 6;
  //please remember to remove it

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

  //TODO: add for ever and remove the 2 lines below:
  sleep(8);
  kill_all_sons(childs_pid, OUTPUT_PIN_NUMBER);

}

void start_output(void) {
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
      kill_all_sons(output_pin_pid, i-1);
      exit(1);
    }
    else if(pid == 0) {
      output_pin_controller(output_pin[i]);
    }
    else {
      output_pin_pid[i] = pid;
    }
  }

  sleep(1); // just to make sure every process spawned
  output_manager(output_pin_pid);
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
