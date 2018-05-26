#define INPUT_PIN1 A1
#define INPUT_PIN2 A2
#define INPUT_PIN3 A3
#define INPUT_PIN4 A4
#define INPUT_PIN5 A5
#define INPUT_PIN6 A6
#define INPUT_PIN_REFERENCE A7

#define INPUT_PINS_ARRAY { INPUT_PIN1, INPUT_PIN2, INPUT_PIN3, INPUT_PIN4, INPUT_PIN5, INPUT_PIN6 }

#define OUTPUT_PIN1 3
#define OUTPUT_PIN2 5
#define OUTPUT_PIN3 2
#define OUTPUT_PIN4 4
#define OUTPUT_PIN5 6
#define OUTPUT_PIN6 8

#define OUTPUT_PINS_ARRAY { OUTPUT_PIN1, OUTPUT_PIN2, OUTPUT_PIN3, OUTPUT_PIN4, OUTPUT_PIN5, OUTPUT_PIN6 }

#define DEBUG 1

void setup() {
  int i;
  
  //prepare input pins
  int input_pins[] = INPUT_PINS_ARRAY;
  for (i = 0; i < 6; i++)
    pinMode (input_pins[i], INPUT);
  pinMode (INPUT_PIN_REFERENCE, INPUT);

  
  //prepare output pins
  int output_pins[] = OUTPUT_PINS_ARRAY;
  for (i = 0; i < 6; i++) {
    pinMode (output_pins[i], OUTPUT);
    digitalWrite(output_pins[i], LOW);
  }
  
  Serial.begin(9600);
  Serial.println("Initialization ok!");
}

bool analize_value(const int reference, const int value) {
  if (value < (reference /2))
    return true;
  return false;
}

void _loop() {
  //define common variables
  int i;
  int output_pins[] = OUTPUT_PINS_ARRAY;
  
  //read reference value  
  for (i = 0; i < 6; i++) {
    Serial.println(output_pins[i]);
    digitalWrite(output_pins[i], HIGH);
    delay (500);
  }
  
  for (i = 0; i < 6; i++) {
    digitalWrite(output_pins[i], LOW);
    delay (500);
  }
}

void loop() {
  //define common variables
  int i;
  int input_pins[] = INPUT_PINS_ARRAY;
  int output_pins[] = OUTPUT_PINS_ARRAY;
  
  //read reference value
  int reference = analogRead(INPUT_PIN_REFERENCE);
  #ifdef DEBUG
    Serial.print("Reference: ");
    Serial.print(reference);
  #endif
  
  for (i = 0; i < 6; i++) {
    int value = analogRead(input_pins[i]);
    bool presence = analize_value (reference, value);

    if (presence)
      digitalWrite(output_pins[i], HIGH);
    else
      digitalWrite(output_pins[i], LOW);

    #ifdef DEBUG
      Serial.print("pin: ");
      Serial.print(i+1);
      Serial.print(" val: ");
      Serial.print(value);
      Serial.print(" pres: ");
      Serial.print(presence == true ? "1" : "0");
      Serial.print(" | ");
    #endif
  }
  #ifdef DEBUG
    Serial.println("");
  #endif

  delay(1000);
}
