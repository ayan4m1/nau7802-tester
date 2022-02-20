#include <Arduino.h>

#include "Scale.hpp"
#include "Settings.hpp"

Settings settings = Settings();
Scale scale = Scale(&settings);

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(100);
  }
  Serial.println(F("Connected!"));

  if (!scale.init()) {
    delay(5000);
    return;
  }

  Serial.println(F("Scale init!"));

  if (SettingsManager::init(&settings)) {
    Serial.println(F("Loaded calibration!"));
  } else {
    scale.calibrate();
  }

  Serial.println(F("Taring..."));
  scale.tare();

  Serial.println(F("Initialized!"));
}

void loop() {
  if (Serial.available()) {
    char typedChar = Serial.read();
    if (typedChar == 'c') {
      scale.calibrate();
    } else if (typedChar == 't') {
      scale.tare();
    }
  } else if (scale.poll()) {
    float mass = scale.getMass();
    Serial.printf("Mass: %.2f\n", mass);
  }
}
