#if !defined(MY_WEB_h)
#define MY_WEB_h

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include <WiFi.h>
#include "my_SD.h"
#include "my_cfg.h"

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
    server.begin();

    dns.setErrorReplyCode(DNSReplyCode::ServerFailure);
    dns.setTTL(300);
    dns.start(53, domain, WiFi.softAPIP());
}

void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{
    if (!index) // open the file on first call and store the file handle in the request object
        request->_tempFile = SD.open("/" + filename, "w");
    if (len) // stream the incoming chunk to the opened file
        request->_tempFile.write(data, len);

    if (final)
    { // close the file handle as the upload is now done
        request->_tempFile.close();
        request->redirect("/");
    }
}

String listFiles()
{
    File root = SD.open("/");
    File foundfile = root.openNextFile();
    String ret{"<table><tr><th align='left'>Name</th><th align='left'>Size</th><th></th><th></th></tr>"};
    while (foundfile)
    {
        if (foundfile.isDirectory())
            ret += "<tr align='left'><td>" + String(foundfile.name()) + "</td></tr>";
        else
        {
            ret += "<tr align='left'><td>" + String(foundfile.name()) + "</td>";
            ret += "<td>" + humanReadableSize(foundfile.size()) + "</td>";
            ret += "<td><button id=\"prev\" onclick=\"showPreview(\'" + String(foundfile.name()) + "\')\">Preview</button></td>";
            ret += "<td><button id=\"down\" onclick=\"downloadDeleteButton(\'" + String(foundfile.name()) + "\', \'download\')\">Download</button></td>";
            ret += "<td><button id=\"del\" onclick=\"downloadDeleteButton(\'" + String(foundfile.name()) + "\', \'delete\')\">Delete</button></td></tr>";
        }
        foundfile = root.openNextFile();
    }
    ret += "</table>";
    foundfile.close();
    root.close();
    return ret;
}

String processor(const String &var)
{
    if (var == "FREESD")
        return humanReadableSize(SD.totalBytes() - SD.usedBytes());
    if (var == "USEDSD")
        return humanReadableSize(SD.usedBytes());
    if (var == "TOTALSD")
        return humanReadableSize(SD.totalBytes());
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
