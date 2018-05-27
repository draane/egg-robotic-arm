#ifndef SERIAL_H
#define SERIAL_H

int serial_start (int port_number, int baud);
int send_message_to_arduino (int fd, int message);
int serial_close(int port);
#endif
