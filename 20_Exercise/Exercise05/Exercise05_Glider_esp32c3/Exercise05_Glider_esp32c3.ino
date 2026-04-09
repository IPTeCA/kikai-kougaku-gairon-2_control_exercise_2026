// Seeed XIAO ESP32-C3 (Glider side)
// Receives lines over ESP-NOW and forwards them to nRF52840 over Serial1.
// Also forwards lines from Serial1 back over ESP-NOW (telemetry / status / acknowledgements).

#include <Arduino.h>

#include "espnow_uart_bridge.h"

#define LED_PIN 2
#define UART_BAUD 115200
#define CMD_PORT Serial
#define RELAY_PORT Serial1

#define D7 20
#define D6 21
#define UART_RX_PIN D7
#define UART_TX_PIN D6

// Set to the PC-side ESP32-C3 STA MAC.
const uint8_t peerMac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  CMD_PORT.begin(UART_BAUD);
  RELAY_PORT.begin(UART_BAUD, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);
  delay(200);

  espnow_uart_bridge::configure(peerMac, CMD_PORT, LED_PIN, &RELAY_PORT);
  espnow_uart_bridge::setRelayReceivedToRelayPort(true);  // ESP-NOW -> Serial1
  espnow_uart_bridge::initializeEspNow();

  CMD_PORT.println("Exercise05 (Glider/ESP32-C3): ESP-NOW <-> Serial1 bridge (Lesson16-style P/PD/PID on nRF)");
  CMD_PORT.println("Serial1: connected to nRF52840 (Exercise05_Glider_nrf52840)");
}

void loop() {
  espnow_uart_bridge::processUartLoop();
  delay(1);
}
