#include "Connection.h"
#include <RemoteXY.h>
#include "RemoteXYNet_WiFiExisting.h"

#define CONN_DEBUG_PRINT(x)   if (CONNECTION_DEBUG) { Serial.print(x); }
#define CONN_DEBUG_PRINTLN(x) if (CONNECTION_DEBUG) { Serial.println(x); }

#pragma pack(push, 1)
uint8_t const PROGMEM RemoteXY_CONF_PROGMEM[] =
  { 255,3,0,8,0,84,0,19,0,0,0,0,16,1,200,90,1,1,5,0,
  4,169,255,23,82,16,118,26,1,120,57,24,24,0,1,24,90,117,114,195,
  188,99,107,0,67,100,252,63,46,77,24,16,1,4,2,56,88,23,176,205,
  26,71,14,241,63,63,51,0,2,24,255,0,0,52,195,0,0,52,67,0,
  0,0,0,0,0,160,65,0,0,0,0 };

struct {
  int8_t slider_speed;
  uint8_t button_back;
  int8_t slider_turn;
  float value_count;
  float instrument_compas;
  uint8_t connect_flag;
} RemoteXY;
#pragma pack(pop)

void Connection::begin(Settings* settings)
{
    _settings = settings;
    CONN_DEBUG_PRINTLN("[CONN] begin");
    _status = CONN_CONNECTING;
    startWifi();
}

void Connection::update()
{
    unsigned long now = millis();

    if (_apMode)
    {
        _status = CONN_AP_MODE;
        return;
    }

    if (_remoteStarted)
    {
        RemoteXY_Handler();
    }

    static wl_status_t lastWifiStatus = WL_IDLE_STATUS;

    if (lastWifiStatus != WiFi.status())
    {
        lastWifiStatus = WiFi.status();
        CONN_DEBUG_PRINT("[WIFI] Status geaendert: ");
        CONN_DEBUG_PRINTLN((int)WiFi.status());
    }

    if (WiFi.status() != WL_CONNECTED)
    {
        _status = CONN_DISCONNECTED;

        if (now - _wifiStartAt > 30000)
        {
            CONN_DEBUG_PRINTLN("[WIFI] Timeout - starte Fallback AP");
            startFallbackAp();
        }

        return;
    }

    static bool wifiConnectedPrinted = false;

    if (!wifiConnectedPrinted)
    {
        wifiConnectedPrinted = true;
        CONN_DEBUG_PRINTLN("[WIFI] Verbunden");
        CONN_DEBUG_PRINT("[WIFI] IP: ");
        CONN_DEBUG_PRINTLN(WiFi.localIP());
        CONN_DEBUG_PRINT("[WIFI] RSSI: ");
        CONN_DEBUG_PRINTLN(WiFi.RSSI());
        startRemote();
    }

    if (!_client.connected())
    {
        _status = CONN_CONNECTING;

        if (now - _lastReconnectTry > 3000)
        {
            _lastReconnectTry = now;
            connectTcp();
        }

        return;
    }

    _status = CONN_CONNECTED;

    while (_client.available() > 0)
    {
        char c = _client.read();

        if (c == '\n')
        {
            _lastPacket = _rxBuffer;
            CONN_DEBUG_PRINT("[RX] ");
            CONN_DEBUG_PRINTLN(_lastPacket);
            _rxBuffer = "";
            _status = CONN_MESSAGE_AVAILABLE;
            return;
        }

        if (c != '\r')
        {
            _rxBuffer += c;
        }
    }
}

void Connection::send(JsonDocument& doc)
{
    if (!_client.connected())
    {
        CONN_DEBUG_PRINTLN("[TX] Nicht gesendet - TCP nicht verbunden");
        return;
    }

    if (CONNECTION_DEBUG)
    {
        Serial.print("[TX] ");
        serializeJson(doc, Serial);
        Serial.println();
    }

    serializeJson(doc, _client);
    _client.print('\n');
}

void Connection::send(String json)
{
    if (!_client.connected())
    {
        CONN_DEBUG_PRINTLN("[TX] Nicht gesendet - TCP nicht verbunden");
        return;
    }

    CONN_DEBUG_PRINT("[TX] ");
    CONN_DEBUG_PRINTLN(json);
    _client.println(json);
}

void Connection::sendStatus(float battery, String drive, String perception, float distance, float heading)
{
    if (_settings == nullptr)
    {
        CONN_DEBUG_PRINTLN("[STATUS] Fehler: settings == nullptr");
        return;
    }

    JsonDocument doc;
    doc["type"] = "status";
    doc["robot_id"] = _settings->getRobotId();
    doc["name"] = _settings->getRobotName();
    doc["battery"] = battery;
    doc["drive"] = drive;
    doc["angle"] = heading;
    doc["distance"] = distance;
    doc["perception"] = perception;
    doc["wifi"] = WiFi.RSSI();
    doc["uptime"] = millis() / 1000;
    send(doc);
}

void Connection::sendRfid(String uid)
{
    CONN_DEBUG_PRINT("[EVENT] RFID ");
    CONN_DEBUG_PRINTLN(uid);
    sendEvent("rfid", "detected", uid);
}

void Connection::sendEvent(String module, String action, String value)
{
    if (_settings == nullptr)
    {
        CONN_DEBUG_PRINTLN("[EVENT] Fehler: settings == nullptr");
        return;
    }

    JsonDocument doc;
    doc["type"] = "event";
    doc["robot_id"] = _settings->getRobotId();
    doc["module"] = module;
    doc["action"] = action;
    doc["value"] = value;
    send(doc);
}

bool Connection::available()
{
    return _status == CONN_MESSAGE_AVAILABLE && _lastPacket.length() > 0;
}

String Connection::receive()
{
    String packet = _lastPacket;
    _lastPacket = "";
    _status = _client.connected() ? CONN_CONNECTED : CONN_DISCONNECTED;
    return packet;
}

bool Connection::read(String& type, String& module, String& action, String& value)
{
    if (_lastPacket.length() == 0)
    {
        return false;
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, _lastPacket);

    if (error)
    {
        CONN_DEBUG_PRINT("[JSON] Fehler: ");
        CONN_DEBUG_PRINTLN(error.c_str());
        CONN_DEBUG_PRINT("[JSON] Paket war: ");
        CONN_DEBUG_PRINTLN(_lastPacket);
        _lastPacket = "";
        return false;
    }

    _lastPacket = "";
    type   = doc["type"]   | "";
    module = doc["module"] | "";
    action = doc["action"] | "";

    if (doc["value"].is<int>())
    {
        value = String((int)doc["value"]);
    }
    else if (doc["value"].is<float>())
    {
        value = String((float)doc["value"]);
    }
    else
    {
        value = doc["value"] | "";
    }

    CONN_DEBUG_PRINT("[CMD READ] type=");
    CONN_DEBUG_PRINT(type);
    CONN_DEBUG_PRINT(" module=");
    CONN_DEBUG_PRINT(module);
    CONN_DEBUG_PRINT(" action=");
    CONN_DEBUG_PRINT(action);
    CONN_DEBUG_PRINT(" value=");
    CONN_DEBUG_PRINTLN(value);

    _status = _client.connected() ? CONN_CONNECTED : CONN_DISCONNECTED;
    return type.length() > 0;
}

ConnectionStatus Connection::getStatus() const { return _status; }
bool Connection::isApMode() const { return _apMode; }
bool Connection::isRemoteConnected() const { return RemoteXY.connect_flag == 1; }

bool Connection::getRemoteBackPressed()
{
    bool pressed = RemoteXY.button_back == 1;
    bool event = pressed && !_lastBackButton;
    _lastBackButton = pressed;
    return event;
}

int Connection::getRemoteSpeed()
{
    int8_t speed = RemoteXY.slider_speed;
    return (int)(speed * 3.5);
}

int Connection::getRemoteTurn()
{
    int8_t turn = RemoteXY.slider_turn;
    return (int)turn;
}

void Connection::setRemoteCounter(int value) { RemoteXY.value_count = (float)value; }
void Connection::setRemoteCompas(int value) { RemoteXY.instrument_compas = (float)value; }

void Connection::startWifi()
{
    CONN_DEBUG_PRINTLN("[WIFI] Starte WLAN");
    WiFi.mode(WIFI_STA);

    if (_settings == nullptr)
    {
        CONN_DEBUG_PRINTLN("[WIFI] Fehler: settings == nullptr");
        startFallbackAp();
        return;
    }

    if (!_settings->hasWifiConfig())
    {
        CONN_DEBUG_PRINTLN("[WIFI] Keine WLAN-Konfiguration gefunden");
        startFallbackAp();
        return;
    }

    CONN_DEBUG_PRINT("[WIFI] SSID: ");
    CONN_DEBUG_PRINTLN(_settings->getWifiSsid());

    WiFi.begin(_settings->getWifiSsid().c_str(), _settings->getWifiPass().c_str());
    _wifiStartAt = millis();
}

void Connection::startFallbackAp()
{
    _apMode = true;
    _status = CONN_AP_MODE;
    WiFi.mode(WIFI_AP);

    String apName = "MiniKate-" + String((uint32_t)ESP.getEfuseMac(), HEX);
    WiFi.softAP(apName.c_str(), "minikate123");

    CONN_DEBUG_PRINTLN("[AP] Fallback AP gestartet");
    CONN_DEBUG_PRINT("[AP] Name: ");
    CONN_DEBUG_PRINTLN(apName);
    CONN_DEBUG_PRINT("[AP] IP: ");
    CONN_DEBUG_PRINTLN(WiFi.softAPIP());
}

bool Connection::connectTcp()
{
    if (_settings == nullptr)
    {
        CONN_DEBUG_PRINTLN("[TCP] Fehler: settings == nullptr");
        return false;
    }

    _client.stop();
    CONN_DEBUG_PRINT("[TCP] Verbinde zu ");
    CONN_DEBUG_PRINT(_settings->getServerHost());
    CONN_DEBUG_PRINT(":");
    CONN_DEBUG_PRINTLN(_settings->getServerPort());

    if (_client.connect(_settings->getServerHost().c_str(), _settings->getServerPort()))
    {
        CONN_DEBUG_PRINTLN("[TCP] VERBUNDEN");
        _status = CONN_CONNECTED;
        sendHello();
        return true;
    }

    CONN_DEBUG_PRINTLN("[TCP] FEHLER - Verbindung fehlgeschlagen");
    _status = CONN_DISCONNECTED;
    return false;
}

void Connection::sendHello()
{
    if (_settings == nullptr)
    {
        CONN_DEBUG_PRINTLN("[HELLO] Fehler: settings == nullptr");
        return;
    }

    JsonDocument doc;
    doc["type"] = "hello";
    doc["robot_id"] = _settings->getRobotId();
    doc["name"] = _settings->getRobotName();
    doc["firmware"] = String(hashString(String(FW_VERSION)), HEX);
    doc["hardware"] = HW_VERSION;
    doc["ip"] = WiFi.localIP().toString();
    doc["wifi"] = WiFi.RSSI();
    doc["uptime"] = millis() / 1000;
    CONN_DEBUG_PRINTLN("[TX] HELLO");
    send(doc);
}

void Connection::startRemote()
{
    if (_remoteStarted)
    {
        return;
    }

    RemoteXYNet* net = new CRemoteXYNet_WiFiExisting();
    RemoteXYGui* gui = RemoteXYEngine.addGui(RemoteXY_CONF_PROGMEM, &RemoteXY);
    gui->addConnectionServer(net, 6377);
    _remoteStarted = true;
    CONN_DEBUG_PRINTLN("[REMOTEXY] gestartet");
}

uint32_t Connection::hashString(const String& text)
{
    uint32_t hash = 2166136261UL;

    for (size_t i = 0; i < text.length(); i++)
    {
        hash ^= text[i];
        hash *= 16777619UL;
    }

    return hash;
}
