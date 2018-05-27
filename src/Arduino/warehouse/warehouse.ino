/* 
  Warehouse implementation:
  Three buttons representing the three slots in the warehouse.
  
*/

const int num_of_eggs = 3;

// 3 Button pins, to change accordingly to the arduino cable management.
const int button_pins[3] = {11, 12, 13}; 

// Led pins
const int led_pins[3] = {14, 15, 16};

const int output_pins[num_of_eggs] = {20, 21, 22};

boolean state[3] = {false, false, false};

int counter = 0;


void setup() {
  // initialize the LED pins as an output:
  for (int i = 0; i<num_of_eggs; i++){
    pinMode(led_pins[i], OUTPUT);
    pinMode(button_pins[i], INPUT);
    pinMode(output_pins[i], OUTPUT);
  }
}

void loop() {
  // read the state of the pushbutton value:
  int button_states[num_of_eggs];
  for (int i = 0; i<num_of_eggs; i++){
    button_states[i] = digitalRead(button_pins[i]); 
  } 
  
  
  for (int i = 0; i<num_of_eggs; i++){
    if (button_states[i] == HIGH) {
      if (state[i] == true){
        digitalWrite(button_pins[i] , LOW);
        counter --;
        state[i] = false;
      }
      else  {
        digitalWrite(button_pins[i], HIGH);
        counter ++;
        state[i] = true;
      }
    } 
   
  } 
  
  // write in output data converted into binary.
  // Writing is contious, but the RaspberryPi will read only when needed.
  
  switch(counter){
    case (0):
      digitalWrite(output_pins[0], LOW);
      digitalWrite(output_pins[1], LOW);
      break;
    case (1):
      digitalWrite(output_pins[0], HIGH);
      digitalWrite(output_pins[1], LOW);
      break;
    case (2):
      digitalWrite(output_pins[0], LOW);
      digitalWrite(output_pins[1], HIGH);
      break;
    default:
      digitalWrite(output_pins[0], HIGH);
      digitalWrite(output_pins[1], HIGH);
      break;
  }
 
  
}
