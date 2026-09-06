#include "NotificationManager.h"
#include <NimBLEDevice.h>

// Official Apple ANCS Service UUID
static NimBLEUUID ancsServiceUUID("7905f431-b5ce-4e99-a40f-4b1e122d00d0");

// Flags for inter-task communication
static volatile bool isConnected = false;
static volatile bool isAuthenticated = false;
static uint16_t currentConnHandle = 0;
static NimBLEServer *pGlobalServer = nullptr;

// 1. Process incoming iOS notifications
void notificationCallback(NimBLERemoteCharacteristic *pChar, uint8_t *pData,
                          size_t length, bool isNotify) {
  if (length < 8)
    return;

  uint8_t eventId = pData[0];    // 0 = Added
  uint8_t categoryId = pData[2]; // App category

  if (eventId == 0) {
    Serial.println("=================================");
    Serial.print("🔔 NEW NOTIFICATION! Category: ");
    switch (categoryId) {
    case 1:
      Serial.println("📞 Incoming call!");
      break;
    case 2:
      Serial.println("📵 Missed call!");
      break;
    case 4:
      Serial.println("💬 Message (Messenger, WhatsApp, SMS)!");
      break;
    case 6:
      Serial.println("📧 E-mail!");
      break;
    default:
      Serial.println("📱 Other (Calendar, Timer, etc.)");
      break;
    }
    Serial.println("=================================");
  }
}

// 2. Independent task to handle pairing and connecting to ANCS
void ancsClientTask(void *parameter) {
  vTaskDelay(500 / portTICK_PERIOD_MS);
  if (!isConnected) {
    vTaskDelete(NULL);
    return;
  }

  Serial.println("Requesting Pairing (Look at your iPhone screen!)...");
  NimBLEDevice::startSecurity(currentConnHandle);

  int timeout = 0;
  while (!isAuthenticated && isConnected) {
    vTaskDelay(200 / portTICK_PERIOD_MS);
    timeout++;
    if (timeout > 150) {
      Serial.println("Pairing timeout! Please disconnect and try again.");
      vTaskDelete(NULL);
      return;
    }
  }

  if (!isAuthenticated || !isConnected) {
    Serial.println("Aborted. Not paired.");
    vTaskDelete(NULL);
    return;
  }

  Serial.println("Pairing confirmed! Attaching to existing connection...");
  NimBLEClient *pClient = pGlobalServer->getClient(currentConnHandle);

  if (pClient != nullptr) {
    Serial.println(
        "Waiting 2 seconds for iOS to expose ANCS characteristics...");
    vTaskDelay(2000 / portTICK_PERIOD_MS);

    Serial.println("Searching for ANCS service on iPhone...");
    NimBLERemoteService *pService = pClient->getService(ancsServiceUUID);

    if (pService != nullptr) {
      Serial.println("✅ Service found! Hunting for Notification Channel...");

      auto chars = pService->getCharacteristics(true);
      NimBLERemoteCharacteristic *pTargetChar = nullptr;

      // SMART SEARCH: Find the characteristic that starts with Apple's base
      // UUID
      for (auto pCh : chars) {
        String uuidStr = pCh->getUUID().toString().c_str();
        if (uuidStr.indexOf("9fbf120d") != -1) {
          pTargetChar = pCh;
          break;
        }
      }

      if (pTargetChar != nullptr) {
        Serial.println("✅ Characteristic matched! Subscribing...");

        if (pTargetChar->canNotify()) {
          if (pTargetChar->subscribe(true, notificationCallback)) {
            Serial.println(
                "✅ SUCCESS! Watch is now capturing iOS notifications.");
          } else {
            Serial.println(
                "❌ Error: Subscribe function failed (iOS rejected).");
          }
        } else {
          Serial.println("❌ Error: Characteristic exists but cannot notify.");
        }
      } else {
        Serial.println(
            "❌ Error: Notification Characteristic not found dynamically.");
      }
    } else {
      Serial.println("❌ Error: ANCS service not found.");
    }
  } else {
    Serial.println("❌ Error: Failed to create Client interface.");
  }

  vTaskDelete(NULL);
}

// 3. Callbacks to handle connection state
class ServerCallbacks : public NimBLEServerCallbacks {
  void onConnect(NimBLEServer *pServer, NimBLEConnInfo &connInfo) override {
    Serial.println("\n✅ iPhone connected!");
    isConnected = true;
    isAuthenticated = false;
    currentConnHandle = connInfo.getConnHandle();

    xTaskCreate(ancsClientTask, "ANCS_Task", 4096, NULL, 1, NULL);
  }

  void onDisconnect(NimBLEServer *pServer, NimBLEConnInfo &connInfo,
                    int reason) override {
    Serial.println("\n❌ iPhone disconnected. Restarting advertising...");
    isConnected = false;
    isAuthenticated = false;
    NimBLEDevice::startAdvertising();
  }

  void onAuthenticationComplete(NimBLEConnInfo &connInfo) override {
    Serial.println("\n🔒 Encryption successful!");
    isAuthenticated = true;
  }
};

void NotificationManager::init() {
  Serial.println("Initializing clean NimBLE with ANCS...");

  NimBLEDevice::init("OmniWrist");
  NimBLEDevice::setMTU(512);
  NimBLEDevice::setSecurityAuth(true, true, true);
  NimBLEDevice::setSecurityIOCap(BLE_HS_IO_NO_INPUT_OUTPUT);

  pGlobalServer = NimBLEDevice::createServer();
  pGlobalServer->setCallbacks(new ServerCallbacks());

  NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
  pAdvertising->setName("OmniWrist");
  pAdvertising->start();

  Serial.println("BLE started! Go to LightBlue and connect.");
}
