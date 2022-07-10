#if !defined(MY_CFG_h)
#define MY_CFG_h

#define secToMil(sec) (sec * 1000)
#define milToSec(mil) (mil / 1000)
#define annodeRgbDigital(val) (1 - val)
#define annodeRgbAnalog(val) (255 - val)

const char *ap_ssid{"Tony_NAS"};
const char *ap_psk{""};
const char *http_id{"Tony_Zhu_admin"};
const char *http_psk{"iLoveHuman(s)"};
const String DNS_domain{"www.tonynas.com"};

const uint wake_time(secToMil(90));
const uint8_t connect_LED_pin(14);
inline String humanReadableSize(uint64_t bytes)
{
    if (bytes < 1024)
        return String(bytes / 1.0) + " B";
    else if (bytes < (1024 * 1024))
        return String(bytes / 1024.0) + " KB";
    else if (bytes < (1024 * 1024 * 1024))
        return String(bytes / 1024.0 / 1024.0) + " MB";
    else
        return String(bytes / 1024.0 / 1024.0 / 1024.0) + " GB";
}

#endif // MY_CFG_h
