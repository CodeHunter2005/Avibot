#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

// WiFi Configuration
const char* ssid = "OnePlus Nord CE4 Lite 5G";
const char* password = "DoNotConnect";
WiFiUDP Udp;
unsigned int localPort = 1234;

// Motor Driver Pins
#define IN1 D1
#define IN2 D2
#define IN3 D3
#define IN4 D4
#define ENA D5  // Left motor PWM
#define ENB D6  // Right motor PWM

// Motor control variables
int left_speed = 0, right_speed = 0;
int max_speed = 255;  // PWM range 0-255
unsigned long last_command_time = 0;
const unsigned long COMMAND_TIMEOUT = 1000;  // Stop if no commands for 1 second

// Encoder simulation (replace with real encoders)
volatile int left_encoder = 0;
volatile int right_encoder = 0;

void setup() {
  Serial.begin(115200);
  
  // Initialize motor pins
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  
  // Stop motors initially
  stop();
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected!");
  
  Udp.begin(localPort);
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());
  Serial.println("Ready to receive motor commands...");
}

void loop() {
  // Check for incoming UDP commands
  if (Udp.parsePacket()) {
    String data = Udp.readStringUntil('\n');
    data.trim();
    processMotorCommand(data);
    last_command_time = millis();
  }
  
  // Safety timeout - stop if no commands received
  if (millis() - last_command_time > COMMAND_TIMEOUT) {
    stop();
    Serial.println("Safety timeout - stopping motors");
    last_command_time = millis();  // Prevent continuous printing
  }
  
  // Simulate encoder movement (replace with real encoder interrupts)
  simulateEncoders();
  
  delay(10);  // Small delay for stability
}

void processMotorCommand(String data) {
  // Expected format: "LEFT_SPEED,RIGHT_SPEED,ENCODER_REQUEST"
  // Example: "0.5,-0.3,1" or "0,0,0"
  
  int firstComma = data.indexOf(',');
  int secondComma = data.indexOf(',', firstComma + 1);
  
  if (firstComma > 0 && secondComma > 0) {
    float left_cmd = data.substring(0, firstComma).toFloat();
    float right_cmd = data.substring(firstComma + 1, secondComma).toFloat();
    int encoder_request = data.substring(secondComma + 1).toInt();
    
    // Convert normalized speeds (-1.0 to 1.0) to PWM (0-255)
    left_speed = constrain(left_cmd * max_speed, -max_speed, max_speed);
    right_speed = constrain(right_cmd * max_speed, -max_speed, max_speed);
    
    // Execute motor movement
    setMotorSpeeds(left_speed, right_speed);
    
    // Send encoder data back if requested
    if (encoder_request == 1) {
      sendEncoderData();
    }
    
    // Debug output
    Serial.printf("CMD: L=%.2f R=%.2f -> PWM: L=%d R=%d\n", 
                  left_cmd, right_cmd, left_speed, right_speed);
  }
}

void setMotorSpeeds(int left_pwm, int right_pwm) {
  // Left motor control
  if (left_pwm > 0) {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    analogWrite(ENA, abs(left_pwm));
  } else if (left_pwm < 0) {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    analogWrite(ENA, abs(left_pwm));
  } else {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    analogWrite(ENA, 0);
  }
  
  // Right motor control
  if (right_pwm > 0) {
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    analogWrite(ENB, abs(right_pwm));
  } else if (right_pwm < 0) {
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    analogWrite(ENB, abs(right_pwm));
  } else {
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
    analogWrite(ENB, 0);
  }
}

void sendEncoderData() {
  // Send encoder counts back to Python
  String encoder_data = String(left_encoder) + "," + String(right_encoder);
  Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
  Udp.write(encoder_data.c_str());
  Udp.endPacket();
  
  Serial.printf("Sent encoders: L=%d R=%d\n", left_encoder, right_encoder);
}

void stop() {
  left_speed = 0;
  right_speed = 0;
  setMotorSpeeds(0, 0);
}

void simulateEncoders() {
  // Simulate encoder ticks based on motor movement
  // Replace this with real encoder interrupt handlers
  
  static unsigned long last_encoder_update = 0;
  if (millis() - last_encoder_update > 100) {  // Update every 100ms
    // Simple simulation: encoders tick when motors are moving
    if (left_speed != 0) {
      left_encoder += abs(left_speed) / 10;
    }
    if (right_speed != 0) {
      right_encoder += abs(right_speed) / 10;
    }
    last_encoder_update = millis();
  }
}

// Real encoder interrupt handlers (uncomment when you have real encoders)
/*
void leftEncoderISR() {
  left_encoder++;
}

void rightEncoderISR() {
  right_encoder++;
}
*/