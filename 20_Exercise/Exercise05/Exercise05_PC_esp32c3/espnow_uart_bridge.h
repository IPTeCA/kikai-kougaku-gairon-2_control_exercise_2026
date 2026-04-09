#pragma once

#include <Arduino.h>
#include <esp_now.h>
#include <stdint.h>

namespace espnow_uart_bridge {

// commandPort: USB Serial - local slash commands and outbound text.
// relayUart: optional second UART - each complete line is sent as-is over ESP-NOW.
void configure(const uint8_t peerMac[6], Stream& commandPort, int ledPin, Stream* relayUart = nullptr);

// When enabled (default), received lines are printed to the configured commandPort (USB).
void setEchoReceivedToCommandPort(bool enabled);

// When enabled (default: disabled), received lines are also written to relayUart (if configured).
void setRelayReceivedToRelayPort(bool enabled);

bool getOwnMacAddress(uint8_t mac[6]);
void formatMacAddress(const uint8_t mac[6], char out[18]);
void printMacAddresses();

bool sendLineFrame(const uint8_t* payload, uint16_t plen);
void printStats();
bool handleLocalCommand(const char* line);

void writeReceivedLine(const uint8_t* line, uint16_t len);
void handleReceivedLine(const uint8_t* line, uint16_t len);
void onRecv(const esp_now_recv_info_t*, const uint8_t* data, int len);

void initializeEspNow();
void processUartLoop();

}  // namespace espnow_uart_bridge

