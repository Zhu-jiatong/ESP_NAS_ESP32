#if !defined(MY_CFG_h)
#define MY_CFG_h

#include "utilityFunctions.h"
#include <Arduino_JSON.h>
#include <Adafruit_SSD1306.h>
Adafruit_SSD1306 display(128, 64, &Wire, -1, 800000);

String ap_ssid{"NAS"};
String ap_psk{""};
String http_id{"Tony_Zhu_admin"};
String http_psk{"iLoveHuman(s)"};
String DNS_domain{"www.nas.com"};

uint wake_time(secToMil(90));
const uint8_t connect_LED_pin(14);
namespace cust
{
    void loadCfgFromFile(const char *path)
    {
        JSONVar cfgData;
        if (SD.exists(path))
        {
            File cfgFile = SD.open(path);
            if (cfgFile.size())
            {
                display.println("config found");
                display.display();

                cfgData = JSON.parse(cfgFile.readString());
                display.println("config read");
                display.display();

                ap_ssid = (const char *)cfgData["ap_ssid"];
                ap_psk = (const char *)cfgData["ap_psk"];
                DNS_domain = (const char *)cfgData["DNS_domain"];
                wake_time = secToMil((int)cfgData["wake_time"]);
                http_id = (const char *)cfgData["http_id"];
                http_psk = (const char *)cfgData["http_psk"];
                display.println("config loaded");
                display.display();

                display.println("done!");
                display.display();
                cfgFile.close();
                return;
            }
            cfgFile.close();
        }
        display.println("config NOT FOUND");
        display.println("using default");
        display.display();

        File newCfgFile = SD.open(path, FILE_WRITE);
        cfgData["ap_ssid"] = ap_ssid;
        cfgData["ap_psk"] = ap_psk;
        cfgData["DNS_domain"] = DNS_domain;
        cfgData["wake_time"] = (int)milToSec(wake_time);
        cfgData["http_id"] = http_id;
        cfgData["http_psk"] = http_psk;
        newCfgFile.print(JSON.stringify(cfgData));
        display.println("new config created");
        newCfgFile.close();
        display.println("done!");
    }
} // namespace cust

#endif // MY_CFG_h
