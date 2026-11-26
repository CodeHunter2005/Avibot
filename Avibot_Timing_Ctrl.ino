// Define motor driver pins
#define DRIVE_MOTOR_1_DIR 23      // Direction pin for Right Motor (GPIO4)
#define DRIVE_MOTOR_1_PWM 22      // PWM pin for Right Motor (GPIO5)
#define DRIVE_MOTOR_2_DIR 21      // Direction pin for Left Motor (GPIO2)
#define DRIVE_MOTOR_2_PWM 19      // PWM pin for Left Motor (GPIO0)
#define CENTER_MOTOR_1_DIR 18      // Direction pin for Center Motor
#define CENTER_MOTOR_1_PWM 5      // PWM pin for Center Motor
#define VACUUM_MOTOR_2_DIR 4      // Direction pin for Vacuum Motor
#define VACUUM_MOTOR_2_PWM 2      // PWM pin for Vacuum Motor
#define TRAY_MOTOR_1_DIR 15      // Direction pin for Tray Motor
#define TRAY_MOTOR_1_PWM 25      // PWM pin for Tray Motor
#define TILT_MOTOR_2_DIR 32      // Direction pin for Right Motor (GPIO4)
#define TILT_MOTOR_2_PWM 33      // PWM pin for Right Motor (GPIO5)

int speed = 540; // Adjusted speed value

void setup() {
  // Set motor control pins as outputs
  Serial.begin(9600);
  pinMode(DRIVE_MOTOR_1_PWM, OUTPUT);
  pinMode(DRIVE_MOTOR_1_DIR, OUTPUT);
  pinMode(DRIVE_MOTOR_2_PWM, OUTPUT);
  pinMode(DRIVE_MOTOR_2_DIR, OUTPUT);
  pinMode(CENTER_MOTOR_1_PWM, OUTPUT);
  pinMode(CENTER_MOTOR_1_DIR, OUTPUT);
  pinMode(VACUUM_MOTOR_2_PWM, OUTPUT);
  pinMode(VACUUM_MOTOR_2_DIR, OUTPUT);
  pinMode(TRAY_MOTOR_1_PWM, OUTPUT);
  pinMode(TRAY_MOTOR_1_DIR, OUTPUT);
  pinMode(TILT_MOTOR_2_PWM, OUTPUT);
  pinMode(TILT_MOTOR_2_DIR, OUTPUT);
  stopMotors();  // Ensure motors are stopped at startup
  stopVacuum();
}

void loop() {
  if (Serial.available() > 0) {  
    char command = Serial.read();
    if (command == 's') {
      manualControl();
    }
  }
}

void manualControl(){
  activateMotor(CENTER_MOTOR_1_DIR, CENTER_MOTOR_1_PWM, LOW);
  delay(2000);
  stopMotors();
  delay(1500);
  activateMotor(VACUUM_MOTOR_2_DIR, VACUUM_MOTOR_2_PWM, HIGH);
  delay(2000);
  stopMotors();
  delay(1500);
  activateMotor(CENTER_MOTOR_1_DIR, CENTER_MOTOR_1_PWM, HIGH);
  delay(2300);
  stopMotors();
  delay(1500);
  activateMotor(TRAY_MOTOR_1_DIR, TRAY_MOTOR_1_PWM, LOW);
  delay(1500);
  stopMotors();
  delay(900);
  stopVacuum();
  delay(2500);
  activateMotor(TRAY_MOTOR_1_DIR, TRAY_MOTOR_1_PWM, HIGH);
  delay(1500);
  stopMotors();
  delay(2000);
  activateMotor(TILT_MOTOR_2_DIR, TILT_MOTOR_2_PWM, LOW);
  delay(900);
  stopMotors();
  delay(1000);
  activateMotor(TILT_MOTOR_2_DIR, TILT_MOTOR_2_PWM, HIGH);
  delay(900);
  stopMotors();
}

void activateMotor(int dirPin, int pwmPin, int direction) {
  digitalWrite(dirPin, direction);
  analogWrite(pwmPin, speed);
}

// Function to move forward
void moveForward(int speed) {
  digitalWrite(DRIVE_MOTOR_1_DIR, HIGH); // Set direction for Motor A
  digitalWrite(DRIVE_MOTOR_2_DIR, HIGH); // Set direction for Motor B
  analogWrite(DRIVE_MOTOR_1_PWM, speed); // Set speed
  analogWrite(DRIVE_MOTOR_2_PWM, speed);
}

// Function to move backward
void moveBackward(int speed) {
  digitalWrite(DRIVE_MOTOR_1_DIR, LOW); // Reverse direction for Motor A
  digitalWrite(DRIVE_MOTOR_2_DIR, LOW); // Reverse direction for Motor B
  analogWrite(DRIVE_MOTOR_1_PWM, speed); // Set speed
  analogWrite(DRIVE_MOTOR_2_PWM, speed);
}

// Function to turn left
void turnLeft(int speed) {
  digitalWrite(DRIVE_MOTOR_1_DIR, HIGH);  // Reverse direction for Motor A
  digitalWrite(DRIVE_MOTOR_2_DIR, LOW); // Forward direction for Motor B
  analogWrite(DRIVE_MOTOR_1_PWM, speed); // Set speed
  analogWrite(DRIVE_MOTOR_2_PWM, speed);
}

// Function to turn right
void turnRight(int speed) {
  digitalWrite(DRIVE_MOTOR_1_DIR, LOW); // Forward direction for Motor A
  digitalWrite(DRIVE_MOTOR_2_DIR, HIGH);  // Reverse direction for Motor B
  analogWrite(DRIVE_MOTOR_1_PWM, speed); // Set speed 
  analogWrite(DRIVE_MOTOR_2_PWM, speed);
}

// Function to stop the motors
void stopMotors() {
  analogWrite(DRIVE_MOTOR_1_PWM, 0);
  analogWrite(DRIVE_MOTOR_2_PWM, 0);
  analogWrite(CENTER_MOTOR_1_PWM, 0);
  analogWrite(TRAY_MOTOR_1_PWM, 0);
  analogWrite(TILT_MOTOR_2_PWM, 0);
}
void stopVacuum(){
  analogWrite(VACUUM_MOTOR_2_PWM, 0);
}
