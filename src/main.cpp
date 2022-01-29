#include <Adafruit_NAU7802.h>
#include <Arduino.h>
#include <Smoothed.h>

int32_t lastReading = 0;
uint32_t offset = 0;
float divider = 0;
Adafruit_NAU7802 loadCell;
static bool initialized = false;

void calibrate() {
  int32_t newOffset = 0;
  Serial.println(F("Send 't' to tare and continue."));

  boolean _resume = false;
  while (_resume == false) {
    if (Serial.available() > 0) {
      char inByte = Serial.read();
      if (inByte == 't') {
        newOffset = loadCell.read();
        _resume = true;
      }
    }
  }

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

  Smoothed<float> tenGrams;
  tenGrams.begin(SMOOTHED_AVERAGE, 10);
  for (uint8_t i = 0; i < 10; i++) {
    tenGrams.add(loadCell.read() / 10.0);
  }

  Serial.println(F("Place 200g on load cell and send 'c'."));

  _resume = false;
  while (_resume == false) {
    if (Serial.available() > 0) {
      char inByte = Serial.read();
      if (inByte == 'c') {
        _resume = true;
      }
    }
  }

  Smoothed<float> fullWeight;
  fullWeight.begin(SMOOTHED_AVERAGE, 10);
  for (uint8_t i = 0; i < 10; i++) {
    fullWeight.add(loadCell.read() / 200.0);
  }

  float newDivider = (tenGrams.get() + fullWeight.get()) / 2.0;

  Serial.printf("New divider is %f\n", newDivider);
  Serial.printf("New tare offset is %d\n", newOffset);
  Serial.print(F("Save this value? y/n"));

  _resume = false;
  while (_resume == false) {
    if (Serial.available() > 0) {
      char inByte = Serial.read();
      if (inByte == 'y') {
        divider = newDivider;
        offset = newOffset;
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
  Serial.begin(115200);

  while (!Serial) {
    delay(100);
  }

  Serial.println(F("Connected!"));

  if (!loadCell.begin()) {
    Serial.println(F("Failed to connect to NAU7802"));
    return;
  }

  loadCell.setLDO(NAU7802_EXTERNAL);
  loadCell.setGain(NAU7802_GAIN_1);
  loadCell.setRate(NAU7802_RATE_10SPS);

  for (uint8_t i = 0; i < 10; i++) {
    while (!loadCell.available()) {
      delayMicroseconds(100);
    }

    loadCell.read();
  }

  while (!loadCell.calibrate(NAU7802_CALMOD_INTERNAL)) {
    Serial.println(F("Failed to calibrate system, retrying..."));
    delay(100);
  }

  while (!loadCell.calibrate(NAU7802_CALMOD_OFFSET)) {
    Serial.println(F("Failed to calibrate offset, retrying..."));
    delay(100);
  }

  initialized = true;
}

void loop() {
  if (!initialized) {
    Serial.println(F("Init failed, need to restart!"));
    delay(1000);
    return;
  }

  if (Serial.available()) {
    char typedChar = Serial.read();
    if (typedChar == 'c') {
      calibrate();
    }
  } else if (loadCell.available()) {
    int32_t raw = loadCell.read();
    Serial.printf("Read value %d\n", raw);
  }
}
