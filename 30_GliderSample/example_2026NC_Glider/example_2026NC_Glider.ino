/*
   Student MCU Firmware (Line Passthrough Telemetry)
   Board: Seeed XIAO nRF52840 Sense
*/
#include <LSM6DS3.h>
#include <Wire.h>
#include <MadgwickAHRS.h>
#include <Servo.h>
#include <Arduino.h>
#include <nrf_timer.h>

#define LED_OFF  digitalWrite(LED_RED, HIGH);digitalWrite(LED_BLUE, HIGH);digitalWrite(LED_GREEN, HIGH);
#define RED_ON   digitalWrite(LED_RED, LOW);digitalWrite(LED_BLUE, HIGH);digitalWrite(LED_GREEN, HIGH);
#define BLUE_ON  digitalWrite(LED_RED, HIGH);digitalWrite(LED_BLUE, LOW);digitalWrite(LED_GREEN, HIGH);
#define GREEN_ON digitalWrite(LED_RED, HIGH);digitalWrite(LED_BLUE, HIGH);digitalWrite(LED_GREEN, LOW);
#define CYAN_ON  digitalWrite(LED_RED, HIGH);digitalWrite(LED_BLUE, LOW);digitalWrite(LED_GREEN, LOW);
#define YELLOW_ON digitalWrite(LED_RED, LOW);digitalWrite(LED_BLUE, HIGH);digitalWrite(LED_GREEN, LOW);

Servo servo[3];
#define MEASURING_FREQ (30)
LSM6DS3 IMU(I2C_MODE, 0x6A);
Madgwick m_;

typedef struct { float x, y, z; } pos3d_t;
pos3d_t gyr_ = {0}, acc_ = {0}, ang_ = {0};

unsigned long t = 0, t_ = 0;

float kp_pit = 2.0; float kd_pit = 3.0; float ki_pit = 0.0;
float kp_rol = 2.0; float kd_rol = 3.0; float ki_rol = 0.0;
int ang_goal = 0;
int ang_trim[] = {85, 105, 70};

float cur_servo_ang[3] = {85, 105, 70};
#define SERVO_STEP_LIMIT 3.0

float preangx = 0, preangy = 0, dx, dy, ix = 0, iy = 0, T;
int mode = 2;
bool param_changed = false;

unsigned long launch_time = 0;
bool launch_detected = false;
bool gain_boosted = false;
const float LAUNCH_THRESHOLD = -3.0;
const unsigned long BOOST_DELAY_MS = 1000;

HardwareSerial& OUT = Serial1;
const unsigned OUT_BAUD = 115200;
const unsigned DBG_BAUD = 115200;
uint32_t seq = 0;
unsigned long prev_ms = 0;

volatile bool control_ready = false;
volatile unsigned long control_t = 0;
volatile unsigned long control_t_prev = 0;
volatile uint8_t control_pending = 0;
NRF_TIMER_Type* control_timer = NRF_TIMER1;
const uint32_t CONTROL_INTERVAL_US = 1000000 / MEASURING_FREQ;

static void emitLine(const char* line) {
  Serial.println(line);
  OUT.println(line);
}

static void emitInfo(const char* tag, const char* msg) {
  char buf[160];
  snprintf(buf, sizeof(buf), "[%s] %s", tag, msg);
  emitLine(buf);
}

extern "C" {
  void TIMER1_IRQHandler(void) {
    if (control_timer->EVENTS_COMPARE[0]) {
      control_timer->EVENTS_COMPARE[0] = 0;
      control_timer->TASKS_CLEAR = 1;
      control_t = millis();
      control_ready = true;
      if (control_pending < 255) control_pending++;
    }
  }
}

void setupControlTimer() {
  control_timer->MODE = TIMER_MODE_MODE_Timer;
  control_timer->BITMODE = TIMER_BITMODE_BITMODE_32Bit;
  control_timer->PRESCALER = 4;
  control_timer->CC[0] = CONTROL_INTERVAL_US;
  control_timer->SHORTS = TIMER_SHORTS_COMPARE0_CLEAR_Enabled << TIMER_SHORTS_COMPARE0_CLEAR_Pos;
  control_timer->INTENSET = TIMER_INTENSET_COMPARE0_Enabled << TIMER_INTENSET_COMPARE0_Pos;

  NVIC_SetPriority(TIMER1_IRQn, 1);
  NVIC_EnableIRQ(TIMER1_IRQn);
  control_timer->TASKS_START = 1;
}

void sendParamLine() {
  char buf[192];
  snprintf(buf, sizeof(buf),
    "[PARAM] Pp=%.2f,Dp=%.2f,Ip=%.2f,Pr=%.2f,Dr=%.2f,Ir=%.2f,G=%d,T0=%d,T1=%d,T2=%d",
    kp_pit, kd_pit, ki_pit, kp_rol, kd_rol, ki_rol, ang_goal,
    ang_trim[0], ang_trim[1], ang_trim[2]);
  emitLine(buf);
  param_changed = false;
}

void printSaveCode() {
  Serial.println("\n// Copy these lines into your code if needed");
  Serial.print("float kp_pit = "); Serial.print(kp_pit, 2); Serial.print("; float kd_pit = "); Serial.print(kd_pit, 2); Serial.print("; float ki_pit = "); Serial.print(ki_pit, 2); Serial.println(";");
  Serial.print("float kp_rol = "); Serial.print(kp_rol, 2); Serial.print("; float kd_rol = "); Serial.print(kd_rol, 2); Serial.print("; float ki_rol = "); Serial.print(ki_rol, 2); Serial.println(";");
  Serial.print("int ang_goal = "); Serial.print(ang_goal); Serial.println(";");
  Serial.print("int ang_trim[] = {"); Serial.print(ang_trim[0]); Serial.print(", "); Serial.print(ang_trim[1]); Serial.print(", "); Serial.print(ang_trim[2]); Serial.println("};\n");
  emitInfo("INFO", "save code printed to USB serial monitor");
}

void processCommand(char c) {
  if (c == '\r' || c == '\n') return;

  if (c == 'A' || c == 'a') {
    mode = 1;
    GREEN_ON;
    ix = 0; iy = 0;
    launch_detected = false;
    gain_boosted = false;
    emitInfo("MODE", "AUTO waiting for launch");
  }
  else if (c == 'M' || c == 'm') {
    mode = 2;
    CYAN_ON;
    emitInfo("MODE", "MANUAL");
  }
  else if (c == 'p') { kp_pit -= 0.1; param_changed = true; }
  else if (c == 'P') { kp_pit += 0.1; param_changed = true; }
  else if (c == 'd') { kd_pit -= 0.1; param_changed = true; }
  else if (c == 'D') { kd_pit += 0.1; param_changed = true; }
  else if (c == 'i') { ki_pit -= 0.01; param_changed = true; }
  else if (c == 'I') { ki_pit += 0.01; param_changed = true; }
  else if (c == 'x') { kp_rol -= 0.1; param_changed = true; }
  else if (c == 'X') { kp_rol += 0.1; param_changed = true; }
  else if (c == 'y') { kd_rol -= 0.1; param_changed = true; }
  else if (c == 'Y') { kd_rol += 0.1; param_changed = true; }
  else if (c == 'z') { ki_rol -= 0.01; param_changed = true; }
  else if (c == 'Z') { ki_rol += 0.01; param_changed = true; }
  else if (c == 'g') { ang_goal -= 1; param_changed = true; }
  else if (c == 'G') { ang_goal += 1; param_changed = true; }
  else if (c == 'l') { ang_trim[0] = constrain(ang_trim[0] - 1, 15, 165); param_changed = true; }
  else if (c == 'L') { ang_trim[0] = constrain(ang_trim[0] + 1, 15, 165); param_changed = true; }
  else if (c == 'r') { ang_trim[1] = constrain(ang_trim[1] - 1, 15, 165); param_changed = true; }
  else if (c == 'R') { ang_trim[1] = constrain(ang_trim[1] + 1, 15, 165); param_changed = true; }
  else if (c == 'e') { ang_trim[2] = constrain(ang_trim[2] - 1, 15, 165); param_changed = true; }
  else if (c == 'E') { ang_trim[2] = constrain(ang_trim[2] + 1, 15, 165); param_changed = true; }
  else if (c == 'c' || c == 'C') {
    ang_trim[0] = 90; ang_trim[1] = 90; ang_trim[2] = 90;
    param_changed = true;
    emitInfo("INFO", "trim centered");
  }
  else if (c == 's' || c == 'S') {
    printSaveCode();
  }
  else if (c == 'h' || c == 'H') {
    seq = 0;
    emitInfo("INFO", "sequence reset");
    sendParamLine();
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

void executeControlLoop() {
  unsigned long t_now = control_t;
  unsigned long t_prev = control_t_prev;
  control_t_prev = t_now;

  T = (t_now - t_prev); if (T <= 0) T = 1;

  acc_.x = IMU.readFloatAccelX(); acc_.y = IMU.readFloatAccelY(); acc_.z = IMU.readFloatAccelZ();
  gyr_.x = IMU.readFloatGyroX();  gyr_.y = IMU.readFloatGyroY();  gyr_.z = IMU.readFloatGyroZ();
  m_.updateIMU(gyr_.x, gyr_.y, gyr_.z, acc_.x, acc_.y, acc_.z);
  ang_.x = m_.getRoll(); ang_.y = m_.getPitch(); ang_.z = m_.getYaw();

  if (mode == 1) {
    if (!launch_detected && acc_.x < LAUNCH_THRESHOLD) {
      launch_detected = true;
      launch_time = t_now;
      emitInfo("EVENT", "launch detected");
    } else if (!gain_boosted && launch_detected && (t_now - launch_time > BOOST_DELAY_MS)) {
      kp_rol *= 2.0; kd_rol *= 2.0; ki_rol *= 2.0;
      gain_boosted = true;
      YELLOW_ON;
      emitInfo("EVENT", "boost roll gains doubled");
      sendParamLine();
    }

    dx = (ang_.x - preangx) / T;
    dy = (ang_.y - preangy) / T;
    ix += (ang_.x + preangx) * T / 2000.0;
    iy += (ang_.y + preangy) * T / 2000.0;

    float roll_control = (ang_.x) * kp_rol - dx * kd_rol + ix * ki_rol;
    float target0 = ang_trim[0] - roll_control;
    float target1 = ang_trim[1] - roll_control;
    float target2 = ang_trim[2] + (ang_.y - ang_goal) * kp_pit + dy * kd_pit + iy * ki_pit;

    updateServoSmooth(0, target0);
    updateServoSmooth(1, target1);
    updateServoSmooth(2, target2);

    preangx = ang_.x; preangy = ang_.y;
  } else {
    updateServoSmooth(0, (float)ang_trim[0]);
    updateServoSmooth(1, (float)ang_trim[1]);
    updateServoSmooth(2, (float)ang_trim[2]);
    ix = 0; iy = 0; preangx = ang_.x; preangy = ang_.y;
  }

  t = t_now;
  t_ = t_prev;
}

void setup() {
  Serial.begin(DBG_BAUD);
  OUT.begin(OUT_BAUD);

  IMU.settings.gyroRange = 2000;
  IMU.settings.accelRange = 4;
  while (IMU.begin() != 0) { LED_OFF; delay(200); RED_ON; delay(200); }

  m_.begin((int)MEASURING_FREQ);
  servo[0].attach(0); servo[1].attach(1); servo[2].attach(2);

  cur_servo_ang[0] = ang_trim[0];
  cur_servo_ang[1] = ang_trim[1];
  cur_servo_ang[2] = ang_trim[2];

  delay(100);
  setupControlTimer();
  control_t_prev = millis();

  emitInfo("READY", "imu_control line telemetry mode");
  emitInfo("SCHEMA", "seq,t_ms,dt_ms,ax,ay,az,gx,gy,gz,roll,pitch,yaw,s0,s1,s2");
  emitInfo("MODE", "MANUAL");
  CYAN_ON;
}

void loop() {
  if (control_ready) {
    control_ready = false;
    if (control_pending > 0) control_pending--;

    executeControlLoop();

    float dt_ms = (float)(t - prev_ms);
    prev_ms = t;
    char line[160];
    snprintf(line, sizeof(line),
      "%lu,%lu,%.1f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f",
      ++seq, t, dt_ms,
      acc_.x, acc_.y, acc_.z,
      gyr_.x, gyr_.y, gyr_.z,
      ang_.x, ang_.y, ang_.z,
      cur_servo_ang[0], cur_servo_ang[1], cur_servo_ang[2]);

    if (Serial.availableForWrite() > 50) Serial.println(line);
    if (OUT.availableForWrite() > 50) OUT.println(line);

    if (control_pending > 0) {
      control_pending = 0;
      control_t_prev = control_t;
      control_t = millis();
      control_ready = true;
    }
    return;
  }

  static uint8_t cmd_count = 0;
  cmd_count++;
  if (cmd_count >= 10) {
    cmd_count = 0;
    while (Serial.available() && control_pending == 0) {
      processCommand(Serial.read());
      if (Serial.available() > 20) break;
    }
    while (OUT.available() && control_pending == 0) {
      processCommand(OUT.read());
      if (OUT.available() > 20) break;
    }
  }

  if (param_changed && control_pending == 0) {
    sendParamLine();
  }

  static unsigned long last_info_ms = 0;
  unsigned long t_now = millis();
  if ((t_now - last_info_ms > 2000) && control_pending == 0) {
    last_info_ms = t_now;
    sendParamLine();
  }

  if (control_pending == 0) delay(1);
}
