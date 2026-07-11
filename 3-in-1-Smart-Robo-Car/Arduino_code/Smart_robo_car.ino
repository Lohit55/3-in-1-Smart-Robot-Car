// Custom 3-in-1 Robot Car Code (IR Remote, Obstacle Avoidance, Line Following)
// No Bluetooth modules required.

#include <IRremote.h>

const int RECV_PIN = A5; // IR Receiver Pin

// Motor Driver Pins
#define enA 10  
#define in1 9  
#define in2 8  
#define in3 7  
#define in4 6  
#define enB 5  

// Sensors & Servo Pins
#define servo A4  
#define R_S A0  
#define L_S A1  
#define echo A2  
#define trigger A3  

// Global Variables
int distance_L, distance_F = 30, distance_R;  
long distance;  
int set = 20; // Obstacle distance threshold in cm
  
int ir_data = 5; // Start in "Stop" state
int Speed = 130; // Default speed
int mode = 0;    // 0 = Manual IR, 1 = Line Follower, 2 = Obstacle Avoidance

void setup() {  
  pinMode(R_S, INPUT);  
  pinMode(L_S, INPUT);  
  pinMode(echo, INPUT);  
  pinMode(trigger, OUTPUT);  
  pinMode(enA, OUTPUT);  
  pinMode(in1, OUTPUT);  
  pinMode(in2, OUTPUT);  
  pinMode(in3, OUTPUT);  
  pinMode(in4, OUTPUT);  
  pinMode(enB, OUTPUT);  
  pinMode(servo, OUTPUT);  
  
  Serial.begin(9600);  
  
  // Start the receiver
  IrReceiver.begin(RECV_PIN, ENABLE_LED_FEEDBACK);  
  
  // Boot-up servo test sweep
  for (int angle = 70; angle <= 140; angle += 5) servoPulse(servo, angle);  
  for (int angle = 140; angle >= 0; angle -= 5) servoPulse(servo, angle);  
  for (int angle = 0; angle <= 70; angle += 5) servoPulse(servo, angle);  
  delay(500);  
  
  Serial.println("Robot Ready! Open Serial Monitor and press remote buttons.");
}  

void loop() {  
  // Read incoming IR Signals
  if (IrReceiver.decode()) {  
    // This command decodes command data cleanly across almost all IRremote library versions
    unsigned long irCode = IrReceiver.decodedIRData.command;  
    
    // CRITICAL: Watch your Serial Monitor to find these numbers!
    Serial.print("Button Pressed! Command Hex code is: 0x");  
    Serial.println(irCode, HEX);  
    
    ir_data = IRremote_data(irCode);  
    IrReceiver.resume();  
    delay(150);  
  }  
  
  // Switch Modes based on remote input
  if (ir_data == 8) { mode = 0; Stop(); }         // Mode Button 1 -> Manual
  else if (ir_data == 9) { mode = 1; Speed = 130; }  // Mode Button 2 -> Line Follower
  else if (ir_data == 10) { mode = 2; Speed = 200; } // Mode Button 3 -> Obstacle Avoidance  
  
  analogWrite(enA, Speed);  
  analogWrite(enB, Speed);  
  
  // === MODE 0: IR MANUAL REMOTE CONTROL ===
  if (mode == 0) {  
    if (ir_data == 1) forword();  
    else if (ir_data == 2) backword();  
    else if (ir_data == 3) turnLeft();  
    else if (ir_data == 4) turnRight();  
    else if (ir_data == 5) Stop();  
  }  
  
  // === MODE 1: LINE FOLLOWING ===
  if (mode == 1) {  
    // 0 = Reflective surface (White), 1 = Non-reflective surface (Black line)
    if ((digitalRead(R_S) == 0) && (digitalRead(L_S) == 0)) forword();  
    else if ((digitalRead(R_S) == 1) && (digitalRead(L_S) == 0)) turnRight();  
    else if ((digitalRead(R_S) == 0) && (digitalRead(L_S) == 1)) turnLeft();  
    else Stop();  
  }  
  
  // === MODE 2: OBSTACLE AVOIDANCE ===
  if (mode == 2) {  
    distance_F = Ultrasonic_read();  
    if (distance_F > set) {
      forword();  
    } else {
      Check_side();  
    }
  }  
  delay(10);  
}  

// === IR REMOTE MAPPING FUNCTION ===
// STEP 2: Replace the custom hex commands below (e.g., 0x18) with the 
// specific codes you see in your Serial Monitor when you press your remote keys!
int IRremote_data(unsigned long code) {  
  if (code == 0x18) return 1;       // Forward Button (Example: Vol+)
  else if (code == 0x52) return 2;  // Backward Button (Example: Vol-)
  else if (code == 0x08) return 3;  // Left Button
  else if (code == 0x5A) return 4;  // Right Button
  else if (code == 0x1C) return 5;  // OK / Stop Button
    
  else if (code == 0x16) return 8;  // Button '1' -> Switch to Manual Mode   
  else if (code == 0x19) return 9;  // Button '2' -> Switch to Line Follow Mode   
  else if (code == 0x0D) return 10; // Button '3' -> Switch to Obstacle Avoid Mode   
  return 0;  
}  

// Servo Pulse Generation
void servoPulse (int pin, int angle) {  
  int pwm = (angle * 11) + 500;  
  digitalWrite(pin, HIGH);  
  delayMicroseconds(pwm);  
  digitalWrite(pin, LOW);  
  delay(50);  
}  

// HC-SR04 Distance Measurement
long Ultrasonic_read() {  
  digitalWrite(trigger, LOW);  
  delayMicroseconds(2);  
  digitalWrite(trigger, HIGH);  
  delayMicroseconds(10);  
  digitalWrite(trigger, LOW);
  distance = pulseIn(echo, HIGH, 30000); // 30ms timeout prevents freezing
  return distance / 29 / 2;  
}  

// Obstacle Avoidance Routing
void compareDistance() {  
  if (distance_L > distance_R) {
    turnLeft();
    delay(400);
  } else if (distance_R > distance_L) {
    turnRight();
    delay(400);
  } else { 
    backword(); 
    delay(400); 
    turnRight(); 
    delay(600); 
  }  
  ir_data = 0; // Clear state after maneuver
}  

void Check_side() {  
  Stop();  
  delay(200);  
  for (int angle = 70; angle <= 140; angle += 5) servoPulse(servo, angle);  
  delay(300);  
  distance_L = Ultrasonic_read();  
  delay(100);  
  for (int angle = 140; angle >= 0; angle -= 5) servoPulse(servo, angle);  
  delay(500);  
  distance_R = Ultrasonic_read();  
  delay(100);  
  for (int angle = 0; angle <= 70; angle += 5) servoPulse(servo, angle);  
  delay(300);  
  compareDistance();  
}  

// Movement Definitions
void forword() {  
  digitalWrite(in1, HIGH); digitalWrite(in2, LOW);  
  digitalWrite(in3, LOW);  digitalWrite(in4, HIGH);  
}  

void backword() {  
  digitalWrite(in1, LOW);  digitalWrite(in2, HIGH);  
  digitalWrite(in3, HIGH); digitalWrite(in4, LOW);  
}  

void turnRight() {  
  digitalWrite(in1, LOW);  digitalWrite(in2, HIGH);  
  digitalWrite(in3, LOW);  digitalWrite(in4, HIGH);  
}  

void turnLeft() {  
  digitalWrite(in1, HIGH); digitalWrite(in2, LOW);  
  digitalWrite(in3, HIGH); digitalWrite(in4, LOW);  
}  

void Stop() {  
  digitalWrite(in1, LOW);  digitalWrite(in2, LOW);  
  digitalWrite(in3, LOW);  digitalWrite(in4, LOW);  
}  