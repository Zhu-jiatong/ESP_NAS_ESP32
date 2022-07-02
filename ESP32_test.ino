#include "libs/my_web.h"
#include "libs/my_cfg.h"
#include "libs/my_hardware.h"

void setup()
{
    custH::begin();
    begin_web(DNS_domain, ap_ssid, ap_psk);
}

void loop()
{
    dns.processNextRequest();
    custH::refresh_main_screen();
}
