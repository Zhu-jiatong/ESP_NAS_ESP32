#if !defined(MY_HARDWARE_h)
#define MY_HARDWARE_h

#include <SD.h>
#include "my_cfg.h"
#include "my_web.h"

namespace cust
{
    void begin()
    {
        touchAttachInterrupt(
            T0, []() {}, 40);
        pinMode(connect_LED_pin, OUTPUT);
        digitalWrite(connect_LED_pin, LOW);
    }

    void initOLED()
    {
        display.begin(SSD1306_SWITCHCAPVCC, 0x3C, false);
        display.setTextColor(SSD1306_WHITE);
        display.setTextSize(1);
        display.clearDisplay();
    }

    void sleep_device()
    {
        display.clearDisplay();
        display.display();
        esp_sleep_enable_touchpad_wakeup();
        esp_deep_sleep_start();
    }

    uint8_t update_sleep_timer()
    {
        static unsigned long prevMillis{};
        unsigned long nowMillis(millis()),
            passMillis{nowMillis - prevMillis};

        if (WiFi.softAPgetStationNum())
            prevMillis = nowMillis;
        else if (passMillis >= wake_time)
            sleep_device();

        return milToSec((wake_time - passMillis));
    }

    void refresh_main_screen()
    {
        display.clearDisplay();
        display.setCursor(0, 0);
        display.printf("Clients: %i     ", WiFi.softAPgetStationNum());
        display.printf("%i s\n", update_sleep_timer());
        display.printf("File: %s\n", uploadFile);
        display.display();
    }
} // namespace cust

#endif // MY_HARDWARE_h
