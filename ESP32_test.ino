#include "libs/my_SD.h"
#include "libs/my_web.h"
#include "libs/my_cfg.h"

void setup()
{
    my_SD.begin();
    begin_web(DNS_domain, ap_ssid, ap_psk);
}

void loop()
{
    dns.processNextRequest();
}
