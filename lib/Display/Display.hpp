// #include <Adafruit_SSD1306.h>
#include <Arduino.h>

#define ESP_PLATFORM

#include <Scale.hpp>
#include <gfx_cpp14.hpp>
#include <ssd1306.hpp>

using namespace arduino;
using namespace gfx;

#define OLED_PORT 0U
#define OLED_PIN_SDA D1
#define OLED_PIN_SCL D2
#define OLED_WIDTH 128
#define OLED_HEIGHT 32
#define OLED_ADDRESS 0x3C

using bus_type = tft_i2c_ex<OLED_PORT>;

using lcd_type = ssd1306<OLED_WIDTH, OLED_HEIGHT, bus_type, OLED_ADDRESS, true,
                         200, -1, -1, true>;
using lcd_color = color<typename lcd_type::pixel_type>;

// using bmp_type = bitmap<rgb_pixel<16>>;
// using bmp_color = color<typename bmp_type::pixel_type>;

struct DisplayState {
  bool loading;
  ScaleInit error;
  float mass;
  bool stable;
};

class Display {
  // Adafruit_SSD1306 display;
  lcd_type display;

 public:
  bool init();
  void update(const DisplayState state);
};
