/*
  uart_echo_demo.ino
  Simple bidirectional UART passthrough demo over ESP-NOW.
  Supports both Parent (PC side) and Child (Aircraft side) configurations.

  Tested on Seeed XIAO ESP32-C3.
  Set IS_PARENT to 1 for Parent (PC/USB), or 0 for Child (Aircraft/Serial1).
  Set peerMac to the target device MAC, or use FF:FF:FF:FF:FF:FF for broadcast.

  Local commands: /help, /mac, /stat  
*/
#include <Arduino.h>
#include "espnow_uart_bridge.h"

// ==========================================
// CONFIGURATION
// ==========================================
// 1: Parent (PC side: bridge over USB Serial)
// 0: Child (Aircraft side: bridge over Serial1 to IMU)
#define IS_PARENT 0

#define LED_PIN 2
#define UART_BAUD 115200

#if IS_PARENT
  #define UART_PORT Serial1
  #define UART_HAS_PINS 0
#else
  #define UART_PORT Serial1
  #define UART_HAS_PINS 1
  #define UART_RX_PIN 20  // D7 / GPIO20
  #define UART_TX_PIN 21  // D6 / GPIO21
#endif

// Set the peer MAC address here.
// To use broadcast, set peerMac to FF:FF:FF:FF:FF:FF.
const uint8_t peerMac[6] = { 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF };

void setup() {
  espnow_uart_bridge::configure(peerMac, UART_PORT, LED_PIN);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

#if UART_HAS_PINS
  UART_PORT.begin(UART_BAUD, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);
#else
  UART_PORT.begin(UART_BAUD);
#endif
  delay(200);

#if IS_PARENT
  UART_PORT.println("LOG,Parent Receiver (PC side): bridge from ESP-NOW to USB Serial");
#else
  // For the Child side, print logs to USB Serial (Serial) so it doesn't pollute the command port (Serial1)
  Serial.begin(115200);
  Serial.println("LOG,Child Sender (Aircraft side): bridge from Serial1 to ESP-NOW");
#endif

  espnow_uart_bridge::initializeEspNow();
}

void loop() {
  espnow_uart_bridge::processUartLoop();
  delay(1);
}

