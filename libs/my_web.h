#if !defined(MY_WEB_h)
#define MY_WEB_h

#include <Arduino_JSON.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include <WiFi.h>
#include <SD.h>
#include "my_cfg.h"
#include "sysWebpage.h"
#include <AsyncElegantOTA.h>

AsyncWebServer server(80);
DNSServer dns;

bool isUpload(false);

void begin_web();
void getFile(AsyncWebServerRequest *request);
void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
void handleFile(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
void handleNotFound(AsyncWebServerRequest *request);
void handleCardinfo(AsyncWebServerRequest *request);
void systemInfo(AsyncWebServerRequest *request);
void handleListFiles(AsyncWebServerRequest *request);
void promptAuth(AsyncWebServerRequest *request);
String listFiles();
String getMime(const String &path);
String listFiles(String path);
bool deleteAll(String path);

void begin_web()
{
    WiFi.softAP(ap_ssid.c_str(), ap_psk.c_str());
    WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info)
                 {  digitalWrite(connect_LED_pin, annodeRgbDigital(HIGH));
                    delay(125);
                    digitalWrite(connect_LED_pin, annodeRgbDigital(LOW)); },
                 ARDUINO_EVENT_WIFI_AP_STACONNECTED);
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { promptAuth(request);
                request->send(SD, "/index.html"); });
    server.on("/listfiles", HTTP_GET, handleListFiles);
    server.on(
        "/file", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, handleFile);
    server.on("/cardinfo", HTTP_GET, handleCardinfo);
    server.on("/systemInfo", systemInfo);
    server.on("/system", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send_P(200, "text/html", system_webpage); });
    server.onNotFound(handleNotFound);
    server.onFileUpload(handleUpload);
    AsyncElegantOTA.begin(&server, http_id.c_str(), http_psk.c_str());
    server.begin();

    dns.setErrorReplyCode(DNSReplyCode::ServerFailure);
    dns.setTTL(300);
    dns.start(53, DNS_domain, WiFi.softAPIP());
}

void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{
    if (!index) // open the file on first call and store the file handle in the request object
        request->_tempFile = SD.open(request->getParam("path")->value() + filename, FILE_WRITE);
    if (len) // stream the incoming chunk to the opened file
        request->_tempFile.write(data, len);
    if (final)
    { // close the file handle as the upload is now done
        request->_tempFile.close();
        request->redirect("/");
    }
}

void handleListFiles(AsyncWebServerRequest *request)
{
    promptAuth(request);
    request->send(200, "application/json", listFiles(request->getParam("path")->value()));
}

String listFiles(String path)
{
    JSONVar ret;
    File root = SD.open(path), foundFile;
    while (foundFile = root.openNextFile())
    {
        ret[foundFile.name()]["size"] = readableSize(foundFile.size());
        ret[foundFile.name()]["isDir"] = foundFile.isDirectory();
        ret[foundFile.name()]["path"] = foundFile.path();
    }
    foundFile.close();
    root.close();
    return JSON.stringify(ret);
}

void handleCardinfo(AsyncWebServerRequest *request)
{
    JSONVar ret;
    ret["freesd"] = readableSize(SD.totalBytes() - SD.usedBytes());
    ret["usedsd"] = readableSize(SD.usedBytes());
    ret["val"] = String(SD.usedBytes());
    ret["max"] = String(SD.totalBytes());
    request->send(200, "application/json", JSON.stringify(ret));
}

void handleNotFound(AsyncWebServerRequest *request)
{
    promptAuth(request);
    String path{request->url()};
    if (SD.exists(path))
        request->send(SD, path, getMime(path), false);
    else
    {
        JSONVar ret;
        ret["uri"] = request->url();
        ret["method"] = request->methodToString();
        for (uint8_t i = 0; i < request->params(); ++i)
            ret["args"][request->getParam(i)->name()] = request->getParam(i)->value();
        request->send(404, "text/plain", JSON.stringify(ret));
    }
}

void handleFile(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
{
    auto rcvData = JSON.parse((const char *)data);
    String filePath = (const char *)rcvData["path"];
    if (SD.exists(filePath))
    {
        String action = (const char *)rcvData["action"],
               newPath = (const char *)rcvData["newPath"];
        JSONVar ret;
        if (action == "download")
        {
            request->send(SD, filePath, String(), true);
            return;
        }
        else if (action == "rename")
            ret["success"] = SD.rename(filePath, newPath);
        else if (action == "delete")
            ret["success"] = deleteAll(filePath);
        else if (action == "mkdir")
            ret["success"] = SD.mkdir(filePath + "/" + newPath);
        else
        {
            request->send(400, "text/plain", "ERROR: invalid action param");
            return;
        }
        ret["action"] = action;
        ret["target"] = filePath;
        request->send(200, "application/json", JSON.stringify(ret));
    }
    else
        request->send(400, "text/plain", "ERROR: file does not exist" + filePath);
}

void systemInfo(AsyncWebServerRequest *request)
{
    JSONVar ret;
    ret["ERROR_MSG"] = "NONE";
    ret["IP_ADDR"] = WiFi.softAPIP().toString();
    ret["MAC_ADDR_AP"] = WiFi.softAPmacAddress();
    ret["MAC_ADDR_STA"] = WiFi.macAddress();
    auto tempSD = SD.cardType();
    ret["SD_TYPE"] = (tempSD = CARD_NONE) ? "NONE" : (tempSD = CARD_MMC) ? "MMC"
                                                 : (tempSD = CARD_SD)    ? "SD"
                                                 : (tempSD = CARD_SDHC)  ? "SDHC"
                                                                         : "UNKNOWN";
    ret["SD_TOTAL_SIZE"] = readableSize(SD.totalBytes());
    ret["SD_USED_SIZE"] = readableSize(SD.usedBytes());
    ret["SD_LEFT_SIZE"] = readableSize(SD.totalBytes() - SD.usedBytes());
    ret["ESP_CHIP_MOD"] = ESP.getChipModel();
    ret["ESP_CHIP_CORE"] = ESP.getChipCores();
    ret["ESP_CHIP_REV"] = ESP.getChipRevision();
    ret["ESP_CPU_FREQ"] = String(ESP.getCpuFreqMHz()) + "MHz";
    ret["ESP_CYCLE_COUNT"] = String(ESP.getCycleCount());
    ret["ESP_EFUSE_MAC"] = String(ESP.getEfuseMac());
    ret["ESP_SDK_VER"] = ESP.getSdkVersion();
    auto tempFM = ESP.getFlashChipMode();
    ret["ESP_FLASH_MODE"] = (tempFM = FM_QIO) ? "QIO" : (tempFM = FM_QOUT)    ? "QOUT"
                                                    : (tempFM = FM_DIO)       ? "DIO"
                                                    : (tempFM = FM_DOUT)      ? "DOUT"
                                                    : (tempFM = FM_FAST_READ) ? "FAST_READ"
                                                    : (tempFM = FM_SLOW_READ) ? "SLOW_READ"
                                                                              : "UNKNOWN";
    ret["ESP_FLASH_SIZE"] = String(ESP.getFlashChipSize());
    ret["ESP_FLASH_SPD"] = String(ESP.getFlashChipSpeed());
    ret["ESP_HEAP_SIZE"] = String(ESP.getHeapSize());
    ret["ESP_FREE_HEAP"] = String(ESP.getFreeHeap());
    ret["ESP_MAX_HEAP"] = String(ESP.getMaxAllocHeap());
    ret["ESP_MIN_HEAP"] = String(ESP.getMinFreeHeap());
    ret["ESP_PSRAM_SIZE"] = String(ESP.getPsramSize());
    ret["ESP_FREE_PSRAM"] = String(ESP.getFreePsram());
    ret["ESP_MAX_PSRAM"] = String(ESP.getMaxAllocPsram());
    ret["ESP_MIN_PSRAM"] = String(ESP.getMinFreePsram());
    ret["ESP_FREE_SKETCH_SPACE"] = String(ESP.getFreeSketchSpace());
    ret["ESP_SKETCH_SIZE"] = String(ESP.getSketchSize());
    ret["ESP_SKETCH_MD5"] = ESP.getSketchMD5();
    request->send(200, "application/json", JSON.stringify(ret));
}

String getMime(const String &path)
{
    return (const char *)JSON.parse(SD.open("/fileType.json").readString())[path.substring(path.lastIndexOf("."))];
}

void promptAuth(AsyncWebServerRequest *request)
{
    if (!request->authenticate(http_id.c_str(), http_psk.c_str()))
        return request->requestAuthentication();
}

bool deleteAll(String path)
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
#endif // MY_WEB_h
