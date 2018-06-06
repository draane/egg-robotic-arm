#include <Servo.h>

#include "motors.h"

#define TURNER_PIN1 48
#define TURNER_PIN2 49
#define TURNER_PIN3 50
#define TURNER_PIN4 51
#define UPDOWN_PIN1 4
#define UPDOWN_PIN2 5
#define UPDOWN_PIN3 6
#define UPDOWN_PIN4 7
#define DEPTH_PIN1 30
#define DEPTH_PIN2 31
#define DEPTH_PIN3 32
#define DEPTH_PIN4 33

// position of the warehouse.
#define EGG_TURN_POS 2100
#define EGG_HEIGHT_POS 600
#define EGG_DEPTH_POS_BACK -700 // Defined not to hit the ball while turning.
#define EGG_DEPTH_POS_ON 1000    // Defined to get the ball geting closer with the pinza opended.

#define EGG1_TURN -EGG_TURN_POS
#define EGG2_TURN -EGG_TURN_POS
#define PINZA_ANGLE 65

Motor Turner_motor(TURNER_PIN1, TURNER_PIN2, TURNER_PIN3, TURNER_PIN4);
Motor UpDown_motor(UPDOWN_PIN1, UPDOWN_PIN2, UPDOWN_PIN3, UPDOWN_PIN4);
Motor DEPTH_motor(DEPTH_PIN1, DEPTH_PIN2, DEPTH_PIN3, DEPTH_PIN4);
Servo pinza;

int turn_pos, height_pos, ab_pos = 0;


void setup()
{ 
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600); 
  pinza.attach(11);
  pinza.write(5);
}


void loop()  //main loop
{
  Serial.println("42"); 
  
  int id_message;
  int old_message = -1;  
  int inByte;
  while (true) {
    if (Serial.available() > 0)
    {
      inByte = Serial.read();  
      id_message = Serial.read();  
      
      if (id_message == old_message) //messaggio vecchio, già eseguito
      { 
        digitalWrite(LED_BUILTIN, HIGH);
        delay(300);
        digitalWrite(LED_BUILTIN, LOW);
        delay(300);
        digitalWrite(LED_BUILTIN, HIGH);
        delay(500);
        digitalWrite(LED_BUILTIN, LOW);
        
        Serial.println(id_message);
      }  
      else
      { 
        //esegui cose
        sposta(inByte);    
        Serial.println(id_message);
        old_message = id_message;    
      }
    }
  
  delay(100);
  }

}

void sposta(int egg_to_move) {
  get_the_ball();

  if (egg_to_move == 1) {
    go_to_egg_1();
  }
  else if (egg_to_move == 2) {
    go_to_egg_2();
  }
  else if (egg_to_move == 3) {
    go_to_egg_3();
  }
  else if (egg_to_move == 4) {
    go_to_egg_4();
  }
  else if (egg_to_move == 5) {
    go_to_egg_5();
  }
  else if (egg_to_move == 6) {
    go_to_egg_6();
  }
  else {
    open_pinza();
    delay(200);
  }

  
  close_pinza();

}


void open_pinza() {
  pinza.write(PINZA_ANGLE);
}
void close_pinza() {
  for (int i = 0; i < PINZA_ANGLE; i++) {
    pinza.write(PINZA_ANGLE - i);
    delay(3);
  }
}

void go_to(int turn, int height, int depth) {
  turn_pos += turn;
  height_pos += height;
  ab_pos += depth;
  int tdir = turn > 0 ? 1 : -1;
  turn = abs(turn);
  int hdir = height > 0 ? 1 : -1;
  height = abs(height);
  int ddir = depth > 0 ? 1 : -1;
  depth = abs(depth);
  for (int i = 0; i < max(max(turn, height), depth); i++) {
    if (i < turn) {
      Turner_motor.Step(tdir == 1 ? forward : backward);
    }
    if (i < height) {
      UpDown_motor.Step(hdir == 1 ? forward : backward);
    }
    if (i < depth) {
      DEPTH_motor.Step(ddir == 1 ? forward : backward);
    }
    delay(1);
  }
}

void go_to_zero() {
  go_to(-turn_pos, -height_pos, -ab_pos);
}

void go_to_zero_plain() {
  go_to(-turn_pos, 0, -ab_pos);
  go_to_zero();
}

void go_to_egg_1() {
  go_to(-1900, 0 , 0);
  go_to(0, -600, 500);
  open_pinza();
  go_to_zero();
  return;
}

void go_to_egg_2() {
  go_to(-1800, 0, 0);
  go_to(0, -600, -100);
  open_pinza();
  go_to_zero();
  return;
}

void go_to_egg_3() {
  go_to(-2400, 300, 0);
  go_to(0, -600, 400);
  open_pinza();
  go_to_zero();
  return;
}

void go_to_egg_4() {
  go_to(-2400, 300, 0);
  go_to(0, -400, -400);
  open_pinza();
  go_to_zero();
  return;
}

void go_to_egg_5() {
  go_to(-2700, 300, 0);
  go_to(0, -200, 400);
  open_pinza();
  go_to_zero();
  return;
}

void go_to_egg_6() {
  go_to(-2700, 0, 0);
  go_to(0, -200, -350);
  open_pinza();
  go_to_zero();
  return;
}


void get_the_ball() {
  go_to_zero();
  open_pinza();
  go_to(EGG_TURN_POS, EGG_HEIGHT_POS, EGG_DEPTH_POS_BACK);
  go_to(0, 0, EGG_DEPTH_POS_ON);
  close_pinza();
  go_to(0, 300, 0);
  go_to_zero_plain();
  return;

}

