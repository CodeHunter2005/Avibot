import cv2
import math
import time
import socket
from ultralytics import YOLO
import numpy as np

# =============================
# CONFIGURATION
# =============================
MODEL_PATH = "best.pt"
CLASS_NAME = "target"
REAL_HEIGHT = 0.20
FOCAL_LENGTH = 700

# Robot parameters
WHEEL_RADIUS = 0.05
ENCODER_RESOLUTION = 360
AXLE_LENGTH = 0.15

# Control parameters
DESIRED_DISTANCE = 0.30
MAX_SPEED = 0.5  # Normalized speed (0.0 to 1.0)
ANGLE_THRESHOLD = 15  # degrees

# UDP Communication
ARDUINO_IP = "192.168.1.100"  # Replace with your Arduino's IP
ARDUINO_PORT = 1234
# =============================

class UDPMotorController:
    def __init__(self, arduino_ip, arduino_port):
        self.arduino_ip = arduino_ip
        self.arduino_port = arduino_port
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.settimeout(0.1)  # 100ms timeout
        
    def send_motor_command(self, left_speed, right_speed, request_encoders=False):
        """Send normalized motor speeds (-1.0 to 1.0) to Arduino"""
        # Format: "left_speed,right_speed,encoder_request"
        encoder_flag = 1 if request_encoders else 0
        command = f"{left_speed:.3f},{right_speed:.3f},{encoder_flag}"
        
        try:
            self.sock.sendto(command.encode(), (self.arduino_ip, self.arduino_port))
            
            # If encoder data requested, try to receive it
            if request_encoders:
                try:
                    data, addr = self.sock.recvfrom(1024)
                    encoder_str = data.decode().strip()
                    left_enc, right_enc = map(int, encoder_str.split(','))
                    return left_enc, right_enc
                except socket.timeout:
                    print("Timeout receiving encoder data")
                    return None, None
                    
        except Exception as e:
            print(f"Error sending motor command: {e}")
            
        return None, None

class RobotController:
    def __init__(self, max_speed, desired_distance, angle_threshold):
        self.max_speed = max_speed
        self.desired_distance = desired_distance
        self.angle_threshold = math.radians(angle_threshold)
        
    def calculate_commands(self, distance, dx_pixels, frame_width, focal_length):
        """
        Calculate normalized motor speeds (-1.0 to 1.0)
        Returns: (left_speed, right_speed)
        """
        # Stop if no valid distance or too close
        if distance <= 0 or distance <= self.desired_distance:
            return 0, 0
        
        # Calculate angular error
        angle_error = math.atan2(dx_pixels, focal_length)
        
        # Control logic
        if abs(angle_error) > self.angle_threshold:
            # Alignment phase: rotate toward object
            rotation_gain = 0.8
            rotation_speed = rotation_gain * self.max_speed
            
            if angle_error > 0:
                # Object is to the right, rotate clockwise
                return rotation_speed, -rotation_speed
            else:
                # Object is to the left, rotate counter-clockwise
                return -rotation_speed, rotation_speed
        else:
            # Approach phase: move forward with correction
            distance_gain = 0.6
            correction_gain = 0.4
            
            base_speed = distance_gain * self.max_speed * min(1.0, (distance - self.desired_distance) / 2.0)
            correction = correction_gain * self.max_speed * angle_error
            
            left_speed = base_speed - correction
            right_speed = base_speed + correction
            
            # Normalize to max speed
            max_magnitude = max(abs(left_speed), abs(right_speed))
            if max_magnitude > self.max_speed:
                scale = self.max_speed / max_magnitude
                left_speed *= scale
                right_speed *= scale
                
            return left_speed, right_speed

# Initialize components
model = YOLO(MODEL_PATH)
motor_controller = UDPMotorController(ARDUINO_IP, ARDUINO_PORT)
controller = RobotController(MAX_SPEED, DESIRED_DISTANCE, ANGLE_THRESHOLD)

# Encoder tracking (optional)
left_encoder_total = 0
right_encoder_total = 0
last_encoder_time = time.time()

# Open camera
cap = cv2.VideoCapture(0)
if not cap.isOpened():
    print("Error: Camera not accessible")
    exit()

print("Starting visual navigation with Arduino control...")
print("Press ESC to exit")

try:
    frame_count = 0
    while True:
        ret, frame = cap.read()
        if not ret:
            break
            
        frame_count += 1
        
        # Run YOLO inference
        results = model(frame, verbose=False)
        
        object_detected = False
        current_distance = -1
        dx_pixels = 0
        left_speed, right_speed = 0, 0
        
        # Process detections
        for r in results:
            boxes = r.boxes
            for box in boxes:
                cls_id = int(box.cls[0])
                label = model.names[cls_id]
                conf = float(box.conf[0])

                if label == CLASS_NAME and conf > 0.5:  # Confidence threshold
                    object_detected = True
                    
                    # Get bounding box
                    x1, y1, x2, y2 = box.xyxy[0].cpu().numpy()
                    x1, y1, x2, y2 = int(x1), int(y1), int(x2), int(y2)
                    h = y2 - y1

                    # Compute distance
                    if h > 0:
                        current_distance = (REAL_HEIGHT * FOCAL_LENGTH) / h
                    else:
                        current_distance = -1

                    # Compute center offset
                    cx = (x1 + x2) // 2
                    frame_cx = frame.shape[1] // 2
                    dx_pixels = cx - frame_cx

                    # Calculate motor commands
                    left_speed, right_speed = controller.calculate_commands(
                        current_distance, dx_pixels, frame.shape[1], FOCAL_LENGTH
                    )
                    
                    # Request encoder data every 10 frames
                    request_encoders = (frame_count % 10 == 0)
                    left_enc, right_enc = motor_controller.send_motor_command(
                        left_speed, right_speed, request_encoders
                    )
                    
                    if left_enc is not None and right_enc is not None:
                        left_encoder_total = left_enc
                        right_encoder_total = right_enc

                    # Draw visualization
                    cv2.rectangle(frame, (x1, y1), (x2, y2), (0, 255, 0), 2)
                    cv2.circle(frame, (cx, frame.shape[0]//2), 5, (0, 0, 255), -1)
                    cv2.circle(frame, (frame_cx, frame.shape[0]//2), 3, (255, 255, 255), -1)
                    cv2.line(frame, (frame_cx, frame.shape[0]//2), (cx, frame.shape[0]//2), (255, 0, 0), 2)
                    
                    break  # Process only the first detection

        if not object_detected:
            # No target detected - stop motors
            motor_controller.send_motor_command(0, 0)
            left_speed, right_speed = 0, 0

        # Display information
        status = "TARGET FOUND" if object_detected else "SEARCHING"
        cv2.putText(frame, f"Status: {status}", (10, 30), 
                   cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)
        
        if object_detected:
            info_text = [
                f"Distance: {current_distance:.2f} m",
                f"Offset: {dx_pixels} px", 
                f"Motors: L={left_speed:.2f} R={right_speed:.2f}",
                f"Encoders: L={left_encoder_total} R={right_encoder_total}"
            ]
            
            for i, text in enumerate(info_text):
                cv2.putText(frame, text, (10, 60 + i*25), 
                           cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 255), 2)

        cv2.imshow("Visual Navigation with Arduino", frame)

        if cv2.waitKey(1) & 0xFF == 27:
            break

except KeyboardInterrupt:
    print("Stopping...")

finally:
    # Cleanup
    cap.release()
    cv2.destroyAllWindows()
    # Stop motors
    motor_controller.send_motor_command(0, 0)
    print("Navigation stopped - motors halted")