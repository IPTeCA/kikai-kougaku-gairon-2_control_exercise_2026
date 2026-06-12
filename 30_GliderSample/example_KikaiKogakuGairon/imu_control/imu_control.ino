// 対象ボード: Seeed XIAO nRF52840

#include <LSM6DS3.h>
#include <MadgwickAHRS.h>
#include <Wire.h>
#include <Servo.h>

// --- Defines ---
#define LED_OFF do { digitalWrite(LED_RED, HIGH); digitalWrite(LED_GREEN, HIGH); digitalWrite(LED_BLUE, HIGH); } while(0)
#define RED_ON do { LED_OFF; digitalWrite(LED_RED, LOW); } while(0)
#define GREEN_ON do { LED_OFF; digitalWrite(LED_GREEN, LOW); } while(0)
#define CYAN_ON do { LED_OFF; digitalWrite(LED_GREEN, LOW); digitalWrite(LED_BLUE, LOW); } while(0)
#define YELLOW_ON do { LED_OFF; digitalWrite(LED_RED, LOW); digitalWrite(LED_GREEN, LOW); } while(0)

#define MEASURING_FREQ 105.0

LSM6DS3 IMU(I2C_MODE, 0x6A);
Madgwick filter;
Servo servo[3];

struct pos3d_t { float x, y, z; };
struct SerialBuffer {
  char buf[32];
  int idx = 0;
  unsigned long last_rx_time = 0;
};
pos3d_t gyr_ = {0}, acc_ = {0}, ang_ = {0};

unsigned long t=0, t_=0;

// --- Configurable Parameters ---
float kp_pit = 2.0; float kd_pit = 3.0; float ki_pit = 0.0;
float kp_rol = 2.0; float kd_rol = 3.0; float ki_rol = 0.0;
int ang_goal = 0;

// Trim (L, R, E)
int ang_trim[] = {85, 105, 70}; 

// --- System Variables ---
float cur_servo_ang[3] = {85.0, 105.0, 70.0}; 
#define SERVO_STEP_LIMIT 3.0 

float preangx=0, preangy=0, dx, dy, ix=0, iy=0, T_ms;
int mode = 2; // 1 = Auto, 2 = Manual (Start in Manual)
bool param_changed = false;

// Auto Boost & Launch Detect
unsigned long launch_time = 0;
bool launch_detected = false;
bool gain_boosted = false;
const float LAUNCH_THRESHOLD = -3.0;       
const unsigned long BOOST_DELAY_MS = 1000; 

// UART
HardwareSerial& OUT = Serial1; 
const unsigned OUT_BAUD = 115200;
const unsigned DBG_BAUD = 115200;
uint32_t seq = 0;
unsigned long prev_ms = 0;

void sendParamLog() {
  char buf[256];
  snprintf(buf, sizeof(buf), 
    "LOG,Param: PP=%.2f PI=%.2f PD=%.2f | RP=%.2f RI=%.2f RD=%.2f | TL=%d TR=%d TE=%d | G=%d", 
    kp_pit, ki_pit, kd_pit, kp_rol, ki_rol, kd_rol, 
    ang_trim[0], ang_trim[1], ang_trim[2], ang_goal);
  Serial.println(buf); OUT.println(buf);
  param_changed = false;
}

void printSaveCode() {
  Serial.println("LOG,Code printed to Serial Monitor.");
  OUT.println("LOG,Code printed to Serial Monitor.");
}

void processCommand(char c) {
  // --- Mode Change ---
  if (c=='A' || c=='a') { 
    mode=1; GREEN_ON; 
    ix=0; iy=0; 
    launch_detected = false; 
    gain_boosted = false;
    Serial.println("LOG,mode:Auto"); OUT.println("LOG,mode:Auto"); 
  }
  else if (c=='M' || c=='m') { 
    mode=2; CYAN_ON;  
    Serial.println("LOG,mode:Manual"); OUT.println("LOG,mode:Manual"); 
  }
  
  // --- Gains (PID) ---
  else if (c=='p') { kp_pit -= 0.1; param_changed=true; } else if (c=='P') { kp_pit += 0.1; param_changed=true; }
  else if (c=='d') { kd_pit -= 0.1; param_changed=true; } else if (c=='D') { kd_pit += 0.1; param_changed=true; }
  else if (c=='i') { ki_pit -= 0.01; param_changed=true; } else if (c=='I') { ki_pit += 0.01; param_changed=true; }
  else if (c=='x') { kp_rol -= 0.1; param_changed=true; } else if (c=='X') { kp_rol += 0.1; param_changed=true; }
  else if (c=='y') { kd_rol -= 0.1; param_changed=true; } else if (c=='Y') { kd_rol += 0.1; param_changed=true; }
  else if (c=='z') { ki_rol -= 0.01; param_changed=true; } else if (c=='Z') { ki_rol += 0.01; param_changed=true; }
  else if (c=='g') { ang_goal -= 1; param_changed=true; } else if (c=='G') { ang_goal += 1; param_changed=true; }
  
  // --- Trim (L, R, E) +/- 1 deg ---
  else if (c=='l') { ang_trim[0] = constrain(ang_trim[0] - 1, 15, 165); param_changed=true; } 
  else if (c=='L') { ang_trim[0] = constrain(ang_trim[0] + 1, 15, 165); param_changed=true; }
  else if (c=='r') { ang_trim[1] = constrain(ang_trim[1] - 1, 15, 165); param_changed=true; } 
  else if (c=='R') { ang_trim[1] = constrain(ang_trim[1] + 1, 15, 165); param_changed=true; }
  else if (c=='e') { ang_trim[2] = constrain(ang_trim[2] - 1, 15, 165); param_changed=true; } 
  else if (c=='E') { ang_trim[2] = constrain(ang_trim[2] + 1, 15, 165); param_changed=true; }

  // --- Center Command ---
  else if (c=='c' || c=='C') {
    ang_trim[0] = 90; ang_trim[1] = 90; ang_trim[2] = 90; 
    param_changed=true; 
  }
  
  // --- Save Code ---
  else if (c=='s' || c=='S') {
    printSaveCode();
  }

  // --- Reset sequence and send parameters ---
  else if (c=='h' || c=='H') {
    seq = 0;
    sendParamLog(); 
  }
}

void updateServoSmooth(int id, float target_angle) {
  if (target_angle < 15.0) target_angle = 15.0;
  if (target_angle > 165.0) target_angle = 165.0;

  float diff = target_angle - cur_servo_ang[id];
  if (diff > SERVO_STEP_LIMIT) diff = SERVO_STEP_LIMIT;
  if (diff < -SERVO_STEP_LIMIT) diff = -SERVO_STEP_LIMIT;

  cur_servo_ang[id] += diff;
  servo[id].write((int)cur_servo_ang[id]);
}

SerialBuffer serialBuf;
SerialBuffer outBuf;

void handleSerialInput(Stream& port, SerialBuffer& sbuf) {
  while (port.available()) {
    char c = (char)port.read();
    sbuf.last_rx_time = millis();
    if (c == '\n' || c == '\r') {
      if (sbuf.idx == 1) {
        processCommand(sbuf.buf[0]);
      }
      sbuf.idx = 0;
    } else {
      if (sbuf.idx < 30) {
        sbuf.buf[sbuf.idx++] = c;
      } else {
        sbuf.idx = 0;
      }
    }
  }

  // If 50ms passed since last RX, check if we have a single character command
  if (sbuf.idx == 1 && (millis() - sbuf.last_rx_time > 50)) {
    processCommand(sbuf.buf[0]);
    sbuf.idx = 0;
  } else if (sbuf.idx > 1 && (millis() - sbuf.last_rx_time > 50)) {
    sbuf.idx = 0; // Discard multi-character junk
  }
}

void setup() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  LED_OFF;

  Serial.begin(DBG_BAUD);
  OUT.begin(OUT_BAUD);

  IMU.settings.gyroRange = 2000;
  IMU.settings.accelRange = 4;
  while (IMU.begin() != 0){ delay(200); RED_ON; delay(200); LED_OFF; }

  filter.begin((int)MEASURING_FREQ);
  
  servo[0].attach(0); 
  servo[1].attach(1); 
  servo[2].attach(2);

  // Initialize servo positions
  cur_servo_ang[0] = ang_trim[0];
  cur_servo_ang[1] = ang_trim[1];
  cur_servo_ang[2] = ang_trim[2];
  updateServoSmooth(0, cur_servo_ang[0]);
  updateServoSmooth(1, cur_servo_ang[1]);
  updateServoSmooth(2, cur_servo_ang[2]);

  delay(100);
  
  Serial.println("LOG,mode:Manual"); OUT.println("LOG,mode:Manual");
  CYAN_ON;
}

void loop() {
  t_ = t; t = millis();
  handleSerialInput(Serial, serialBuf);
  handleSerialInput(OUT, outBuf);

  if (param_changed) sendParamLog();

  static unsigned long last_info_ms = 0;
  if (t - last_info_ms > 2000) {
    last_info_ms = t;
    sendParamLog();
  }

  T_ms = (float)(t - t_); 
  if(T_ms<=0) T_ms=1;
  
  acc_.x = IMU.readFloatAccelX(); acc_.y = IMU.readFloatAccelY(); acc_.z = IMU.readFloatAccelZ();
  gyr_.x = IMU.readFloatGyroX();  gyr_.y = IMU.readFloatGyroY();  gyr_.z = IMU.readFloatGyroZ();
  filter.updateIMU(gyr_.x, gyr_.y, gyr_.z, acc_.x, acc_.y, acc_.z);
  ang_.x = filter.getRoll(); ang_.y = filter.getPitch(); ang_.z = filter.getYaw();

  if (mode == 1){ 
    // --- Auto Mode ---
    if (!launch_detected) {
      if (acc_.x < LAUNCH_THRESHOLD) {
        launch_detected = true;
        launch_time = t; 
        Serial.println("LOG,Launch Detected!"); OUT.println("LOG,Launch Detected!");
      }
    } 
    else if (!gain_boosted && (t - launch_time > BOOST_DELAY_MS)) {
      kp_rol *= 2.0; kd_rol *= 2.0; ki_rol *= 2.0;
      gain_boosted = true;
      YELLOW_ON; 
      Serial.println("LOG,Boost: Roll Gains Doubled!"); OUT.println("LOG,Boost: Roll Gains Doubled!");
      sendParamLog();
    }

    dx = (ang_.x - preangx)/T_ms; dy = (ang_.y - preangy)/T_ms;
    ix += (ang_.x + preangx)*T_ms/2000.0; iy += (ang_.y + preangy)*T_ms/2000.0;

    // Control (Aileron Reversed)
    float roll_control = (ang_.x) * kp_rol - dx * kd_rol + ix * ki_rol;
    
    float target0 = ang_trim[0] - roll_control; 
    float target1 = ang_trim[1] - roll_control; 
    float target2 = ang_trim[2] + (ang_.y - ang_goal) * kp_pit + dy * kd_pit + iy * ki_pit;
    
    updateServoSmooth(0, target0);
    updateServoSmooth(1, target1);
    updateServoSmooth(2, target2);

    preangx = ang_.x; preangy = ang_.y;
  }
  else if (mode == 2){
    // Manual Mode
    updateServoSmooth(0, (float)ang_trim[0]);
    updateServoSmooth(1, (float)ang_trim[1]);
    updateServoSmooth(2, (float)ang_trim[2]);
    ix = 0; iy = 0; preangx = ang_.x; preangy = ang_.y;
  }

  float dt_ms = (float)(t - prev_ms); prev_ms = t;
  char line[128];
  
  snprintf(line, sizeof(line), "DAT,%lu,%lu,%.1f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f",
    ++seq, t, dt_ms, 
    acc_.x, acc_.y, acc_.z, 
    gyr_.x, gyr_.y, gyr_.z, 
    ang_.x, ang_.y, ang_.z,
    cur_servo_ang[0], cur_servo_ang[1], cur_servo_ang[2]);
  
  Serial.println(line); OUT.println(line);
  delay(15);
}
