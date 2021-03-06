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

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinza.attach(11);
  pinza.write(5);
}

void loop() {
  int turnsteps = Serial.parseInt();
  while (turnsteps == 0) {
    turnsteps = Serial.parseInt();
  }
  if (turnsteps != 0) {
    if (turnsteps == -2) {
      get_the_ball();
      return;
    }
    else if (turnsteps == 1) {
      //get_the_ball();
      go_to_egg_1();
      close_pinza();
      return;
    }
    else if (turnsteps == 2) {
      //get_the_ball();
      go_to_egg_2();
      close_pinza();
      return;
    }
    else if (turnsteps == 3) {
      //get_the_ball();
      go_to_egg_3();
      close_pinza();
      return;
    }
    else if (turnsteps == 4) {
      //get_the_ball();
      go_to_egg_4();
      close_pinza();
      return;
    }
    else if (turnsteps == 5) {
      //get_the_ball();
      go_to_egg_5();
      close_pinza();
      return;
    }
    else if (turnsteps == 6) {
      //get_the_ball();
      go_to_egg_6();
      close_pinza();
      return;
    }

    Serial.print("(");
    Serial.print(turnsteps);
    Serial.print(", ");
  }

  int updownsteps = Serial.parseInt();
  while (updownsteps == 0) {
    updownsteps = Serial.parseInt();
  }
  if (updownsteps != 0) {
    Serial.print(updownsteps);
    Serial.print(", ");
  }

  int absteps = Serial.parseInt();
  while (absteps == 0) {
    absteps = Serial.parseInt();
  }
  if (absteps != 0) {
    Serial.print(absteps);
    Serial.print(")");
  }
  Serial.println();
  go_to(turnsteps, updownsteps, absteps);
  // put your main code here, to run repeatedly:


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

