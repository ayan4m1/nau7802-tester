#include <Arduino.h>
#include <ESP_EEPROM.h>
#include <SparkFun_Qwiic_Scale_NAU7802_Arduino_Library.h>

struct Settings {
  int32 readings[3];
  float factors[3];
} settings;

NAU7802 loadCell;
static bool initialized = false;

void calibrate() {
  Serial.println(F("Send 't' to tare and continue."));

  boolean _resume = false;
  while (_resume == false) {
    if (Serial.available() > 0) {
      char inByte = Serial.read();
      if (inByte == 't') {
        loadCell.calculateZeroOffset();
        _resume = true;
      }
    }
  }

  Serial.println(F("Place a dime on load cell and send 'c'."));

  _resume = false;
  while (_resume == false) {
    if (Serial.available() > 0) {
      char inByte = Serial.read();
      if (inByte == 'c') {
        _resume = true;
      }
    }
  }

  float dimeCal = 0;
  for (uint8_t i = 0; i < 5; i++) {
    loadCell.calculateCalibrationFactor(2.268);
    dimeCal += loadCell.getCalibrationFactor();
  }
  settings.readings[0] = loadCell.getAverage(8);
  settings.factors[0] = dimeCal / 5.0;

  Serial.printf("Raw 2.268g: %d\n", settings.readings[0]);
  Serial.printf("Cal 2.268g: %.4f\n", settings.factors[0]);

  Serial.println(F("Place a nickel on load cell and send 'c'."));

  _resume = false;
  while (_resume == false) {
    if (Serial.available() > 0) {
      char inByte = Serial.read();
      if (inByte == 'c') {
        _resume = true;
      }
    }
  }

  float nickelCal = 0;
  for (uint8_t i = 0; i < 5; i++) {
    loadCell.calculateCalibrationFactor(5.0);
    nickelCal += loadCell.getCalibrationFactor();
  }
  settings.readings[1] = loadCell.getAverage(8);
  settings.factors[1] = nickelCal / 5.0;

  Serial.printf("Raw 5g: %d\n", settings.readings[1]);
  Serial.printf("Cal 5g: %.4f\n", settings.factors[1]);

  Serial.println(F("Place 10g on load cell and send 'c'."));

  _resume = false;
  while (_resume == false) {
    if (Serial.available() > 0) {
      char inByte = Serial.read();
      if (inByte == 'c') {
        _resume = true;
      }
    }
  }

  float tenGramCal = 0;
  for (uint8_t i = 0; i < 5; i++) {
    loadCell.calculateCalibrationFactor(10.0);
    tenGramCal += loadCell.getCalibrationFactor();
  }
  settings.readings[2] = loadCell.getAverage(8);
  settings.factors[2] = tenGramCal / 5.0;

  Serial.printf("Raw 10g: %d\n", settings.readings[2]);
  Serial.printf("Cal 10g: %.4f\n", settings.factors[2]);

  Serial.printf("(%d %.2f) (%d %.2f) (%d %.2f)", settings.readings[0],
                settings.factors[0], settings.readings[1], settings.factors[1],
                settings.readings[2], settings.factors[2]);
  Serial.print(F("Save these values? y/n"));

  _resume = false;
  while (_resume == false) {
    if (Serial.available() > 0) {
      char inByte = Serial.read();
      if (inByte == 'y') {
        EEPROM.put(0, settings);
        if (!EEPROM.commit()) {
          Serial.println(F("Failed to write to EEPROM!"));
        }
        // loadCell.setCalibrationFactor(calFactor);
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

void setup() {
  EEPROM.begin(sizeof(Settings));

  Serial.begin(115200);

  while (!Serial) {
    delay(100);
  }

  Serial.println(F("Connected!"));

  Wire.begin();
  Wire.setClock(4000000);

  // Init communication to sensor
  if (!loadCell.begin(Wire, false)) {
    Serial.println(F("Failed to connect to NAU7802"));
    return;
  }

  // Reset all registers
  if (!loadCell.reset()) {
    Serial.println(F("Failed to reset"));
    return;
  }

  // Use external clock source
  // if (!loadCell.setBit(NAU7802_PU_CTRL_OSCS, NAU7802_PU_CTRL)) {
  //   Serial.println(F("Failed to set CTRL_OSCS"));
  //   return;
  // }

  // Bypass LDO for direct AVDD supply
  // if (!loadCell.clearBit(NAU7802_PU_CTRL_AVDDS, NAU7802_PU_CTRL)) {
  //   Serial.println(F("Failed to bypass LDO"));
  //   return;
  // }

  // Set LDO to highest possible voltage
  if (!loadCell.setLDO(NAU7802_LDO_4V5)) {
    Serial.println(F("Failed to set LDO"));
    return;
  }

  // Power up sensor
  if (!loadCell.powerUp()) {
    Serial.println(F("Failed to power up"));
    return;
  }

  // Set gain
  if (!loadCell.setGain(NAU7802_GAIN_32)) {
    Serial.println(F("Failed to set gain"));
    return;
  }

  // Set sample rate
  if (!loadCell.setSampleRate(NAU7802_SPS_10)) {
    Serial.println(F("Failed to set sample rate"));
    return;
  }

  // Disable clock chopper
  if (!loadCell.setRegister(NAU7802_ADC, 0x30)) {
    Serial.println(F("Failed to disable CLK_CHP"));
    return;
  }

  // Enable 330pF decoupling cap on chan 2.
  if (!loadCell.setBit(NAU7802_PGA_PWR_PGA_CAP_EN, NAU7802_PGA_PWR)) {
    Serial.println(F("Failed to set up PWR_PGA_CAP_EN"));
    return;
  }

  // Calibrate analog front end
  while (!loadCell.calibrateAFE()) {
    Serial.println(F("Failed to calibrate system, retrying..."));
    delay(100);
  }

  Serial.println(F("Setting zero offset..."));

  // Set up scale parameters
  loadCell.calculateZeroOffset();

  EEPROM.get(0, settings);
  if (settings.readings[0] != 0) {
    // loadCell.setCalibrationFactor(calFactor);
    Serial.println(F("Loaded calibration!"));
    Serial.printf("(%d %.2f) (%d %.2f) (%d %.2f)", settings.readings[0],
                  settings.factors[0], settings.readings[1],
                  settings.factors[1], settings.readings[2],
                  settings.factors[2]);
  } else {
    calibrate();
  }

  initialized = true;
  Serial.println(F("Initialized!"));
}

void loop() {
  // Bail if init failed
  if (!initialized) {
    Serial.println(F("Init failed, need to restart!"));
    delay(1000);
    return;
  }

  // Check for calibration request
  if (Serial.available()) {
    char typedChar = Serial.read();
    if (typedChar == 'c') {
      calibrate();
    } else if (typedChar == 't') {
      Serial.println(F("Taring..."));
      loadCell.calculateZeroOffset();
    }
  } else if (loadCell.available()) {
    float x0 = settings.readings[0];
    float x1 = settings.readings[1];
    float x2 = settings.readings[2];
    float y0 = settings.factors[0];
    float y1 = settings.factors[1];
    float y2 = settings.factors[2];
    float x = loadCell.getAverage(8);
    float calFactor = ((((x - x1) * (x - x2)) / ((x0 - x1) * (x0 - x2))) * y0) +
                      ((((x - x0) * (x - x2)) / ((x1 - x0) * (x1 - x2))) * y1) +
                      ((((x - x0) * (x - x1)) / ((x2 - x0) * (x2 - x1))) * y2);

    Serial.printf("Cal factor %.2f\n", calFactor);
    float mass = (x - loadCell.getZeroOffset()) / calFactor;

    Serial.printf("Raw value %d\n", x);
    Serial.printf("Scaled value %.2f\n", mass);
  }
}
