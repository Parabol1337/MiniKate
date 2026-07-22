#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoJson.h>

#include "Config.h"
#include "Settings.h"

#define CONNECTION_DEBUG true

enum ConnectionStatus
{
    CONN_DISCONNECTED,
    CONN_CONNECTING,
    CONN_CONNECTED,
    CONN_MESSAGE_AVAILABLE,
    CONN_AP_MODE
};

class Connection
{
public:
    void begin(Settings* settings);
    void update();
    void send(JsonDocument& doc);
    void send(String json);
    void sendStatus(float battery, String drive, String perception, float distance, float heading);
    void sendRfid(String uid);
    void sendEvent(String module, String action, String value);
    bool available();
    String receive();
    bool read(String& type, String& module, String& action, String& value);
    ConnectionStatus getStatus() const;
    bool isApMode() const;
    bool isRemoteConnected() const;
    int getRemoteSpeed();
    int getRemoteTurn();
    bool getRemoteBackPressed();
    void setRemoteCounter(int value);
    void setRemoteCompas(int value);
private:
    Settings* _settings = nullptr;
    WiFiClient _client;
    ConnectionStatus _status = CONN_DISCONNECTED;
    bool _apMode = false;
    bool _remoteStarted = false;
    unsigned long _wifiStartAt = 0;
    unsigned long _lastReconnectTry = 0;
    String _lastPacket = "";
    String _rxBuffer = "";
    bool _lastBackButton = false;
    bool _lastMpuButton = false;
    void startWifi();
    void startFallbackAp();
    bool connectTcp();
    void sendHello();
    void startRemote();
    uint32_t hashString(const String& text);
};
