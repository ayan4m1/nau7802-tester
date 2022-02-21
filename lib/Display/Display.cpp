#include "Display.hpp"

bool Display::init() {
  // SoftwareWire scaleInterface = SoftwareWire(D0, D5);
  Wire.begin();

  display = Adafruit_SSD1306(128, 32, &Wire, -1);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("Could not connect to OLED display!"));
    return false;
  }

  display.clearDisplay();
  display.setRotation(2);
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.setTextWrap(false);
  display.setTextColor(WHITE);
  display.println("All the Flavors");
  display.print("Scale v0.1");
  display.display();
  delay(3000);

  return true;
}

void Display::update(const DisplayState state) {
  display.clearDisplay();
  display.setTextSize(3);
  display.setCursor(0, 4);
  display.printf("%.2fg", state.mass);
  display.setTextSize(1);
  display.setCursor(110, 12);
  display.print(state.stable ? 'S' : 'U');
  display.display();
}
