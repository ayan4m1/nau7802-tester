#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <SoftwareWire.h>

#include <Scale.hpp>

struct DisplayState {
  bool loading;
  ScaleInit error;
  float mass;
  bool stable;
};

class Display {
  Adafruit_SSD1306 display;

 public:
  bool init();
  void update(const DisplayState state);
};
