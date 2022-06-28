#if !defined(MY_HARDWARE_h)
#define MY_HARDWARE_h

#include "my_display.h"
#include "my_SD.h"
#include "my_cfg.h"
#include "my_web.h"

void sleep_device()
{
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.print("\n Sleeping\n ...");
    display.display();
    esp_sleep_enable_touchpad_wakeup();
    esp_deep_sleep_start();
}

uint8_t update_sleep_timer()
{
    static ulong prevMillis{}, leftMillis{};
    ulong nowMillis(millis()), passMillis{nowMillis - prevMillis};
    leftMillis = wake_time - passMillis;
    if (!WiFi.softAPgetStationNum())
    {
        if (passMillis >= wake_time)
            sleep_device();
    }
    else
        prevMillis = nowMillis;
    return milToSec(leftMillis);
}

void refresh_main_screen()
{
    display.clearDisplay();
    display.setCursor(0, 0);
    display.printf("AP_Client: %i\n", WiFi.softAPgetStationNum());
    display.printf("Size: %s\n", humanReadableSize(SD.cardSize()));
    display.println("CPU temp: " + String(temperatureRead(), 2) + " C");
    display.printf("Upload: %s\n", (isUpload ? "Uploading..." : "Idle"));
    display.printf("To sleep: %i sec", update_sleep_timer());
    display.display();
}
#endif // MY_HARDWARE_h
