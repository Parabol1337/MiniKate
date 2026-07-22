#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include "Config.h"

class Settings
{
public:
    void begin();
    int getRobotId();
    String getRobotName();
    String getWifiSsid();
    String getWifiPass();
    String getServerHost();
    int getServerPort();
    int getMotorRightLimit();
    int getMotorLeftLimit();
    float getBatteryMultiplier();
    bool hasWifiConfig();
    void save(
        int robotId,
        String robotName,
        String wifiSsid,
        String wifiPass,
        String serverHost,
        int serverPort,
        float batteryMultiplier,
        int motorLeftLimit,
        int motorRightLimit
    );
private:
    Preferences prefs;
};
