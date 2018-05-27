/*
 * usefull wrapper to write reliable comands to an arduino Mega
 * created by: Andrea Dalla Costa (andraane@libero.it) - 2016
 * based on: arduino-serial by Tod E. Kurt, http://todbot.com/blog/
 */

#include <stdio.h>    // Standard input/output definitions
#include <stdlib.h>
#include <string.h>   // String function definitions
#include <unistd.h>   // for usleep()
#include <termios.h>  // POSIX terminal control definitions

#include "arduino-serial-lib.h"

#define ms_to_wait 100

#define DEBUG 0

int serial_start (int port_number, int baud)
{
  int fd;
  char base_port_string[13] = "/dev/ttyACM";

  char port_string[15];
  if (port_number == -1) {
    int i;
    for (i = 0; i < 10 && fd == -1; i++) {
      sprintf(port_string, "%s%d", base_port_string, i);
  		fd = serialport_init(port_string, baud);
    }
  }
  else {
    sprintf(port_string, "%s%d", base_port_string, port_number);
    fd = serialport_init(port_string, baud);
  }

  if (fd == -1)
    return -1;

  //send init message
  uint8_t init_message = 7;
  if (serialport_writebyte(fd, init_message) == -1)
  {
    serialport_close(fd);
		return -2;
  }

  //read init response
  char eolchar = '\n';
  int timeout = 5000;
  char buf[4];
  if (serialport_read_until(fd, buf, eolchar, 4, timeout) < 0) {
    serialport_close(fd);
		return -3;
  }

  if ( buf[0] != '4' || buf[1] != '2' ) {
    serialport_close(fd);
		return -4;
  }

  tcflush(fd, TCIOFLUSH);
  return fd;
}


int send_message_to_arduino (int fd, int message) {
  static uint8_t message_id = 25;
	char eolchar = '\n';
	int timeout = 5000;
  char id_buffer[3];

	int error = 0;

	do {
    serialport_flush(fd);
    error = 0;

    if (serialport_writebyte(fd, message) == -1) {
      return -1;
    }

    if (serialport_writebyte(fd, message_id) == -1) {
      return -2;
    }

    if (serialport_read_until(fd, id_buffer, eolchar, 8, timeout) == -2) {
    	return -4;
    }

    uint8_t id_return = atoi (id_buffer);

    if (id_return != message_id)
      error++;
    else
      error = 0;

    if (error > 0)
    	usleep( ms_to_wait * 1000 );

  } while (error > 0);

  message_id ++;

  return 0;
}

int serial_close(int port) {
  serialport_close(port);
}
