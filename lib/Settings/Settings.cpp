#include "Settings.hpp"

bool SettingsManager::init(Settings *settings) {
  if (!SPIFFS.begin()) {
    return false;
  }

  if (!SPIFFS.exists(SETTINGS_PATH)) {
    return false;
  }

  auto file = SPIFFS.open(SETTINGS_PATH, MODE_READ);

  if (!file) {
    return false;
  }

  file.read((uint8_t *)&settings, sizeof(settings));
  file.close();

  return settings->readings[0] != 0;
}

bool SettingsManager::commit(const Settings settings) {
  auto file = SPIFFS.open(SETTINGS_PATH, MODE_WRITE);

  if (!file) {
    return false;
  }

  auto written = file.write((uint8_t *)&settings, sizeof(settings));

  file.close();

  return written > 0;
}
