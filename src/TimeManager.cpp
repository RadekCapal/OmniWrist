#include "TimeManager.h"
#include <WiFi.h>
#include <time.h>

void TimeManager::syncTime(const char *ssid, const char *password) {
  Serial.print("[TIME] connecting to wifi...");
  WiFi.begin(ssid, password);

  // wait for connection (timeout after 10s)
  uint32_t timeout = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - timeout < 10000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[TIME] wifi connected. syncing ntp...");

    // config ntp server and central european timezone (prague)
    configTzTime("CET-1CEST,M3.5.0,M10.5.0/3", "pool.ntp.org", "time.nist.gov");

    // wait until time is actually synced (year will be > 1970)
    struct tm timeinfo;
    while (!getLocalTime(&timeinfo)) {
      delay(100);
      Serial.print(".");
    }

    Serial.println("\n[TIME] sync successful!");
    Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  } else {
    Serial.println("\n[TIME] wifi connection failed. rtc not synced.");
  }

  // turn off wifi to save battery
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  Serial.println("[TIME] wifi disabled.");
}
