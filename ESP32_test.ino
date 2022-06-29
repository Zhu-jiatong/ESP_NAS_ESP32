#include "libs/my_web.h"
#include "libs/my_cfg.h"
#include "libs/my_hardware.h"

void setup()
{
    my_SD.begin();
    initOLED();
    touchAttachInterrupt(
        T0, []() {}, 40);
    pinMode(connect_LED_pin, OUTPUT);
    begin_web(DNS_domain, ap_ssid, ap_psk);
}

void loop()
{
    dns.processNextRequest();
    refresh_main_screen();
}
