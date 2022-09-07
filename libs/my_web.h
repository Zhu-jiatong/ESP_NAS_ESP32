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

void begin_web(const String domain, const char *ap_ssid, const char *ap_psk);
void getFile(AsyncWebServerRequest *request);
void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
void handleFile(AsyncWebServerRequest *request);
void handleNotFound(AsyncWebServerRequest *request);
void handleCardinfo(AsyncWebServerRequest *request);
void systemInfo(AsyncWebServerRequest *request);
String listFiles();
String processor(const String &var);
String getMime(const String &path);

void begin_web(const String domain, const char *ap_ssid, const char *ap_psk = nullptr)
{
    WiFi.softAP(ap_ssid, ap_psk);
    WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info)
                 {  digitalWrite(connect_LED_pin, annodeRgbDigital(HIGH));
                    delay(125);
                    digitalWrite(connect_LED_pin, annodeRgbDigital(LOW)); },
                 ARDUINO_EVENT_WIFI_AP_STACONNECTED);

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { if(!request->authenticate(http_id, http_psk))
                    return request->requestAuthentication();
                request->send(SD, "/index.html"); });
    server.on("/listfiles", HTTP_GET, [](AsyncWebServerRequest *request)
              { if(!request->authenticate(http_id, http_psk))
                    return request->requestAuthentication();
                request->send(200, "text/html", listFiles()); });
    server.on("/file", HTTP_GET, handleFile);
    server.on("/cardinfo", HTTP_GET, handleCardinfo);
    server.on("/systemInfo", systemInfo);
    server.on("/system", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send_P(200, "text/html", system_webpage); });
    server.onNotFound(handleNotFound);
    server.onFileUpload(handleUpload);
    AsyncElegantOTA.begin(&server, http_id, http_psk);
    server.begin();

    dns.setErrorReplyCode(DNSReplyCode::ServerFailure);
    dns.setTTL(300);
    dns.start(53, domain, WiFi.softAPIP());
}

void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{
    if (!index) // open the file on first call and store the file handle in the request object
    {
        request->_tempFile = SD.open("/" + filename, "w");
        isUpload = true;
    }
    if (len) // stream the incoming chunk to the opened file
        request->_tempFile.write(data, len);
    if (final)
    { // close the file handle as the upload is now done
        request->_tempFile.close();
        isUpload = false;
        request->redirect("/");
    }
}

String listFiles()
{
    File root = SD.open("/"), foundfile = root.openNextFile();
    String ret{"<table><tr><th align='left'>Name</th><th align='left'>Size</th><th></th><th></th></tr>"};
    for (; foundfile; foundfile = root.openNextFile())
    {
        if (foundfile.isDirectory())
        {
            ret += "<tr align='left'><td>" + String(foundfile.name()) + "</td>";
            if (SD.exists(String(foundfile.path()) + "/comix.html"))
                ret += "<td></td><td><button id=\"prev\" onclick=\"window.open(\'/" + String(foundfile.name()) + "/comix.html\'" + ")\">Read</button></td></tr>";
        }
        else
        {
            ret += "<tr align='left'><td>" + String(foundfile.name()) + "</td>";
            ret += "<td>" + readableSize(foundfile.size()) + "</td>";
            ret += "<td><button id=\"prev\" onclick=\"showPreview(\'" + String(foundfile.name()) + "\')\">Preview</button></td>";
            ret += "<td><button id=\"down\" onclick=\"downloadDeleteButton(\'" + String(foundfile.name()) + "\', \'download\')\">Download</button></td>";
            ret += "<td><button id=\"del\" onclick=\"downloadDeleteButton(\'" + String(foundfile.name()) + "\', \'delete\')\">Delete</button></td></tr>";
        }
    }
    ret += "</table>";
    foundfile.close();
    root.close();
    return ret;
}

void handleCardinfo(AsyncWebServerRequest *request)
{
    JSONVar ret;
    ret["freesd"] = readableSize(SD.totalBytes() - SD.usedBytes());
    ret["usedsd"] = readableSize(SD.usedBytes());
    ret["val"] = String(SD.usedBytes());
    ret["max"] = String(SD.totalBytes());
    request->send(200, "text/plain", JSON.stringify(ret));
}

void handleNotFound(AsyncWebServerRequest *request)
{
    if (!request->authenticate(http_id, http_psk))
        return request->requestAuthentication();
    String path{request->url()};
    if (SD.exists(path))
        request->send(SD, path, getMime(path), false);
    else
    {
        String message = "URI: " + request->url() + "\nMethod: " + request->methodToString() + "\nArguments: " + String(request->args()) + "\n";
        for (uint8_t i = 0; i < request->args(); i++)
            message += " NAME:" + request->argName(i) + "\n VALUE:" + request->arg(i) + "\n";
        request->send(404, "text/plain", message);
    }
}

void handleFile(AsyncWebServerRequest *request)
{
    if (!request->authenticate(http_id, http_psk))
        return request->requestAuthentication();
    if (request->hasParam("name") && request->hasParam("action"))
    {
        String fileName = "/" + request->getParam("name")->value();
        String fileAction = request->getParam("action")->value();

        if (!SD.exists(fileName))
            request->send(400, "text/plain", "ERROR: file does not exist" + fileName);
        else
        {
            if (fileAction == "download")
                request->send(SD, fileName, String(), true);
            else if (fileAction == "delete")
            {
                SD.remove(fileName);
                request->send(200, "text/plain", "Deleted File: " + fileName);
            }
            else
                request->send(400, "text/plain", "ERROR: invalid action param supplied");
        }
    }
    else
        request->send(400, "text/plain", "ERROR: name and action params required");
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
    if (path.endsWith(".html") || path.endsWith(".htm"))
        return "text/html";
    else if (path.endsWith(".css"))
        return "text/css";
    else if (path.endsWith(".js"))
        return "text/javascript";
    else if (path.endsWith(".png"))
        return "image/png";
    else if (path.endsWith(".gif"))
        return "image/gif";
    else if (path.endsWith(".jpg") || path.endsWith(".jpeg"))
        return "image/jpeg";
    else if (path.endsWith(".ico"))
        return "image/x-icon";
    else if (path.endsWith(".xml"))
        return "text/xml";
    else if (path.endsWith(".mp4"))
        return "video/mp4";
    else if (path.endsWith(".pdf"))
        return "application/pdf";
    return String();
}
#endif // MY_WEB_h
