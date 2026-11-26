// Controlling tray Motor, suction motor with two limit switches

const int trayMotor_IN1 = D4;      // Motor control pin 1
const int trayMotor_IN2 = D5;      // Motor control pin 2

const int vacuumMotor = D6;    // Vacuum motor control pin
const int ENA1 = D1;           // Motor speed control

const int topSwitch = D3;      // Limit switch at top
const int bottomSwitch = D2;   // Limit switch at bottom

void setup() {
  Serial.begin(9600);
  
  pinMode(trayMotor_IN1, OUTPUT);
  pinMode(trayMotor_IN2, OUTPUT);
  pinMode(vacuumMotor, OUTPUT);
  pinMode(topSwitch, INPUT);
  pinMode(bottomSwitch, INPUT);
  pinMode(ENA1, OUTPUT);
}

void loop() {
  int topState = digitalRead(topSwitch);
  int bottomState = digitalRead(bottomSwitch);

  Serial.print("Top Switch: ");
  Serial.print(topState);
  Serial.print(" | Bottom Switch: ");
  Serial.println(bottomState);

  if (topState == HIGH) {
    trayMotor_down();
    delay(2000);  // Delay for stopping vacuum motor (change delay by trial & error)
    trayMotor_stop();
    vacuumMotor_stop();
    delay(1000);
    trayMotor_down();
  } 
  else if (bottomState == HIGH) {
    trayMotor_stop();
    vacuumMotor_start();
    delay(2000);  
    trayMotor_up();
  }
}

void trayMotor_up() {
  digitalWrite(trayMotor_IN2, LOW);
  digitalWrite(trayMotor_IN1, HIGH);
  analogWrite(ENA1, 200);
  Serial.println("Tray moving up");
}

void trayMotor_down() {
  digitalWrite(trayMotor_IN1, LOW);
  digitalWrite(trayMotor_IN2, HIGH)
  analogWrite(ENA1, 200);
  Serial.println("Tray moving down");
}

void trayMotor_stop() {
  digitalWrite(trayMotor_IN1, LOW);
  digitalWrite(trayMotor_IN2, LOW);
  analogWrite(ENA1, 0);
  Serial.println("Tray stopped");
}

void vacuumMotor_start() {
  Serial.println("starting vacuum motor...");
  for (int speed = 0; speed <= 255; speed += 30) {  // Slowly increase speed (trial & error)
    analogWrite(vacuumMotor, speed);
    delay(30);
  }
}

void vacuumMotor_start() {
  Serial.println("Gradually starting vacuum motor...");
  for (int speed = 255; speed <= 0; speed -= 20) {  // Slowly decrease speed (trail & error)
    analogWrite(vacuumMotor, speed);
    delay(30);
  }
}
