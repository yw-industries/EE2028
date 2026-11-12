/* Template: copy this file to wifi_secrets.h (same folder) and fill in
 * your own values. wifi_secrets.h is listed in .gitignore, so your
 * password never gets pushed to GitHub. */
#ifndef WIFI_SECRETS_H
#define WIFI_SECRETS_H

#define BOARD_ID "Y"     /* "B" or "A" on the other board */
#define WIFI_SSID      "YourHotspotName"
#define WIFI_PASSWORD  "YourHotspotPassword"
#define LAPTOP_IP      {192, 168, 1, 100}   /* laptop's IP on the hotspot */
#define LAPTOP_PORT    5000                 /* must match PORT in server.py */

#endif
