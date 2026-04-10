// Seeed XIAO nRF52840 (Glider side)
// Lesson16 と同一の P/PD/PID。USB 相当のシリアルは Serial1（ESP32-C3 経由 ESP-NOW）へ置き換え。

#include <LSM6DS3.h>
#include <Wire.h>
#include <MadgwickAHRS.h>
#include <Servo.h>
#include <ctype.h>
#include <stdlib.h>

#define MEASURING_FREQ 50
#define SERVO_PIN 2

#if defined(SERIAL_PORT_MONITOR)
#define USB_SERIAL SERIAL_PORT_MONITOR
#else
#define USB_SERIAL Serial
#endif

#define RADIO_SERIAL Serial1
#define UART_BAUD 115200

// Gains (tune later)
float Kp = 1.0f;
float Ki = 0.2f;
float Kd = 0.02f;

const float targetPitchDeg = 0.0f;
const float integralLimit = 200.0f;  // anti-windup (simple clamp)

LSM6DS3 IMU(I2C_MODE, 0x6A);
Madgwick filter;
Servo servo;

enum ControlMode {
  MODE_P = 1,
  MODE_PD = 2,
  MODE_PID = 3,
};
ControlMode mode = MODE_P;
ControlMode prevMode = MODE_P;

float integralE = 0.0f;
float prevE = 0.0f;
uint32_t prevUs = 0;

static char cmdBuf[80];
static size_t cmdLen = 0;

static bool iequals(const char* a, const char* b) {
  if (!a || !b) return false;
  while (*a && *b) {
    char ca = (char)tolower((unsigned char)*a);
    char cb = (char)tolower((unsigned char)*b);
    if (ca != cb) return false;
    a++;
    b++;
  }
  return *a == '\0' && *b == '\0';
}

static char* nextToken(char*& p) {
  if (!p) return nullptr;
  while (*p && isspace((unsigned char)*p)) p++;
  if (!*p) return nullptr;
  char* tok = p;
  while (*p && !isspace((unsigned char)*p)) p++;
  if (*p) {
    *p = '\0';
    p++;
  }
  return tok;
}

static bool parseFloat(const char* s, float* out) {
  if (!s || !out) return false;
  while (*s && isspace((unsigned char)*s)) s++;
  if (!*s) return false;
  char* endp = nullptr;
  float v = strtof(s, &endp);
  if (endp == s) return false;
  while (*endp && isspace((unsigned char)*endp)) endp++;
  if (*endp != '\0') return false;
  *out = v;
  return true;
}

static void printStatus(Stream& out) {
  out.print("mode=");
  out.print((int)mode);
  out.print(",Kp=");
  out.print(Kp, 6);
  out.print(",Ki=");
  out.print(Ki, 6);
  out.print(",Kd=");
  out.println(Kd, 6);
}

static void printHelp(Stream& out) {
  out.println("Exercise05 (nRF52840): Lesson16 on Serial1 (wireless link)");
  out.println("Keys: 1=P, 2=PD, 3=PID");
  out.println("Line commands:");
  out.println("  kp <v> / ki <v> / kd <v>");
  out.println("  status");
  out.println("  help");
}

static void replyLine(Stream& out, const char* msg) {
  out.println(msg);
}

static void handleCommandLine(char* line) {
  if (!line) return;
  while (*line && isspace((unsigned char)*line)) line++;
  if (!*line) return;

  char* p = line;
  char* cmd = nextToken(p);
  if (!cmd) return;

  if (iequals(cmd, "help") || iequals(cmd, "?")) {
    printHelp(USB_SERIAL);
    printHelp(RADIO_SERIAL);
    return;
  }
  if (iequals(cmd, "status")) {
    printStatus(USB_SERIAL);
    printStatus(RADIO_SERIAL);
    return;
  }

  char* arg = nextToken(p);
  if (!arg) {
    replyLine(USB_SERIAL, "missing argument");
    replyLine(RADIO_SERIAL, "missing argument");
    return;
  }

  float v = 0.0f;
  if (iequals(cmd, "kp")) {
    if (!parseFloat(arg, &v)) goto parse_error;
    Kp = v;
    printStatus(USB_SERIAL);
    printStatus(RADIO_SERIAL);
    return;
  }
  if (iequals(cmd, "ki")) {
    if (!parseFloat(arg, &v)) goto parse_error;
    Ki = v;
    printStatus(USB_SERIAL);
    printStatus(RADIO_SERIAL);
    return;
  }
  if (iequals(cmd, "kd")) {
    if (!parseFloat(arg, &v)) goto parse_error;
    Kd = v;
    printStatus(USB_SERIAL);
    printStatus(RADIO_SERIAL);
    return;
  }

  replyLine(USB_SERIAL, "unknown command");
  replyLine(RADIO_SERIAL, "unknown command");
  return;

parse_error:
  replyLine(USB_SERIAL, "parse error");
  replyLine(RADIO_SERIAL, "parse error");
  return;
}

// Lesson16::pollSerialCommands と同じ仕様（入力元が RADIO_SERIAL）
static void pollRadioCommands() {
  while (RADIO_SERIAL.available()) {
    int c = RADIO_SERIAL.read();
    if (c < 0) break;

    if (cmdLen == 0 && (c == '1' || c == '2' || c == '3')) {
      if (c == '1') mode = MODE_P;
      if (c == '2') mode = MODE_PD;
      if (c == '3') mode = MODE_PID;
      continue;
    }

    if (c == '\r') continue;
    if (c == '\n') {
      cmdBuf[cmdLen] = '\0';
      if (cmdLen > 0) {
        handleCommandLine(cmdBuf);
      }
      cmdLen = 0;
      continue;
    }

    if (c == 0x08 || c == 0x7F) {
      if (cmdLen > 0) cmdLen--;
      continue;
    }

    if (cmdLen + 1 < sizeof(cmdBuf)) {
      cmdBuf[cmdLen++] = (char)c;
    } else {
      cmdLen = 0;
    }
  }
}

void setup() {
  USB_SERIAL.begin(UART_BAUD);
  RADIO_SERIAL.begin(UART_BAUD);

  servo.attach(SERVO_PIN);
  servo.write(90);

  IMU.settings.gyroRange = 2000;
  IMU.settings.accelRange = 4;
  while (IMU.begin() != 0) {
    delay(200);
  }

  IMU.writeRegister(LSM6DS3_ACC_GYRO_CTRL2_G, 0x8C);
  IMU.writeRegister(LSM6DS3_ACC_GYRO_CTRL1_XL, 0x8A);
  IMU.writeRegister(LSM6DS3_ACC_GYRO_CTRL7_G, 0x00);
  IMU.writeRegister(LSM6DS3_ACC_GYRO_CTRL8_XL, 0x09);
  filter.begin(MEASURING_FREQ);

  prevUs = micros();

  USB_SERIAL.println("Exercise05 (nRF52840): Lesson16 P/PD/PID");
  RADIO_SERIAL.println("Exercise05 (nRF52840): Lesson16 P/PD/PID");
  printHelp(USB_SERIAL);
  printHelp(RADIO_SERIAL);
  printStatus(USB_SERIAL);
  printStatus(RADIO_SERIAL);
}

void loop() {
  pollRadioCommands();

  uint32_t nowUs = micros();
  float dt = (nowUs - prevUs) / 1000000.0f;
  if (dt <= 0.0f) dt = 1.0f / (float)MEASURING_FREQ;
  prevUs = nowUs;

  float ax = IMU.readFloatAccelX();
  float ay = IMU.readFloatAccelY();
  float az = IMU.readFloatAccelZ();
  float gx = IMU.readFloatGyroX();
  float gy = IMU.readFloatGyroY();
  float gz = IMU.readFloatGyroZ();
  filter.updateIMU(gx, gy, gz, ax, ay, az);
  float pitchDeg = filter.getPitch();

  float e = targetPitchDeg - pitchDeg;

  if (mode != prevMode) {
    prevE = e;
    if (mode == MODE_P || mode == MODE_PD) {
      integralE = 0.0f;
    }
    prevMode = mode;
  }

  if (mode == MODE_PID) {
    integralE += e * dt;
    if (integralE > integralLimit) integralE = integralLimit;
    if (integralE < -integralLimit) integralE = -integralLimit;
  } else {
    integralE = 0.0f;
  }

  float de = 0.0f;
  if (mode == MODE_PD || mode == MODE_PID) {
    de = (e - prevE) / dt;
  }
  prevE = e;

  float u = 0.0f;
  if (mode == MODE_P) {
    u = Kp * e;
  } else if (mode == MODE_PD) {
    u = Kp * e + Kd * de;
  } else {
    u = Kp * e + Ki * integralE + Kd * de;
  }

  int servoAngle = (int)(90.0f + u);
  if (servoAngle > 180) servoAngle = 180;
  if (servoAngle < 0) servoAngle = 0;
  servo.write(servoAngle);

  // Lesson16 と同じテレメトリ（USB）。無線帯域節約のため RADIO には短い行のみ。
  USB_SERIAL.print("mode=");
  USB_SERIAL.print((int)mode);
  USB_SERIAL.print(",");
  USB_SERIAL.print("pitch=");
  USB_SERIAL.print(pitchDeg);
  USB_SERIAL.print(",e=");
  USB_SERIAL.print(e);
  USB_SERIAL.print(",I=");
  USB_SERIAL.print(integralE);
  USB_SERIAL.print(",dE=");
  USB_SERIAL.print(de);
  USB_SERIAL.print(",u=");
  USB_SERIAL.print(u);
  USB_SERIAL.print(",servo=");
  USB_SERIAL.println(servoAngle);

  RADIO_SERIAL.print("mode=");
  RADIO_SERIAL.print((int)mode);
  RADIO_SERIAL.print(",pitch=");
  RADIO_SERIAL.println(pitchDeg);

  delay(1000 / MEASURING_FREQ);
}
