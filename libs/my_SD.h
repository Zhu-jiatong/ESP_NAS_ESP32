#if !defined(MY_SD_h)
#define MY_SD_h

#include <SD.h>

class my_SDClass
{
public:
    uint8_t cs_pin, inst_pin;
    bool haveSD;
    struct SDinfo
    {
        uint64_t max_size{}, used_size{}, free_size{};
    } SD_info;
    bool begin();
} my_SD;

bool my_SDClass::begin()
{
    bool ret = SD.begin(SS, SPI, 80000000);
    if (ret)
    {
        SD_info.max_size = SD.totalBytes();
        SD_info.used_size = SD.usedBytes();
        SD_info.free_size = SD_info.max_size - SD_info.used_size;
    }

    return ret;
}

#endif // MY_SD_h
