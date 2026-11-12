#ifndef WIFI_LINK_H
#define WIFI_LINK_H

#include <stdint.h>

/* Joins the hotspot and opens a TCP connection to the laptop.
 * Returns 0 on success, negative on failure (reason printed over UART). */
int WifiLink_Init(void);

/* Sends a string to the laptop. Returns 0 on success, -1 on failure.
 * If the connection dropped, it retries at most once every 5 s
 * so a missing laptop doesn't freeze the main loop. */
int WifiLink_Send(const char *msg);

/* Waits up to timeout_ms for data from the laptop (blocks for that long).
 * Returns number of bytes received; buf is always null-terminated. */
int WifiLink_Receive(char *buf, uint16_t maxlen, uint32_t timeout_ms);

int WifiLink_IsConnected(void);

/* Prints to UART, and also to the laptop if WiFi is connected. */
void Comms_Print(const char *msg);

#endif
