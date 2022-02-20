#include "Settings.hpp"

bool SettingsManager::init(Settings *settings) {
  EEPROM.begin(sizeof(Settings));

  Settings result;
  EEPROM.get(0, result);

  for (uint8_t i = 0; i < 3; i++) {
    settings->factors[i] = result.factors[i];
    settings->readings[i] = result.readings[i];
  }

  return settings->readings[0] != 0;
}

bool SettingsManager::commit(const Settings settings) {
  EEPROM.put(0, settings);
  return EEPROM.commit();
}
