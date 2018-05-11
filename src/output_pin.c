#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "output_pin.h"
#include "utils.h"
#include "output_settings.h"

#define ever (;;)

void set_pin(const int pin, const int value);

void output_pin_controller(const int pin) {
/*
  at startup set the pin to 0;
  waits for signal and then call set_pin with the right value
*/
  set_pin(pin, 0);

  sigset_t set;
  sigemptyset(&set);
  sigaddset(&set, SIGNAL0);
  sigaddset(&set, SIGNAL1);

  for ever {
    int sig; //recived signal
    if (sigwait(&set, &sig) != 0) {
      PRINT("Error: sigwait in output_pin: %i failed!\n", pin);
      exit(1);
    }
    if (sig == SIGNAL0) {
      set_pin(pin, 0);
    }
    else if (sig == SIGNAL1) {
      set_pin(pin, 1);
    }
    else {
      // should never get here
      PRINT("Error: wrong signal!\n");
    }
  }
}


void set_pin(const int pin, const int value) {
/*
  set pin to value
*/
  PRINT("Set pin %i at value: %i\n", pin, value);
}
