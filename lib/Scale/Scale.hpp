#include <Arduino.h>
#include <SparkFun_Qwiic_Scale_NAU7802_Arduino_Library.h>

#include <Settings.hpp>

#ifndef SCALE_H
#define SCALE_H

enum ScaleInit {
  SUCCESS = 0,
  CONNECT_FAIL = -1,
  RESET_FAIL = -2,
  EXTERNAL_CLOCK_FAIL = -3,
  LDO_FAIL = -4,
  POWER_UP_FAIL = -5,
  GAIN_FAIL = -6,
  SAMPLE_RATE_FAIL = -7,
  CLK_CHP_FAIL = -8,
  CHAN2_CAP_FAIL = -9,
  CALIBRATION_FAIL = -10
};

class Scale {
  Settings settings;
  static NAU7802 loadCell;
  bool initialized = false;
  void printCalibration();
  void calibrateSlot(uint8_t slot, float weight, uint8_t samples);

 public:
  ScaleInit init(bool ldo, bool oscs);
  void calibrate();
  void calibrate(const Settings settings);
  void tare();
  bool poll();
  float getMass();
};

#endif
