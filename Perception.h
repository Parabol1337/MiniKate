#pragma once

#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include "Config.h"
#include "Settings.h"

enum RfidStatus
{
    RFID_IDLE,
    RFID_DETECTED
};

enum BatteryStatus
{
    BATTERY_OK,
    BATTERY_LOW
};

enum DistanceStatus
{
    DISTANCE_CLEAR,
    DISTANCE_NEAR,
    DISTANCE_STOP,
    DISTANCE_UNKNOWN
};

class Perception
{
public:
    void begin(Settings *settings);
    void update();
    bool rfidAvailable();
    String getUid();
    float getBatteryVoltage();
    int getBatteryRaw();
    float getDistance();
    RfidStatus getRfidStatus();
    BatteryStatus getBatteryStatus();
    DistanceStatus getDistanceStatus();
    String getRfidStatusText();
    String getBatteryStatusText();
    String getDistanceStatusText();
private:
    MFRC522 rfid = MFRC522(RFID_SS, RFID_RST);
    String uid = "";
    Settings *settings = nullptr;
    int battraw = 0;
    RfidStatus rfidStatus = RFID_IDLE;
    BatteryStatus batteryStatus = BATTERY_OK;
    DistanceStatus distanceStatus = DISTANCE_UNKNOWN;
    float batteryVoltage = 0.0;
    float distance = -1;
    unsigned long lastBatteryCheck = 0;
    unsigned long lastDistanceCheck = 0;
    void updateRfid();
    void updateBattery();
    void updateDistance();
    float readBatteryVoltage();
    float readDistance();
};
