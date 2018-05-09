#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "output_pin.h"
#include "utils.h"

#define ever (;;)

void set_pin(int pin, int value) {
  PRINT("Set pin %i at value: %i\n", pin, value);
}

void output_pin_controller(int pin) {
  set_pin(pin, 0);

  sigset_t set;
  sigemptyset(&set);
  sigaddset(&set, SIGUSR1);
  sigaddset(&set, SIGUSR2);

  while(1) {
    PRINT("WAITING\n");
    int signal;
    if (sigwait(&set, &signal) != 0) {
      PRINT("Error: sigwait in output_pin: %i failed!\n", pin);
      exit(1);
    }
    PRINT("AFTER SIGNAL\n");
    if (signal == SIGUSR1)
      set_pin(pin, 1);
    else if (signal == SIGUSR2)
      set_pin(pin, 0);
    else
      PRINT("Error: wrong signal!\n");
  }
  pause();
}
