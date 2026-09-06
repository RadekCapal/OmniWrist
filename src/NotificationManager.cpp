#include "NotificationManager.h"
#include <NimBLEDevice.h>
#include <vector>

#define MAX_HISTORY 10
static std::vector<NotificationData> notificationHistory;

// Apple ANCS Service Base
static NimBLEUUID ancsServiceUUID("7905f431-b5ce-4e99-a40f-4b1e122d00d0");

// Flags and Globals
static volatile bool isConnected = false;
static volatile bool isAuthenticated = false;
static uint16_t currentConnHandle = 0;
static NimBLEServer *pGlobalServer = nullptr;
static NimBLERemoteCharacteristic *pControlPoint = nullptr;

// Buffer for text parsing
static std::vector<uint8_t> dataBuffer;
static String currentTitle = "";
static String currentMessage = "";

// --- Bezpečná schránka pro hlavní vlákno ---
static NotificationData pendingNotification;
static volatile bool isNewNotificationReady = false;

// =========================================================================
// 1. MAILBOX DELIVERY
// =========================================================================
void displayNotificationOnScreen(String title, String message) {
  pendingNotification.title = title;
  pendingNotification.message = message;
  isNewNotificationReady = true;

  notificationHistory.insert(notificationHistory.begin(), {title, message});

  if (notificationHistory.size() > MAX_HISTORY) {
    notificationHistory.pop_back();
  }
}

bool NotificationManager::hasNewNotification() {
  return isNewNotificationReady;
}

NotificationData NotificationManager::getNotification() {
  isNewNotificationReady = false;
  return pendingNotification;
}

// =========================================================================
// 2. DATA SOURCE CALLBACK
// =========================================================================
void dataSourceCallback(NimBLERemoteCharacteristic *pChar, uint8_t *pData,
                        size_t length, bool isNotify) {
  Serial.print("📥 Data Source received chunk of ");
  Serial.print(length);
  Serial.println(" bytes.");

  for (size_t i = 0; i < length; i++) {
    dataBuffer.push_back(pData[i]);
  }

  if (dataBuffer.size() >= 5) {
    uint8_t commandId = dataBuffer[0];
    if (commandId != 0) {
      dataBuffer.clear();
      return;
    }

    int index = 5;
    bool parsingComplete = true;

    currentTitle = "Unknown";
    currentMessage = "";

    // Explicit cast to int to avoid vector size comparison warnings
    while (index + 3 <= (int)dataBuffer.size()) {
      uint8_t attrID = dataBuffer[index];
      uint16_t attrLen = dataBuffer[index + 1] | (dataBuffer[index + 2] << 8);

      if (index + 3 + attrLen <= (int)dataBuffer.size()) {
        String text = "";
        for (int i = 0; i < attrLen; i++) {
          text += (char)dataBuffer[index + 3 + i];
        }

        if (attrID == 1)
          currentTitle = text;
        if (attrID == 3)
          currentMessage = text;

        index += 3 + attrLen;
      } else {
        parsingComplete = false;
        break;
      }
    }

    if (parsingComplete && index >= (int)dataBuffer.size()) {
      displayNotificationOnScreen(currentTitle, currentMessage);
      dataBuffer.clear();
    }
  }
}

// =========================================================================
// 3. MICRO-TASK TO REQUEST TEXT
// =========================================================================
void requestTextTask(void *parameter) {
  uint32_t uid = (uint32_t)(uintptr_t)parameter;

  uint8_t command[11] = {0x00,
                         (uint8_t)(uid & 0xFF),
                         (uint8_t)((uid >> 8) & 0xFF),
                         (uint8_t)((uid >> 16) & 0xFF),
                         (uint8_t)((uid >> 24) & 0xFF),
                         0x01,
                         0x40,
                         0x00,
                         0x03,
                         0xFF,
                         0x00};

  if (pControlPoint) {
    pControlPoint->writeValue(command, sizeof(command), true);
  }

  vTaskDelete(NULL);
}

// =========================================================================
// 4. NOTIFICATION SOURCE CALLBACK
// =========================================================================
void notificationCallback(NimBLERemoteCharacteristic *pChar, uint8_t *pData,
                          size_t length, bool isNotify) {
  if (length < 8)
    return;

  uint8_t eventId = pData[0];

  if (eventId == 0 && pControlPoint != nullptr) {
    Serial.println("🔔 Notification ping! Spawning task to request text...");
    uint32_t uid =
        pData[4] | (pData[5] << 8) | (pData[6] << 16) | (pData[7] << 24);
    dataBuffer.clear();
    xTaskCreate(requestTextTask, "ReqText", 2048, (void *)(uintptr_t)uid, 1,
                NULL);
  }
}

// =========================================================================
// 5. CONNECTION TASK
// =========================================================================
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
      Serial.println("✅ Service found! Hunting for all 3 Apple channels...");

      auto chars = pService->getCharacteristics(true);
      NimBLERemoteCharacteristic *pNotifSource = nullptr;
      NimBLERemoteCharacteristic *pDataSource = nullptr;
      pControlPoint = nullptr;

      for (auto pCh : chars) {
        String uuidStr = pCh->getUUID().toString().c_str();
        if (uuidStr.indexOf("9fbf120d") != -1)
          pNotifSource = pCh;
        if (uuidStr.indexOf("22eac6e9") != -1)
          pDataSource = pCh;
        if (uuidStr.indexOf("69d1d8f3") != -1)
          pControlPoint = pCh;
      }

      if (pNotifSource && pDataSource && pControlPoint) {
        Serial.println("✅ All 3 channels found! Subscribing...");
        pDataSource->subscribe(true, dataSourceCallback);
        pNotifSource->subscribe(true, notificationCallback);
        Serial.println("✅ SUCCESS! Watch is now FULLY capturing and reading "
                       "iOS notifications.");
      } else {
        Serial.println(
            "❌ Error: Missing one or more required ANCS characteristics.");
      }
    } else {
      Serial.println("❌ Error: ANCS service not found.");
    }
  } else {
    Serial.println("❌ Error: Failed to create Client interface.");
  }

  vTaskDelete(NULL);
}

// =========================================================================
// 6. BLE SERVER CALLBACKS & INIT
// =========================================================================
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
  Serial.println("Initializing clean NimBLE with ANCS FULL TEXT...");

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

int NotificationManager::getHistoryCount() {
  return notificationHistory.size();
}

NotificationData NotificationManager::getHistoryItem(int index) {
  if (index >= 0 && index < notificationHistory.size()) {
    return notificationHistory[index];
  }
  return {"", ""};
}
