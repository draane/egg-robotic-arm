#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
//lib for the low level pin
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#include "output_pin.h"
#include "utils.h"
#include "output_settings.h"

#define ever (;;)

static int enable_pin(const int pin);
static int set_pin(const int pin, const int value);

void output_pin_controller(const int pin) {
/*
  at startup set the pin to 0;
  waits for signal and then call set_pin with the right value
*/
  #ifdef ON_THE_RASPBERRY
  if (enable_pin(pin) != 0) {
    PRINT("Error: enable_pin fail!");
    exit(1);
  }
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

static int enable_pin(const int pin){
  //Enable GPIO pins
  #define BUFFER_MAX 3
  char buffer[BUFFER_MAX];
	ssize_t bytes_written;
	int fd;

	fd = open("/sys/class/gpio/export", O_WRONLY);
	if (-1 == fd) {
		PRINT("Failed to open export for writing!\n");
		return -1;
	}

	bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
	write(fd, buffer, bytes_written);
	close(fd);


  //Set GPIO directions to OUTPUT
	static const char s_directions_str[]  = "out"; //old string was "in\0out"

  #define DIRECTION_MAX 35
	char path[DIRECTION_MAX];

	snprintf(path, DIRECTION_MAX, "/sys/class/gpio/gpio%d/direction", pin);
	fd = open(path, O_WRONLY);
	if (-1 == fd) {
		PRINT("Failed to open gpio direction for writing!\n");
		return -1;
	}

	if (-1 == write(fd, &s_directions_str, 3)) {
		PRINT("Failed to set direction!\n");
		return -1;
	}

	close(fd);

  return 0;
}

static int set_pin(const int pin, const int value) {
/*
  set pin to value
*/
  PRINT("Set pin %i at value: %i\n", pin, value);

  #ifdef ON_THE_RASPBERRY
	static const char s_values_str[] = "01";
  #define VALUE_MAX 30
	char path[VALUE_MAX];
	int fd;

	snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
	fd = open(path, O_WRONLY);
	if (-1 == fd) {
		printf("Failed to open gpio value for writing!\n");
		return(-1);
	}

	if (1 != write(fd, &s_values_str[ value == 0 ? 0 : 1], 1)) {
		PRINT("Failed to write value!\n");
		return(-1);
	}

	close(fd);
  #endif

  return 0;
}
