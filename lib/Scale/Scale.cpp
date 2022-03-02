#include "Scale.hpp"

NAU7802 Scale::loadCell = NAU7802();

void waitForCharacter(char c) {
  boolean _resume = false;
  while (_resume == false) {
    if (Serial.available() > 0) {
      char inByte = Serial.read();
      if (inByte == c) {
        _resume = true;
      }
    }
  }
}

void Scale::printCalibration() {
  Serial.printf("(5 %d %.2f)\n(10 %d %.2f)\n(200 %d %.2f)\n\n",
                settings.readings[0], settings.factors[0], settings.readings[1],
                settings.factors[1], settings.readings[2], settings.factors[2]);
}

void Scale::calibrateSlot(uint8_t slot, float weight, uint8_t samples = 4) {
  int32_t reading = 0;
  float factor = 0;
  for (uint8_t i = 0; i < samples; i++) {
    int32_t avg = loadCell.getAverage(4);
    reading += avg;
    factor += (avg - loadCell.getZeroOffset()) / weight;
  }
  reading /= (float)samples;
  factor /= (float)samples;
  settings.readings[slot] = reading;
  settings.factors[slot] = factor;

  Serial.printf("Reading %fg: %d\n", weight, reading);
  Serial.printf("Factor %fg: %.4f\n", weight, factor);
}

ScaleInit Scale::init(bool ldo, bool oscs) {
  Wire.begin();

  // Init communication to sensor
  if (!loadCell.begin(Wire, false)) {
    Serial.println(F("Failed to connect to NAU7802"));
    return ScaleInit::CONNECT_FAIL;
  }

  // Reset all registers
  if (!loadCell.reset()) {
    Serial.println(F("Failed to reset"));
    return ScaleInit::RESET_FAIL;
  }

  // Use external clock source
  if (oscs && !loadCell.setBit(NAU7802_PU_CTRL_OSCS, NAU7802_PU_CTRL)) {
    Serial.println(F("Failed to set CTRL_OSCS"));
    return ScaleInit::EXTERNAL_CLOCK_FAIL;
  }

  if (!ldo) {
    // Bypass LDO for direct AVDD supply
    if (!loadCell.clearBit(NAU7802_PU_CTRL_AVDDS, NAU7802_PU_CTRL)) {
      Serial.println(F("Failed to bypass LDO"));
      return ScaleInit::LDO_FAIL;
    }
  } else {
    // Set LDO to lowest possible voltage
    if (!loadCell.setLDO(NAU7802_LDO_2V4)) {
      Serial.println(F("Failed to set LDO"));
      return ScaleInit::LDO_FAIL;
    }
  }

  // Power up sensor
  if (!loadCell.powerUp()) {
    Serial.println(F("Failed to power up"));
    return ScaleInit::POWER_UP_FAIL;
  }

  // Set highest possible gain
  if (!loadCell.setGain(NAU7802_GAIN_128)) {
    Serial.println(F("Failed to set gain"));
    return ScaleInit::GAIN_FAIL;
  }

  // Set sample rate
  if (!loadCell.setSampleRate(NAU7802_SPS_10)) {
    Serial.println(F("Failed to set sample rate"));
    return ScaleInit::SAMPLE_RATE_FAIL;
  }

  // Disable clock chopper
  if (!loadCell.setRegister(NAU7802_ADC, 0x30)) {
    Serial.println(F("Failed to disable CLK_CHP"));
    return ScaleInit::CLK_CHP_FAIL;
  }

  // Enable 330pF decoupling cap on chan 2.
  if (!loadCell.setBit(NAU7802_PGA_PWR_PGA_CAP_EN, NAU7802_PGA_PWR)) {
    Serial.println(F("Failed to set up PWR_PGA_CAP_EN"));
    return ScaleInit::CHAN2_CAP_FAIL;
  }

  // Calibrate analog front end
  if (!loadCell.calibrateAFE()) {
    Serial.println(F("Failed to calibrate system"));
    return ScaleInit::CALIBRATION_FAIL;
  }

  initialized = true;
  return ScaleInit::OK;
}

void Scale::calibrate(const Settings settings) {
  if (!initialized) {
    return;
  }

  this->settings = settings;
  printCalibration();
}

void Scale::calibrate() {
  if (!initialized) {
    return;
  }

  Serial.println(F("Send 't' to tare and continue."));

  waitForCharacter('t');
  tare();

  Serial.println(F("Place a nickel on load cell and send 'c'."));

  waitForCharacter('c');
  calibrateSlot(0, 5);

  Serial.println(F("Place 10g on load cell and send 'c'."));

  waitForCharacter('c');
  calibrateSlot(1, 10.0);

  Serial.println(F("Place 200g on load cell and send 'c'."));

  waitForCharacter('c');
  calibrateSlot(2, 200.0);
  printCalibration();

  Serial.print(F("Save these values? y/n"));

  boolean _resume = false;
  while (_resume == false) {
    if (Serial.available() > 0) {
      char inByte = Serial.read();
      if (inByte == 'y') {
        EEPROM.put(0, settings);
        if (!EEPROM.commit()) {
          Serial.println(F("Failed to write to EEPROM!"));
        }
        Serial.println(F("Calibration saved!"));
        _resume = true;
      } else if (inByte == 'n') {
        Serial.println(F("Calibration not saved"));
        _resume = true;
      }
    }
  }

  Serial.println(F("End calibration"));
}

void Scale::tare() {
  if (!initialized) {
    return;
  }

  Serial.println(F("Taring..."));
  loadCell.calculateZeroOffset(4);
}

bool Scale::poll() { return initialized && loadCell.available(); }

float Scale::getMass() {
  if (!initialized) {
    return 0;
  }

  float x0 = settings.readings[0];
  float x1 = settings.readings[1];
  float x2 = settings.readings[2];
  float y0 = settings.factors[0];
  float y1 = settings.factors[1];
  float y2 = settings.factors[2];
  int32_t x = loadCell.getAverage(8);

  // three point lagrangian interpolation
  float calFactor = ((((x - x1) * (x - x2)) / ((x0 - x1) * (x0 - x2))) * y0) +
                    ((((x - x0) * (x - x2)) / ((x1 - x0) * (x1 - x2))) * y1) +
                    ((((x - x0) * (x - x1)) / ((x2 - x0) * (x2 - x1))) * y2);
  // four point lagrangian interpolation
  // float calFactor = (y0 * (((x - x1) / (x0 - x1)) * ((x - x2) / (x0 - x2))
  // *
  //                          ((x - x3) / (x0 - x3)))) +
  //                   (y1 * (((x - x0) / (x1 - x0)) * ((x - x2) / (x1 - x2))
  //                   *
  //                          ((x - x3) / (x1 - x3)))) +
  //                   (y2 * (((x - x0) / (x2 - x0)) * ((x - x1) / (x2 - x1))
  //                   *
  //                          ((x - x3) / (x2 - x3)))) +
  //                   (y3 * (((x - x0) / (x3 - x0)) * ((x - x1) / (x3 - x1))
  //                   *
  //                          ((x - x2) / (x3 - x2))));

  float mass = (x - loadCell.getZeroOffset()) / calFactor;

  Serial.printf("Divider %.2f\n", calFactor);
  Serial.printf("Raw %d\n", loadCell.getReading());
  Serial.printf("Zero %d\n", loadCell.getZeroOffset());

  return mass;
}
