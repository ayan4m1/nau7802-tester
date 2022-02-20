#include <Arduino.h>
#include <SparkFun_Qwiic_Scale_NAU7802_Arduino_Library.h>

#include <Settings.hpp>

#ifndef SCALE_H
#define SCALE_H

class Scale {
  Settings *settings;
  static NAU7802 loadCell;
  bool initialized = false;
  void printCalibration();
  void calibrateSlot(uint8_t slot, float weight, uint8_t samples);

 public:
  Scale(Settings *settings);
  bool init();
  void calibrate();
  void tare();
  bool poll();
  float getMass();
};

#endif
