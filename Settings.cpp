#include "Settings.h"

void Settings::begin()
{
    prefs.begin("minikate", false);
}

int Settings::getRobotId()
{
    return prefs.getInt("robot_id", 1);
}

String Settings::getRobotName()
{
    return prefs.getString("robot_name", "MiniKate");
}

String Settings::getWifiSsid()
{
    return prefs.getString("wifi_ssid", "");
}

String Settings::getWifiPass()
{
    return prefs.getString("wifi_pass", "");
}

String Settings::getServerHost()
{
    return prefs.getString("server_host", "10.40.74.204");
}

int Settings::getServerPort()
{
    return prefs.getInt("server_port", 8010);
}
int Settings::getMotorLeftLimit()
{
    return prefs.getInt("limit_left", 255);
}
int Settings::getMotorRightLimit()
{
    return prefs.getInt("limit_right", 255);
}

float Settings::getBatteryMultiplier()
{
    return prefs.getFloat("bat_multi", 0.02476f);
}

bool Settings::hasWifiConfig()
{
    return getWifiSsid().length() > 0;
}

void Settings::save(
    int robotId,
    String robotName,
    String wifiSsid,
    String wifiPass,
    String serverHost,
    int serverPort,
    float batteryMultiplier,
    int motorLeftLimit,
    int motorRightLimit
)
{
    prefs.putInt("robot_id", robotId);
    prefs.putString("robot_name", robotName);
    prefs.putString("wifi_ssid", wifiSsid);
    prefs.putString("wifi_pass", wifiPass);
    prefs.putString("server_host", serverHost);
    prefs.putInt("server_port", serverPort);
    prefs.putFloat("bat_multi", batteryMultiplier);
    prefs.putInt("limit_left", motorLeftLimit);
    prefs.putInt("limit_right", motorRightLimit);
}
