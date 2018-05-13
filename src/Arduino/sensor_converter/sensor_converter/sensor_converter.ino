void setup() {
  // put your setup code here, to run once:
  pinMode (A0, INPUT);
  pinMode (A7, INPUT);
  pinMode (2, OUTPUT);  
  Serial.begin(9600);
}

void loop() {
  int reference = analogRead(A7);
  int val = analogRead(A0);
  int presence;
  if (val < reference/3*2)
    presence = 1;
  else
    presence = 0;

  if (presence == 1)
    digitalWrite(2, HIGH);
  else
    digitalWrite(2, LOW);
  
  Serial.print("ref: ");
  Serial.print(reference);  
  Serial.print(" val: ");
  Serial.print(val);
  Serial.print(" presence: ");
  Serial.println(presence);
}
