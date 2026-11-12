#include "main.h"
#include "wifi_link.h"
#include "wifi.h"          /* from ST's WiFi/Common folder */
#include "uart.h"
#include <string.h>
#include <stdio.h>

/* WIFI_SSID, WIFI_PASSWORD, LAPTOP_IP and LAPTOP_PORT live in wifi_secrets.h,
 * which is NOT committed to Git. Copy wifi_secrets_example.h to
 * wifi_secrets.h and fill in your own values. */
#include "wifi_secrets.h"

static uint8_t laptop_ip[4] = LAPTOP_IP;

#define SOCKET_ID          0
#define RETRY_INTERVAL_MS  5000

static int connected = 0;
static uint32_t last_retry = 0;

static int open_socket(void)
{
    if (WIFI_OpenClientConnection(SOCKET_ID, WIFI_TCP_PROTOCOL, "laptop",
                                  laptop_ip, LAPTOP_PORT, 0) != WIFI_STATUS_OK)
    {
        connected = 0;
        return -1;
    }
    connected = 1;
    return 0;
}

int WifiLink_Init(void)
{
    char msg[80];

    if (WIFI_Init() != WIFI_STATUS_OK) {
        UART_Print("WiFi: module init failed\r\n");
        return -1;
    }
    UART_Print("WiFi: module OK, joining " WIFI_SSID "...\r\n");

    if (WIFI_Connect(WIFI_SSID, WIFI_PASSWORD, WIFI_ECN_WPA2_PSK) != WIFI_STATUS_OK) {
        UART_Print("WiFi: failed to join hotspot (name/password? 2.4 GHz?)\r\n");
        return -2;
    }
    UART_Print("WiFi: joined hotspot\r\n");

    sprintf(msg, "WiFi: connecting to %d.%d.%d.%d:%d\r\n",
            laptop_ip[0], laptop_ip[1], laptop_ip[2], laptop_ip[3], LAPTOP_PORT);
    UART_Print(msg);

    last_retry = HAL_GetTick();
    if (open_socket() != 0) {
        UART_Print("WiFi: TCP connect failed (server running? IP right? firewall?)\r\n");
        return -3;
    }
    UART_Print("WiFi: TCP connected\r\n");
    return 0;
}

int WifiLink_Send(const char *msg)
{
    uint16_t len = (uint16_t)strlen(msg);
    uint16_t sent = 0;

    if (!connected) {
        if (HAL_GetTick() - last_retry < RETRY_INTERVAL_MS) return -1;
        last_retry = HAL_GetTick();
        if (open_socket() != 0) return -1;
        UART_Print("WiFi: reconnected\r\n");
    }

    if (WIFI_SendData(SOCKET_ID, (uint8_t *)msg, len, &sent, 1000) != WIFI_STATUS_OK
        || sent != len)
    {
        WIFI_CloseClientConnection(SOCKET_ID);
        connected = 0;
        UART_Print("WiFi: send failed, connection dropped\r\n");
        return -1;
    }
    return 0;
}

int WifiLink_Receive(char *buf, uint16_t maxlen, uint32_t timeout_ms)
{
    uint16_t got = 0;

    if (maxlen < 2) return 0;
    buf[0] = '\0';
    if (!connected) return 0;

    if (WIFI_ReceiveData(SOCKET_ID, (uint8_t *)buf, maxlen - 1, &got, timeout_ms)
        != WIFI_STATUS_OK)
    {
        return 0;
    }
    buf[got] = '\0';
    return got;
}

int WifiLink_IsConnected(void)
{
    return connected;
}


void Comms_Print(const char *msg)
{
    UART_Print((char *)msg);          /* cast only needed while uart.h takes char* */

    if (WifiLink_IsConnected())
    {
        WifiLink_Send(msg);
    }
}
