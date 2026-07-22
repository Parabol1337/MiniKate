#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <DFRobot_BMI160.h>

#include "Motor.h"
#include "Config.h"
#include "Settings.h"

enum DriveStatus
{
    DRIVE_IDLE,
    DRIVE_LINE,
    DRIVE_LOST,
    DRIVE_FREE,
    DRIVE_GAME,
    DRIVE_TURNING,
    DRIVE_FORWARD,
    DRIVE_BACKWARD
};

class Drive
{
public:
    Drive();
    void begin(Settings* settings);
    void update();
    void setDriveIdle();
    void setDriveLine();
    void setDriveFree();
    void setDriveGame();
    void setDriveTurn(float speed, long distanceMm,bool left=false , bool return2status = false);
    void setDriveForward(float speed, long distanceMm, bool return2status = false);
    void setDriveBackward(float speed, long distanceMm, bool return2status = false);
    void setTargetHeading(float heading);
    void setTargetSpeed(float speed);
    void setTargetTurn(float turn);
    void resetHeading();
    DriveStatus getStatus() const;
    String getStatusText() const;
    float getHeading() const;
private:
    Settings* _settings = nullptr;
    DFRobot_BMI160 _imu;
    float _gyroZOffset = 0.0f;
    unsigned long _lastImuUpdate = 0;
    void updateImu();
    void calibrateImu();
    Motor _motorLeft;
    Motor _motorRight;
    DriveStatus _status = DRIVE_IDLE;
    DriveStatus _nextStatus = DRIVE_IDLE;
    float _currentHeading = 0.0f;
    float _mpuTargetHeading = 0.0f;
    float _targetSpeed = 0.0f;
    float _targetHeading = 0.0f;
    float _targetTurn = 0.0f;
    float _freeKpHeading = 0.8f;
    float _turnKp = 2.0f;
    float _turnMaxSpeed = 250.0f;
    float _turnTolerance = 3.0f;
    bool _moveTargetLeft = false;
    long _moveTargetTicks = 0;
    long _moveStartLeftTicks = 0;
    long _moveStartRightTicks = 0;
    void updateIdle();
    void updateLine();
    void updateLost();
    void updateFree();
    void updateTurning();
    void updateForward();
    void updateBackward();
    void stopMotors();
    void setStatus(DriveStatus newStatus);
    void finishAction();
    float normalizeAngle(float angle) const;
    long mmToTicks(long mm) const;
    long getAverageMoveTicks() const;
};
