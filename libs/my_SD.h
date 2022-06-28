#if !defined(MY_SD_h)
#define MY_SD_h

#include <SD.h>

class my_SDClass
{
public:
    uint8_t cs_pin, inst_pin;
    bool haveSD;
    bool begin();
} my_SD;

bool my_SDClass::begin()
{
    bool ret = SD.begin(SS, SPI, 80000000);
    return ret;
}

#endif // MY_SD_h
