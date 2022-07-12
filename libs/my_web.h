#if !defined(MY_WEB_h)
#define MY_WEB_h

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include <WiFi.h>
#include <SD.h>
#include "my_cfg.h"
#include <AsyncElegantOTA.h>

AsyncWebServer server(80);
DNSServer dns;

bool isUpload(false);

void begin_web(const String domain, const char *ap_ssid, const char *ap_psk);
void getFile(AsyncWebServerRequest *request);
void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
void handleFile(AsyncWebServerRequest *request);
void handleNotFound(AsyncWebServerRequest *request);
String listFiles();
String processor(const String &var);
String getMime(const String &path);

void begin_web(const String domain, const char *ap_ssid, const char *ap_psk = nullptr)
{
    WiFi.softAP(ap_ssid, ap_psk);
    WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info)
                 {  digitalWrite(connect_LED_pin, annodeRgbDigital(HIGH));
                    delay(250);
                    digitalWrite(connect_LED_pin, annodeRgbDigital(LOW)); },
                 ARDUINO_EVENT_WIFI_AP_STACONNECTED);

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { if(!request->authenticate(http_id, http_psk))
                    return request->requestAuthentication();
                request->send(SD, "/index.html", String(), false, processor); });
    server.on("/listfiles", HTTP_GET, [](AsyncWebServerRequest *request)
              { if(!request->authenticate(http_id, http_psk))
                    return request->requestAuthentication();
                request->send(200, "text/html", listFiles()); });
    server.on("/file", HTTP_GET, handleFile);
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

String processor(const String &var)
{
    if (var == "FREESD")
        return readableSize(SD.totalBytes() - SD.usedBytes());
    if (var == "USEDSD")
        return readableSize(SD.usedBytes());
    if (var == "TOTALSD")
        return readableSize(SD.totalBytes());
    return String();
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
