#include <RPLidar.h>

#define RPLIDAR_MOTOR 3  // Define the motor control pin

RPLidar lidar;

void setup() {
  Serial.begin(115200);
  lidar.begin(Serial1);  // Assuming the RPLidar is connected to Serial1 (adjust as needed)
  pinMode(RPLIDAR_MOTOR, OUTPUT);
  digitalWrite(RPLIDAR_MOTOR, HIGH);  // Start the motor
}

void loop() {
  if (IS_OK(lidar.waitPoint())) {  // Wait for a point from the LIDAR
    float distance = lidar.getCurrentPoint().distance;  // Distance in mm
    float angle = lidar.getCurrentPoint().angle;        // Angle in degrees
    
    // Control logic based on LIDAR data:
    if (distance > 0 && distance < 500) {  // If an obstacle is too close (e.g., 500 mm)
      stopMotors();
      Serial.println("Obstacle detected, stopping!");
    } else {
      moveForward();
    }
  } else {
    Serial.println("Error reading from RPLidar.");
    lidar.reset();  // Reset the sensor if there is an error
  }
}

void moveForward() {
  // Code to move forward
  digitalWrite(RPLIDAR_MOTOR, HIGH);  // Keep motor running forward
}

void stopMotors() {
  // Code to stop motors
  digitalWrite(RPLIDAR_MOTOR, LOW);  // Stop motor
}

void stopLidar() {
  digitalWrite(RPLIDAR_MOTOR, LOW);  // Stop the motor
  lidar.end();
}
