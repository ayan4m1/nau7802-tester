#include <Arduino.h>

#include "Display.hpp"
#include "Scale.hpp"
#include "Settings.hpp"

Settings settings = Settings();
Scale scale = Scale();
Display display = Display();
float lastMass = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(100);
  }
  Serial.println(F("Initializing scale..."));

  if (!scale.init()) {
    delay(5000);
    return;
  }

  Serial.println(F("Initializing settings..."));

  if (SettingsManager::init(&settings)) {
    Serial.println(F("Loaded calibration!"));
    scale.calibrate(settings);
  } else {
    // todo: make this interactive on-device instead of over serial
    scale.calibrate();
  }

  Serial.println(F("Initializing display..."));
  if (!display.init()) {
    delay(5000);
    return;
  }

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
    DisplayState state = DisplayState();

    state.mass = scale.getMass();
    state.stable =
        (max(state.mass, lastMass) - min(state.mass, lastMass)) < 0.02;
    lastMass = state.mass;
    display.update(state);
    Serial.printf("Mass: %.2f\n", state.mass);
  }
}
