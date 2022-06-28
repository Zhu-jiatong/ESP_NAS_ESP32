#if !defined(MY_DISPLAY_H)
#define MY_DISPLAY_H

#include <Adafruit_SSD1306.h>
Adafruit_SSD1306 display(128, 64, &Wire, -1);

void initOLED()
{
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C, false);
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.clearDisplay();
}

#endif // MY_DISPLAY_H
