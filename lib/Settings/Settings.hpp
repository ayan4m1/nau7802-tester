#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>
#include <SPIFFS.h>

#define SETTINGS_PATH "/settings"
#define MODE_READ "r"
#define MODE_WRITE "w+"

struct Settings {
  int32_t readings[3];
  float factors[3];
};

class SettingsManager {
 public:
  static bool init(Settings *settings);
  static bool commit(const Settings settings);
};

#endif
