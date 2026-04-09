// 対象ボード: Seeed XIAO nRF52840

void setup() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);

  digitalWrite(LED_RED, HIGH);
  digitalWrite(LED_GREEN, HIGH);
  digitalWrite(LED_BLUE, HIGH);
}

void loop() {
  digitalWrite(LED_RED, LOW);
  delay(500);
  digitalWrite(LED_RED, HIGH);
  delay(500);
}
