#ifndef MOTORS_H
#define MOTORS_H 1
#define N_PINS 4
#define N_STATES 8
enum direction{
  forward = 1,
  backward = -1
};
class Motor{
private:
    //the 8th state is a fictional STOP state
    int pin1;
    int pin2;
    int pin3;
    int pin4;
    int step;
public:
    Motor(int pin1, int pin2, int pin3, int pin4);
    void Step(direction);
    void Stop();
};
#endif