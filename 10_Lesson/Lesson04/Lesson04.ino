// 対象ボード: Seeed XIAO nRF52840

#define LED_OFF digitalWrite(LED_RED, HIGH); digitalWrite(LED_GREEN, HIGH); digitalWrite(LED_BLUE, HIGH)
#define RED_ON digitalWrite(LED_RED, LOW); digitalWrite(LED_GREEN, HIGH); digitalWrite(LED_BLUE, HIGH)
#define GREEN_ON digitalWrite(LED_RED, HIGH); digitalWrite(LED_GREEN, LOW); digitalWrite(LED_BLUE, HIGH)
#define BLUE_ON digitalWrite(LED_RED, HIGH); digitalWrite(LED_GREEN, HIGH); digitalWrite(LED_BLUE, LOW)

void setup() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);

  Serial.begin(115200);
  LED_OFF;
}

void loop() {
  if (Serial.available()) {
    char c = Serial.read();

    if (c == 'r') {
      RED_ON;
      Serial.println("RED");
    } else if (c == 'g') {
      GREEN_ON;
      Serial.println("GREEN");
    } else if (c == 'b') {
      BLUE_ON;
      Serial.println("BLUE");
    } else if (c == '0') {
      LED_OFF;
      Serial.println("OFF");
    }
  }
}
