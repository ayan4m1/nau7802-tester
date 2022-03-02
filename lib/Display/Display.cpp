#include "Display.hpp"

bool Display::init() {
  Wire.begin();

  display = Adafruit_SSD1306(128, 32, &Wire, -1);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("Could not connect to OLED display!"));
    return false;
  }

  display.setRotation(2);
  display.setTextWrap(false);
  display.setTextColor(WHITE);

  return true;
}

void Display::update(const DisplayState state) {
  display.clearDisplay();

  if (state.loading) {
    if (state.error) {
      display.setCursor(0, 4);
      display.setTextSize(3);

      switch (state.error) {
        case CONNECT_FAIL:
          display.print(F("ERR_CONNECT"));
          break;
        case RESET_FAIL:
          display.print(F("ERR_RESET"));
          break;
        case EXTERNAL_CLOCK_FAIL:
          display.print(F("ERR_EX_CLK"));
          break;
        case LDO_FAIL:
          display.print(F("ERR_LDO"));
          break;
        case POWER_UP_FAIL:
          display.print(F("ERR_POWER"));
          break;
        case GAIN_FAIL:
          display.print(F("ERR_GAIN"));
          break;
        case SAMPLE_RATE_FAIL:
          display.print(F("ERR_SR"));
          break;
        case CLK_CHP_FAIL:
          display.print(F("ERR_CLK_CHP"));
          break;
        case CHAN2_CAP_FAIL:
          display.print(F("ERR_C2_CAP"));
          break;
        case CALIBRATION_FAIL:
          display.print(F("ERR_CAL"));
          break;
        default:
          display.print(F("ERR_UNK"));
          break;
      }
    } else {
      display.setCursor(0, 0);
      display.setTextSize(1);
      display.println("All the Flavors");
      display.print("Scale v0.1");
    }
  } else {
    char mass[12];
    sprintf(mass, "%.2fg", state.mass);
    int16_t *x1, *y1;
    uint16_t *w, *h;

    display.setTextSize(3);
    display.getTextBounds(mass, 0, 4, x1, y1, w, h);

    if (*w >= 110) {
      display.setTextSize(2);
    }

    display.print(mass);

    display.setCursor(0, 4);
    display.setTextSize(1);
    display.setCursor(110, 12);
    display.print(state.stable ? 'S' : 'U');
  }

  display.display();
}
