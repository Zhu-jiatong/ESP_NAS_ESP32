#if !defined(UTILITY_FUNCT_h)
#define UTILITY_FUNCT_h

#define secToMil(sec) (sec * 1000)
#define milToSec(mil) (mil / 1000)
#define annodeRgbDigital(val) (1 - val)
#define annodeRgbAnalog(val) (255 - val)

#endif // UTILITY_FUNCT_h
