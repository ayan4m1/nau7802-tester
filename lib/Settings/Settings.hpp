#include <Arduino.h>
#include <ESP_EEPROM.h>

#ifndef SETTINGS_H
#define SETTINGS_H

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
