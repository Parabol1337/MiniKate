#include "Drive.h"
#include "Perception.h"
#include "Connection.h"
#include "Effects.h"
#include "Interface.h"
#include "Settings.h"

#define DEBUG_ENABLED true
#define DEBUG_PRINT(x)   if (DEBUG_ENABLED) { Serial.print(x); }
#define DEBUG_PRINTLN(x) if (DEBUG_ENABLED) { Serial.println(x); }

Settings settings;
Interface interface;
Drive drive;
Perception perception;
Connection connection;
Effects effects;

unsigned long _lastStatusSend = 0;
bool _RemoteTurn;
DriveStatus _lastDriveStatus = DRIVE_IDLE;
BatteryStatus _lastBatteryStatus = BATTERY_OK;
DistanceStatus _lastDistanceStatus = DISTANCE_UNKNOWN;

void setup()
{
    Serial.begin(115200);
    delay(500);
    DEBUG_PRINTLN("");
    DEBUG_PRINTLN("================================");
    DEBUG_PRINTLN("MiniKate Boot");
    DEBUG_PRINTLN("================================");
    settings.begin();
    effects.begin();
    drive.begin(&settings);
    perception.begin(&settings);
    connection.begin(&settings);
    interface.begin(&settings, &perception);
    drive.setDriveIdle();
    effects.setAnimIdle(true);
    DEBUG_PRINTLN("[SETUP] MiniKate gestartet");
}

void loop()
{
    updateModules();
    sendStatus();
    handleRfid();
    handleSafety();
    handleRemote();
    handleCommands();
    _lastDriveStatus = drive.getStatus();
    _lastBatteryStatus = perception.getBatteryStatus();
    _lastDistanceStatus = perception.getDistanceStatus();
}

void updateModules()
{
    drive.update();
    perception.update();
    connection.update();
    effects.update();
    interface.update();
}

void sendStatus()
{
    if (millis() - _lastStatusSend <= 5000) return;
    _lastStatusSend = millis();
    connection.sendStatus(perception.getBatteryVoltage(), drive.getStatusText(), perception.getBatteryStatusText(), perception.getDistance(), drive.getHeading());
}

void handleRfid()
{
    if (!perception.rfidAvailable()) return;
    String uid = perception.getUid();
    if (drive.getStatus() == DRIVE_LINE) drive.setDriveIdle();
    effects.setOverlayRfid();
    connection.sendRfid(uid);
}

void handleSafety()
{
    DriveStatus driveStatus = drive.getStatus();
    BatteryStatus batteryStatus = perception.getBatteryStatus();
    DistanceStatus distanceStatus = perception.getDistanceStatus();

    if (batteryStatus == BATTERY_LOW && _lastBatteryStatus != BATTERY_LOW)
    {
        DEBUG_PRINTLN("[SAFETY] Akku niedrig");
        effects.setAnimError(false);
        effects.setBlinker(BLINK_BOTH);
    }

    if (batteryStatus == BATTERY_OK && _lastBatteryStatus != BATTERY_OK)
    {
        DEBUG_PRINTLN("[SAFETY] Akku OK");
        effects.setBlinker(BLINK_NONE);
        effects.resetAnim();
    }

    if (driveStatus == DRIVE_LOST && _lastDriveStatus != DRIVE_LOST)
    {
        DEBUG_PRINTLN("[SAFETY] Linie verloren");
        effects.setAnimError(false);
        effects.setBlinker(BLINK_BOTH);
    }

    if (driveStatus == DRIVE_FREE && _lastDriveStatus != DRIVE_FREE)
    {
        DEBUG_PRINTLN("[INFO] Remote aktiv");
    }

    if (distanceStatus == DISTANCE_STOP && _lastDistanceStatus != DISTANCE_STOP)
    {
        DEBUG_PRINTLN("[SAFETY] Hindernis STOP");
        effects.setAnimStop(false);
        effects.setBlinker(BLINK_BOTH);
    }
    else if (distanceStatus == DISTANCE_NEAR && _lastDistanceStatus != DISTANCE_NEAR)
    {
        DEBUG_PRINTLN("[SAFETY] Hindernis NEAR");
        effects.setAnimObstacle(false);
        effects.setBlinker(BLINK_BOTH);
    }
    else if (distanceStatus == DISTANCE_CLEAR && _lastDistanceStatus != DISTANCE_CLEAR)
    {
        DEBUG_PRINTLN("[SAFETY] Abstand wieder frei");
        effects.setBlinker(BLINK_NONE);
        effects.resetAnim();
    }
}

void handleRemote()
{
    if (!connection.isRemoteConnected()) return;
    connection.setRemoteCompas(drive.getHeading());
    if (drive.getStatus() != DRIVE_FREE && drive.getStatus() != DRIVE_GAME) return;

    if (connection.getRemoteBackPressed())
    {
        drive.setDriveBackward(100, 150, true);
        effects.setOverlaySuccess();
        return;
    }

    int remoteSpeed = connection.getRemoteSpeed();
    int remoteTurn = connection.getRemoteTurn();

    if (perception.getDistanceStatus() == DISTANCE_NEAR) remoteSpeed = constrain(remoteSpeed, 0, 150);
    if (perception.getDistanceStatus() == DISTANCE_STOP) remoteSpeed = 0;

    drive.setTargetSpeed(remoteSpeed);
    drive.setTargetTurn(remoteTurn);
}

void handleCommands()
{
    String type, module, action, value;
    if (!connection.read(type, module, action, value)) return;

    DEBUG_PRINT("[CMD] type="); DEBUG_PRINT(type);
    DEBUG_PRINT(" module="); DEBUG_PRINT(module);
    DEBUG_PRINT(" action="); DEBUG_PRINT(action);
    DEBUG_PRINT(" value="); DEBUG_PRINTLN(value);

    if (type != "cmd") return;

    if (module == "drive")
    {
        if (action == "line") { drive.setDriveLine(); effects.setAnimLine(true); }
        else if (action == "free") { effects.setOverlayStart(); drive.setDriveFree(); effects.setAnimFree(true); }
        else if (action == "game") { effects.setOverlayStart(); drive.setDriveGame(); effects.setAnimGame(true); }
        else if (action == "idle") { effects.setOverlayStop(); drive.setDriveIdle(); effects.setAnimIdle(true); }
        return;
    }

    if (module == "remote" && action == "countdown")
    {
        connection.setRemoteCounter(value.toInt());
    }
}
