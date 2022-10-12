#include "libs/my_web.h"
#include "libs/my_cfg.h"
#include "libs/my_hardware.h"

void setup()
{
    SD.begin(SS, SPI, 80000000);
    cust::initOLED();
    cust::loadCfgFromFile(cfgPath);
    cust::begin();
    display.println("hardware success!");
    display.display();
    begin_web();
    display.print("web success!");
    display.display();
    delay(500);
}

void loop()
{
    dns.processNextRequest();
    cust::refresh_main_screen();
}
