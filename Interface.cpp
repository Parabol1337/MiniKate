#include "Interface.h"

void Interface::begin(Settings *settings, Perception *perception)
{
    this->settings = settings;
    this->perception = perception;

    server.on("/", HTTP_GET, [this]() { handleIndex(); });
    server.on("/config", HTTP_GET, [this]() { handleConfig(); });
    server.on("/config", HTTP_POST, [this]() { handleConfigSave(); });
    server.on("/update", HTTP_POST, [this]() { handleUploadDone(); }, [this]() { handleUpload(); });
    server.begin();
}

void Interface::update()
{
    server.handleClient();
}

String Interface::htmlHeader(String title)
{
    return String("<!doctype html><html lang='de'><head><meta charset='utf-8'><meta name='viewport' content='width=device-width,initial-scale=1'><title>") + title + "</title><style>body{font-family:Arial,sans-serif;max-width:760px;margin:30px auto;padding:0 16px;background:#f5f5f5;color:#222}main{background:white;padding:24px;border-radius:12px;box-shadow:0 2px 12px #0002}label{display:block;margin-top:12px;font-weight:bold}input{width:100%;box-sizing:border-box;padding:9px;margin-top:4px}button,a.button{display:inline-block;margin-top:16px;padding:10px 16px;border:0;border-radius:7px;background:#1677ff;color:white;text-decoration:none}table{border-collapse:collapse;width:100%}td{padding:6px;border-bottom:1px solid #ddd}</style></head><body><main><h1>" + title + "</h1>";
}

String Interface::htmlFooter()
{
    return "</main></body></html>";
}

void Interface::handleIndex()
{
    String html = htmlHeader("MiniKate");
    html += "<table>";
    html += "<tr><td>Roboter</td><td>" + settings->getRobotName() + "</td></tr>";
    html += "<tr><td>ID</td><td>" + String(settings->getRobotId()) + "</td></tr>";
    html += "<tr><td>Akku</td><td>" + String(perception->getBatteryVoltage(), 2) + " V</td></tr>";
    html += "<tr><td>Akku Rohwert</td><td>" + String(perception->getBatteryRaw()) + "</td></tr>";
    html += "<tr><td>Abstand</td><td>" + String(perception->getDistance(), 1) + " cm</td></tr>";
    html += "</table><a class='button' href='/config'>Konfiguration</a>";
    html += "<h2>Firmware-Update</h2><form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='firmware' accept='.bin' required><button type='submit'>Firmware hochladen</button></form>";
    html += htmlFooter();
    server.send(200, "text/html", html);
}

void Interface::handleConfig()
{
    String html = htmlHeader("MiniKate Konfiguration");
    html += "<form method='POST' action='/config'>";
    html += "<label>Roboter-ID<input type='number' name='robot_id' value='" + String(settings->getRobotId()) + "'></label>";
    html += "<label>Robotername<input name='robot_name' value='" + settings->getRobotName() + "'></label>";
    html += "<label>WLAN-SSID<input name='wifi_ssid' value='" + settings->getWifiSsid() + "'></label>";
    html += "<label>WLAN-Passwort<input type='password' name='wifi_pass' value='" + settings->getWifiPass() + "'></label>";
    html += "<label>Server<input name='server_host' value='" + settings->getServerHost() + "'></label>";
    html += "<label>Server-Port<input type='number' name='server_port' value='" + String(settings->getServerPort()) + "'></label>";
    html += "<label>Akku-Multiplikator<input type='number' step='0.00001' name='bat_multi' value='" + String(settings->getBatteryMultiplier(), 5) + "'></label>";
    html += "<label>Motorlimit links<input type='number' name='limit_left' value='" + String(settings->getMotorLeftLimit()) + "'></label>";
    html += "<label>Motorlimit rechts<input type='number' name='limit_right' value='" + String(settings->getMotorRightLimit()) + "'></label>";
    html += "<button type='submit'>Speichern</button></form><a href='/'>Zurück</a>";
    html += htmlFooter();
    server.send(200, "text/html", html);
}

void Interface::handleConfigSave()
{
    settings->save(
        server.arg("robot_id").toInt(),
        server.arg("robot_name"),
        server.arg("wifi_ssid"),
        server.arg("wifi_pass"),
        server.arg("server_host"),
        server.arg("server_port").toInt(),
        server.arg("bat_multi").toFloat(),
        server.arg("limit_left").toInt(),
        server.arg("limit_right").toInt()
    );

    server.send(200, "text/html", htmlHeader("Gespeichert") + "<p>Die Einstellungen wurden gespeichert. MiniKate startet neu.</p>" + htmlFooter());
    delay(800);
    ESP.restart();
}

void Interface::handleUpload()
{
    HTTPUpload& upload = server.upload();

    if (upload.status == UPLOAD_FILE_START)
    {
        Update.begin(UPDATE_SIZE_UNKNOWN);
    }
    else if (upload.status == UPLOAD_FILE_WRITE)
    {
        Update.write(upload.buf, upload.currentSize);
    }
    else if (upload.status == UPLOAD_FILE_END)
    {
        Update.end(true);
    }
}

void Interface::handleUploadDone()
{
    bool ok = !Update.hasError();
    server.send(ok ? 200 : 500, "text/html", htmlHeader(ok ? "Update erfolgreich" : "Update fehlgeschlagen") + (ok ? "<p>MiniKate startet neu.</p>" : "<p>Das Firmware-Update ist fehlgeschlagen.</p>") + htmlFooter());

    if (ok)
    {
        delay(800);
        ESP.restart();
    }
}
