#ifndef GPIO_H
#define GPIO_H

#define IN  0
#define OUT 1

#define LOW  0
#define HIGH 1

#define BUFFER_MAX 3
#define DIRECTION_MAX 35
#define VALUE_MAX 30

int enable_pin(const int pin, const int type);
int disable_pin(const int pin);
int set_pin(const int pin, const int value);
int read_pin(const int pin, int* res);

#endif
