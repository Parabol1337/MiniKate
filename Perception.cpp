#include "Perception.h"

void Perception::begin(Settings *settings)
{
    this->settings = settings;
    
    SPI.begin(RFID_SCK, RFID_MISO, RFID_MOSI, RFID_SS);
    rfid.PCD_Init();

    analogSetWidth(12);                 // wichtig beim ESP32
    analogReadResolution(12);
    analogSetPinAttenuation(BATTERY_PIN, ADC_0db);

    pinMode(BATTERY_PIN, INPUT);

    pinMode(US_TRIG, OUTPUT);
    pinMode(US_ECHO, INPUT);

    digitalWrite(US_TRIG, LOW);
}

void Perception::update()
{
    updateRfid();
    updateBattery();
    updateDistance();
}

void Perception::updateRfid()
{
    if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial())
    {
        uid = "";

        for (byte i = 0; i < rfid.uid.size; i++)
        {
            if (rfid.uid.uidByte[i] < 0x10)
            {
                uid += "0";
            }

            uid += String(rfid.uid.uidByte[i], HEX);

            if (i < rfid.uid.size - 1)
            {
                uid += ":";
            }
        }

        uid.toUpperCase();

        rfid.PICC_HaltA();
        rfid.PCD_StopCrypto1();

        rfidStatus = RFID_DETECTED;
    }
}

void Perception::updateBattery()
{
    if (millis() - lastBatteryCheck < 2000)
    {
        return;
    }

    lastBatteryCheck = millis();

    batteryVoltage = readBatteryVoltage();

    if (batteryVoltage < 6.5)
    {
        batteryStatus = BATTERY_LOW;
    }
    else
    {
        batteryStatus = BATTERY_OK;
    }
}

void Perception::updateDistance()
{
    if (millis() - lastDistanceCheck < 100)
    {
        return;
    }

    lastDistanceCheck = millis();

    distance = readDistance();

    if (distance < 0)
    {
        distanceStatus = DISTANCE_UNKNOWN;
        return;
    }

    switch (distanceStatus)
    {
        case DISTANCE_STOP:
            if (distance > 7)
            {
                distanceStatus = DISTANCE_NEAR;
            }
            break;

        case DISTANCE_NEAR:
            if (distance < 5)
            {
                distanceStatus = DISTANCE_STOP;
            }
            else if (distance > 18)
            {
                distanceStatus = DISTANCE_CLEAR;
            }
            break;

        case DISTANCE_CLEAR:
        case DISTANCE_UNKNOWN:
        default:
            if (distance < 5)
            {
                distanceStatus = DISTANCE_STOP;
            }
            else if (distance < 15)
            {
                distanceStatus = DISTANCE_NEAR;
            }
            else
            {
                distanceStatus = DISTANCE_CLEAR;
            }
            break;
    }
}

float Perception::readDistance()
{
    digitalWrite(US_TRIG, LOW);
    delayMicroseconds(2);

    digitalWrite(US_TRIG, HIGH);
    delayMicroseconds(10);

    digitalWrite(US_TRIG, LOW);

    long t = pulseIn(US_ECHO, HIGH, 30000);

    if (t == 0)
    {
        return -1;
    }

    return t * 0.0343 / 2;
}

float Perception::readBatteryVoltage()
{
    int raw = getBatteryRaw();
    return raw * settings->getBatteryMultiplier();
}

int Perception::getBatteryRaw()
{
    const int samples = 20;
    long sum = 0;

    for(int i = 0; i < samples; i++)
    {
        sum += analogRead(BATTERY_PIN);
        delayMicroseconds(200);
    }

    int raw = sum / samples;

    if(raw < 50)
    {
        return battraw;
    }

    if(battraw <= 0)
    {
        battraw = raw;
    }
    else
    {
        battraw = (battraw * 9 + raw) / 10;
    }

    return battraw;
}

bool Perception::rfidAvailable()
{
    return rfidStatus == RFID_DETECTED;
}

String Perception::getUid()
{
    rfidStatus = RFID_IDLE;
    return uid;
}

float Perception::getBatteryVoltage()
{
    return batteryVoltage;
}

float Perception::getDistance()
{
    return distance;
}

RfidStatus Perception::getRfidStatus()
{
    return rfidStatus;
}

BatteryStatus Perception::getBatteryStatus()
{
    return batteryStatus;
}

DistanceStatus Perception::getDistanceStatus()
{
    return distanceStatus;
}

String Perception::getRfidStatusText()
{
    switch (rfidStatus)
    {
        case RFID_IDLE: return "idle";
        case RFID_DETECTED: return "detected";
        default: return "unknown";
    }
}

String Perception::getBatteryStatusText()
{
    switch (batteryStatus)
    {
        case BATTERY_OK: return "ok";
        case BATTERY_LOW: return "low";
        default: return "unknown";
    }
}

String Perception::getDistanceStatusText()
{
    switch (distanceStatus)
    {
        case DISTANCE_CLEAR: return "clear";
        case DISTANCE_NEAR: return "near";
        case DISTANCE_STOP: return "stop";
        case DISTANCE_UNKNOWN: return "unknown";
        default: return "unknown";
    }
}
