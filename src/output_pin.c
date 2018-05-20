#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include "gpio.h"
#include "output_pin.h"
#include "utils.h"
#include "output_settings.h"

pid_t output_pin_father_pid;

#define ever (;;)

static void shutdown(const int exit_value);
static void output_pin_end_signal_handler(int signal);

void output_pin_controller(const int pin) {
/*
  at startup set the pin to 0;
  waits for signal and then call set_pin with the right value
*/

  //set handler for SIGTERM
  signal(SIGTERM, output_pin_end_signal_handler);
  //set handler for SIGINT (needed for the /little/ chanche that ^C is send
  //while the progam is executing this process
  signal(SIGINT, output_pin_end_signal_handler);

  //set the pin to 0
  if (set_pin(pin, 0) != 0) {
    PRINT("Error: set_pin fail!\n");
    shutdown(1);
  }

  //create signal set for sigwait
  sigset_t set;
  sigemptyset(&set);
  sigaddset(&set, SIGNAL0);
  sigaddset(&set, SIGNAL1);

  for ever {
    int sig; //recived signal
    int value;
    if (sigwait(&set, &sig) != 0) {
      PRINT("Error: sigwait in output_pin: %i failed!\n", pin);
      shutdown(1);
    }
    if (sig == SIGNAL0) {
      value = LOW;
    }
    else if (sig == SIGNAL1) {
      value = HIGH;
    }
    else {
      // should never get here
      PRINT("Error: wrong signal!\n");
      shutdown(3);
    }

    //set the pin to the value recived via the signals
    if (set_pin(pin, value) != 0) {
      PRINT("Error: set_pin fail!\n");
      shutdown(2);
    }
  }
}

static void shutdown(const int exit_value) {
/*
if exit_value is -1 then a SIGTERM signal has been recived. so no need to send
the signal to the father
*/
  if (exit_value != -1)
    kill(output_pin_father_pid, SIGTERM);
  exit (exit_value);
}

static void output_pin_end_signal_handler(int signal) {
/*
just call shutdown with different param depending in the signal recived
*/
  if (signal == SIGTERM)
    shutdown(-1);
  else
    shutdown(-2);
}
