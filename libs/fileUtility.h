#if !defined(FILEUTILS_h)
#define FILEUTILS_h

#include <SD.h>
#include <Arduino_JSON.h>
#include <Arduino.h>

namespace cust
{
    inline String readableSize(float bytes)
    {
        const char *units[]{"B", "kB", "MB", "GB"};
        uint8_t i = 0;
        for (; units[i] && bytes >= 1024; ++i)
            bytes /= 1024;
        return String(bytes, 1) + " " + units[i];

/*         if (bytes < 1024)
            return String(bytes / 1.0) + " B";
        else if (bytes < (1024 * 1024))
            return String(bytes / 1024.0) + " KB";
        else if (bytes < (1024 * 1024 * 1024))
            return String(bytes / 1024.0 / 1024.0) + " MB";
        else
            return String(bytes / 1024.0 / 1024.0 / 1024.0) + " GB";
 */    }

bool deleteAll(const String &path)
{
    if (SD.exists(path))
    {
        File root = SD.open(path);
        if (root.isDirectory())
        {
            File found;
            while (found = root.openNextFile())
                deleteAll(found.path());
            root.close();
            return SD.rmdir(path);
        }
        else
        {
            root.close();
            return SD.remove(path);
        }
    }
    else
        return false;
}

String listFiles(const String &path)
{
    JSONVar ret;
    File root = SD.open(path), foundFile;
    while (foundFile = root.openNextFile())
    {
        if (!foundFile.isDirectory())
        {
            ret[foundFile.name()]["isDir"] = false;
            ret[foundFile.name()]["size"] = String(foundFile.size());
        }
        else
            ret[foundFile.name()]["isDir"] = true;
        ret[foundFile.name()]["path"] = foundFile.path();
    }
    foundFile.close();
    root.close();
    return JSON.stringify(ret);
}

JSONVar parseJSON(const String &path)
{
    if (SD.exists(path))
    {
        File dataFile = SD.open(path);
        JSONVar dataStr = JSON.parse(dataFile.readString());
        dataFile.close();
        return dataStr;
    }
    return nullptr;
}

} // namespace cust

#endif // FILEUTILS_h
