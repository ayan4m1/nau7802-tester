#include <Arduino.h>

#include "Display.hpp"
#include "Scale.hpp"
#include "Settings.hpp"

Settings settings = Settings();
Scale scale = Scale();
Display display = Display();
DisplayState state = DisplayState();

void setup() {
  Serial.begin(115200);

  Serial.println(F("Initializing display..."));
  if (!display.init()) {
    delay(5000);
    return;
  }

  state.loading = true;
  display.update(state);

  Serial.println(F("Initializing scale..."));
  auto scaleInit = scale.init(true, false);
  if (scaleInit != ScaleInit::SUCCESS) {
    state.error = scaleInit;
    display.update(state);
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

  scale.tare();
  state.loading = false;
  display.update(state);
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
    float newMass = scale.getMass();
    state.stable = (max(state.mass, newMass) - min(state.mass, newMass)) < 0.02;
    state.mass = newMass;
    display.update(state);
    Serial.printf("Mass: %.2f\n", state.mass);
  }
}
