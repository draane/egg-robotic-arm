#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include "gpio.h"
#include "output_pin.h"
#include "utils.h"
#include "output_settings.h"

pid_t output_pin_father_pid;

#define ever (;;)

static void shutdown(void);
static void output_pin_sigterm_handler(int signal);

void output_pin_controller(const int pin) {
/*
  at startup set the pin to 0;
  waits for signal and then call set_pin with the right value
*/
  #ifdef ON_THE_RASPBERRY
  /*if (enable_pin(pin) != 0) {
    PRINT("Error: enable_pin fail!");
    exit(1);
  }*/
  #endif

  if (set_pin(pin, 0) != 0) {
    PRINT("Error: set_pin fail!\n");
    exit(2);
  }

  sigset_t set;
  sigemptyset(&set);
  sigaddset(&set, SIGNAL0);
  sigaddset(&set, SIGNAL1);

  for ever {
    int sig; //recived signal
    int value;
    if (sigwait(&set, &sig) != 0) {
      PRINT("Error: sigwait in output_pin: %i failed!\n", pin);
      exit(1);
    }
    if (sig == SIGNAL0) {
      value = 0;
    }
    else if (sig == SIGNAL1) {
      value = 1;
    }
    else {
      // should never get here
      PRINT("Error: wrong signal!\n");
    }

    if (set_pin(pin, value) != 0) {
      PRINT("Error: set_pin fail!\n");
      exit(2);
    }
  }
}

static void shutdown(void) {
  kill(output_pin_father_pid, SIGTERM);
  exit (0);
}

static void output_pin_sigterm_handler(int signal) {
  shutdown();
}
