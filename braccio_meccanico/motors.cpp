#include"motors.h"
#include<Arduino.h>
Motor::Motor(int pin1, int pin2, int pin3, int pin4){
    step = 0;
    this->pin1 = pin1;
    this->pin2 = pin2;
    this->pin3 = pin3;
    this->pin4 = pin4;
    //CPP IS A TRASHBAG LANGUAGE AND I CANT { } INITIALIZE WITHOUT IMPORTING MEGS OF FUCKING LIBRARYES
    pinMode(pin1, OUTPUT);
    pinMode(pin2, OUTPUT);
    pinMode(pin3, OUTPUT);
    pinMode(pin4, OUTPUT);

}
void Motor::Step(direction d){
    switch(step){
        case 0:
          digitalWrite(pin1, LOW); 
          digitalWrite(pin2, LOW);
          digitalWrite(pin3, LOW);
          digitalWrite(pin4, HIGH);
        break; 
        case 1:
          digitalWrite(pin1, LOW); 
          digitalWrite(pin2, LOW);
          digitalWrite(pin3, HIGH);
          digitalWrite(pin4, HIGH);
        break; 
        case 2:
          digitalWrite(pin1, LOW); 
          digitalWrite(pin2, LOW);
          digitalWrite(pin3, HIGH);
          digitalWrite(pin4, LOW);
        break; 
        case 3:
          digitalWrite(pin1, LOW); 
          digitalWrite(pin2, HIGH);
          digitalWrite(pin3, HIGH);
          digitalWrite(pin4, LOW);
        break; 
        case 4:
          digitalWrite(pin1, LOW); 
          digitalWrite(pin2, HIGH);
          digitalWrite(pin3, LOW);
          digitalWrite(pin4, LOW);
        break; 
        case 5:
          digitalWrite(pin1, HIGH); 
          digitalWrite(pin2, HIGH);
          digitalWrite(pin3, LOW);
          digitalWrite(pin4, LOW);
        break; 
          case 6:
          digitalWrite(pin1, HIGH); 
          digitalWrite(pin2, LOW);
          digitalWrite(pin3, LOW);
          digitalWrite(pin4, LOW);
        break; 
        case 7:
          digitalWrite(pin1, HIGH); 
          digitalWrite(pin2, LOW);
          digitalWrite(pin3, LOW);
          digitalWrite(pin4, HIGH);
        break; 
        default:
          digitalWrite(pin1, LOW); 
          digitalWrite(pin2, LOW);
          digitalWrite(pin3, LOW);
          digitalWrite(pin4, LOW);
        break;
    }
    step += d;
    if(step>7){step=0;}
    if(step<0){step=7; }
}
void Motor::Stop(){}