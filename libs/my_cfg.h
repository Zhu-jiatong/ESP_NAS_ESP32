#if !defined(MY_CFG_h)
#define MY_CFG_h

#include "utilityFunctions.h"

const char *ap_ssid{"Tony_NAS"};
const char *ap_psk{""};
const char *http_id{"Tony_Zhu_admin"};
const char *http_psk{"iLoveHuman(s)"};
const String DNS_domain{"www.tonynas.com"};

const uint wake_time(secToMil(90));
const uint8_t connect_LED_pin(14);

#endif // MY_CFG_h
