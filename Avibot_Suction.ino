#define ENA 14  // PWM pin for speed control (D5)
#define IN1 12  // Direction pin (D6)
#define IN2 13  // Direction pin (D7)

void setup() {
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
}

void loop() {
  // Move Forward at 50% speed
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, 128);  // 128 out of 255 (~50% speed)
  delay(5000);

  // Stop Motor
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  delay(2000);

  // Move Backward at Full Speed
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  analogWrite(ENA, 255);  // Full speed
  delay(5000);

  // Stop Motor
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  delay(2000);
}
