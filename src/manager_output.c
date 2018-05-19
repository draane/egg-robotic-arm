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

#define fuckoff_and_kill_yourself(c, n) kill_all_sons(c, n); exit(1);

void kill_all_sons(int* childs_pid, const int len);

void parse_manager_message(int*, int*, int*, unsigned char*);

void parse_manager_message(int* eggs_in_the_box, int* eggs_to_move, int* eggs_to_order, unsigned char* msg_received_from_manager){
  /*
    - bits 0-2: eggs in the box (configuration 111 is not used.)
    - bits 3-4: usually set to 0, when set they sign the number of eggs to move from the warehouse to the box.
    - bits 5-7: eggs to order (To refill the warehouse or to fill the box?).
  */
  int i;
  unsigned char msg = msg_received_from_manager[0];
  unsigned char eggs_to_order_c = msg & ((unsigned char)7);
  msg = msg >> 3;
  unsigned char eggs_to_move_c = msg & ((unsigned char)3);
  msg =  msg >> 2;
  unsigned char eggs_in_the_box_c = msg & ((unsigned char)7);

  *(eggs_to_order) = eggs_to_order_c;
  *(eggs_in_the_box) = eggs_in_the_box_c;
  *(eggs_to_move) = eggs_to_move_c;
}

void output_manager(int* childs_pid, int pipe_output_write, int pipe_output_read) {
/*
  Wait for information from the pipe,, calculate the output
  and then send a signal to each output_pin process.
*/
  PRINT("Output Manager process started...\n");

  char msg_received[MAX_INFO_TO_SEND_SIZE];
  strcpy(msg_received, "\0");
  for ever {
    strcpy(msg_received, "\0");
    read(pipe_output_read, msg_received, DIM_OF_MSG_PIPE);

    if (strcmp(msg_received, START_MSG) != 0) {
      PRINT("Output process didn't receive the start command as expected.\n");
      close(pipe_output_read);
      close(pipe_output_write);
      exit(1);
    }
    else {
      write(pipe_output_write, "ack\0", MAX_INFO_TO_SEND_SIZE);
    }

    int parameters[NUM_PARAMETERS_RECEIVED];

    int i; // just a counter

    unsigned char msg_received_from_manager[2];

    read (pipe_output_read, msg_received_from_manager, MAX_INFO_TO_SEND_SIZE);
    PRINT("received %u\n", msg_received_from_manager[0]);
    write(pipe_output_write, "ack\0", MAX_INFO_TO_SEND_SIZE);
   
    int eggs_in_the_case, eggs_to_move, eggs_to_order;
    parse_manager_message(&eggs_in_the_case, &eggs_to_move, &eggs_to_order, msg_received_from_manager);
    
    PRINT("Eggs in the case: %d\nEggs to move: %d\nEggs to order: %d\n", eggs_in_the_case, eggs_to_move, eggs_to_order);
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

    write(pipe_output_write, "finish_output\0", MAX_INFO_TO_SEND_SIZE);
    read(pipe_output_read, msg_received, MAX_INFO_TO_SEND_SIZE);
    if (strcmp(msg_received, "ack\0") != 0) {
      close(pipe_output_write);
      close(pipe_output_read);
      exit(0);
    }
  }

  //TODO: add for ever and remove the 2 lines below:
  sleep(8);
  kill_all_sons(childs_pid, OUTPUT_PIN_NUMBER);

}

void start_output(int pipe_write, int pipe_read) {
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
      enable_pin(output_pin[i]);
      output_pin_pid[i] = pid;
    }
  }

  sleep(1); // just to make sure every process spawned
  output_manager(output_pin_pid, pipe_write, pipe_read);
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
