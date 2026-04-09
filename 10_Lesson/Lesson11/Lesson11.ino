// 対象ボード: Seeed XIAO ESP32-C3

#include <Arduino.h>
#include "espnow_uart_bridge.h"

#define LED_PIN 2
#define UART_BAUD 115200
#define UART_PORT Serial

const uint8_t peerMac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

void setup() {
  espnow_uart_bridge::configure(peerMac, UART_PORT, LED_PIN);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  UART_PORT.begin(UART_BAUD);
  delay(200);

  espnow_uart_bridge::initializeEspNow();
  UART_PORT.println("Lesson11: espnow-uart-passthrough setup");
  UART_PORT.println("Type /mac to check your MAC address.");
  UART_PORT.println("Type /help to show available local commands.");
}

void loop() {
  espnow_uart_bridge::processUartLoop();
  delay(1);
}
