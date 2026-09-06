#ifndef NOTIFICATION_MANAGER_H
#define NOTIFICATION_MANAGER_H

#include <Arduino.h>

// Structure to hold our parsed notification
struct NotificationData {
  String title;
  String message;
};

class NotificationManager {
public:
  static void init();
  static int getHistoryCount();
  static NotificationData getHistoryItem(int index);

  // Functions for the main loop to check and fetch new notifications safely
  static bool hasNewNotification();
  static NotificationData getNotification();
};

#endif
