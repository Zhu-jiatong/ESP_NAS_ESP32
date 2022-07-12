#if !defined(UTILITY_FUNCT_h)
#define UTILITY_FUNCT_h

#define secToMil(sec) (sec * 1000)
#define milToSec(mil) (mil / 1000)
#define annodeRgbDigital(val) (1 - val)
#define annodeRgbAnalog(val) (255 - val)

inline String readableSize(uint64_t bytes)
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

#endif // UTILITY_FUNCT_h
