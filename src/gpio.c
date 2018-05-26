#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "gpio.h"
#include "utils.h"

#ifdef ON_THE_RASPBERRY
	static int GPIOExport(int pin);
	static int GPIOUnexport(int pin);
	static int GPIODirection(int pin, int dir);
	static int GPIORead(int pin);
	static int GPIOWrite(int pin, int value);
#endif

int enable_pin(const int pin, const int type) {
  #ifdef ON_THE_RASPBERRY
    if (GPIOExport(pin) != 0) {
      PRINT("Error: GPIOExport failed!\n");
      return -1;
    }

    if (GPIODirection(pin, type) != 0) {
      PRINT("Error: GPIODirection failed! (pin: %d)\n", pin);
      return -1;
    }
  #endif

  return 0;
}

int disable_pin(const int pin) {
  #ifdef ON_THE_RASPBERRY
    if (GPIOUnexport(pin) != 0) {
      PRINT("Error: GPIOUnexport failed!\n");
      return -1;
    }
  #endif

  return 0;
}

int set_pin(const int pin, const int value) {
  PRINT("Set pin %d at value: %d\n", pin, value);
	#ifdef ON_THE_RASPBERRY
		int r = GPIOWrite(pin, value);
    if (r != 0) {
      PRINT ("Error: GPIOWrite failed! (%d, pin: %d)\n", r, pin);
      return -1;
    }
  #endif

  return 0;
}

int read_pin(const int pin, int* res) {
  //PRINT("Read from pin: %d\n", pin);
  int value = 0;
	value = rand() % 2;
  #ifdef ON_THE_RASPBERRY
    value = GPIORead(pin);
    if (value == -1) {
      PRINT("Error: GPIORead failed!(pin: %d)\n", pin);
      return -1;
    }
  #endif

  *res = value;
  return 0;
}

#ifdef ON_THE_RASPBERRY

	static int GPIOExport(int pin)
	{
		char buffer[BUFFER_MAX];
		ssize_t bytes_written;
		int fd;

		fd = open("/sys/class/gpio/export", O_WRONLY);
		if (fd == -1)
			return -1;

		bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
		write(fd, buffer, bytes_written);

		close(fd);
		return 0;
	}

	static int GPIOUnexport(int pin)
	{
		char buffer[BUFFER_MAX];
		ssize_t bytes_written;
		int fd;

		fd = open("/sys/class/gpio/unexport", O_WRONLY);
		if (fd == -1)
			return -1;

		bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
		write(fd, buffer, bytes_written);

		close(fd);
		return 0;
	}

	static int GPIODirection(int pin, int dir)
	{
		static const char s_directions_str[]  = "in\0out";

		char path[DIRECTION_MAX];
		int fd;

		snprintf(path, DIRECTION_MAX, "/sys/class/gpio/gpio%d/direction", pin);
		fd = open(path, O_WRONLY);
		if (fd == -1)
			return -1;

		write(fd, &s_directions_str[IN == dir ? 0 : 3], IN == dir ? 2 : 3);

		close(fd);
		return(0);
	}

	static int GPIORead(int pin)
	{
		char path[VALUE_MAX];
		char value_str[3];
		int fd;

		snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
		fd = open(path, O_RDONLY);
		if (fd == -1)
			return -1;

		if (read(fd, value_str, 3) == -1)
			return -1;

		close(fd);
		return(atoi(value_str));
	}

	static int GPIOWrite(int pin, int value)
	{
		static const char s_values_str[] = "01";

		char path[VALUE_MAX];
		int fd;

		snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
		fd = open(path, O_WRONLY);
		if (fd == -1)
			return -1;

		if (write(fd, &s_values_str[LOW == value ? 0 : 1], 1) != 1)
			return -2;

		close(fd);
		return(0);
	}
#endif
