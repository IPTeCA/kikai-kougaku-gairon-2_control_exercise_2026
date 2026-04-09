#if defined(SERIAL_PORT_MONITOR)
#define LESSON_SERIAL SERIAL_PORT_MONITOR
#else
#define LESSON_SERIAL Serial
#endif

void setLed(bool redOn, bool greenOn, bool blueOn) {
  digitalWrite(LED_RED, redOn ? LOW : HIGH);
  digitalWrite(LED_GREEN, greenOn ? LOW : HIGH);
  digitalWrite(LED_BLUE, blueOn ? LOW : HIGH);
}

void setup() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  LESSON_SERIAL.begin(115200);
  setLed(false, false, false);
}

void loop() {
  if (!LESSON_SERIAL.available()) {
    return;
  }

  char c = LESSON_SERIAL.read();
  LESSON_SERIAL.print("command: ");
  LESSON_SERIAL.println(c);

  if (c == 'g') {
    setLed(false, true, false);
  } else if (c == 'c') {
    setLed(false, true, true);
  } else if (c == 'm') {
    setLed(true, false, true);
  } else if (c == 'w') {
    setLed(true, true, true);
  } else if (c == 'o') {
    setLed(false, false, false);
  }
}
