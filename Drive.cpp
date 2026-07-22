#include "Drive.h"

Drive::Drive()
: _motorLeft(
      M2_FW,
      M2_BW,
      M2_SPEED,
      M2_ENCODER,
      M2_PWM_CHANNEL,
      M2_WHEEL,
      M2_PULSE
  ),
  _motorRight(
      M1_FW,
      M1_BW,
      M1_SPEED,
      M1_ENCODER,
      M1_PWM_CHANNEL,
      M1_WHEEL,
      M1_PULSE
  )
{
}

void Drive::begin(Settings* settings)
{
    _settings = settings;

    pinMode(LINE_LEFT, INPUT);
    pinMode(LINE_CENTER, INPUT);
    pinMode(LINE_RIGHT, INPUT);

    Wire.begin(I2C_SDA, I2C_SCL);
    Wire.setClock(100000);

    _imu.softReset();
    delay(100);
    _imu.I2cInit(0x69);

    calibrateImu();
    resetHeading();
    _lastImuUpdate = millis();

    _motorLeft.begin();
    _motorRight.begin();

    _motorLeft.setStartBoostPwm(_settings->getMotorLeftLimit());
    _motorRight.setStartBoostPwm(_settings->getMotorRightLimit());

    stopMotors();
}

void Drive::update()
{
    updateImu();
    _motorLeft.update();
    _motorRight.update();

    switch (_status)
    {
        case DRIVE_IDLE: updateIdle(); break;
        case DRIVE_LINE: updateLine(); break;
        case DRIVE_LOST: updateLost(); break;
        case DRIVE_FREE: updateFree(); break;
        case DRIVE_GAME: updateFree(); break;
        case DRIVE_TURNING: updateTurning(); break;
        case DRIVE_FORWARD: updateForward(); break;
        case DRIVE_BACKWARD: updateBackward(); break;
    }
}

void Drive::updateIdle()
{
    stopMotors();
}

void Drive::updateLine()
{
    // TODO: später Linienfolger
}

void Drive::updateLost()
{
    stopMotors();
}

void Drive::updateFree()
{
    _targetSpeed = constrain(_targetSpeed, -350.0f, 350.0f);
    _targetTurn  = constrain(_targetTurn, -100.0f, 100.0f);

    if (_targetSpeed == 0)
    {
        stopMotors();
        return;
    }

    float turn = _targetTurn / 100.0f;
    float leftSpeed  = _targetSpeed;
    float rightSpeed = _targetSpeed;

    if (turn < 0)
    {
        leftSpeed = _targetSpeed * (1.0f + turn);
    }
    else if (turn > 0)
    {
        rightSpeed = _targetSpeed * (1.0f - turn);
    }

    _motorLeft.setTargetSpeed(leftSpeed);
    _motorRight.setTargetSpeed(rightSpeed);
}

void Drive::updateTurning()
{
    float current = normalizeAngle(_currentHeading);
    float error = normalizeAngle(_targetHeading - current);

    if (abs(error) <= _turnTolerance)
    {
        finishAction();
        return;
    }

    float turnSpeed = error * _turnKp;
    turnSpeed = constrain(turnSpeed, -_turnMaxSpeed, _turnMaxSpeed);
    _motorLeft.setTargetSpeed(-turnSpeed);
    _motorRight.setTargetSpeed(turnSpeed);
}

void Drive::updateForward()
{
    if (getAverageMoveTicks() >= _moveTargetTicks)
    {
        finishAction();
        return;
    }

    _motorLeft.setTargetSpeed(_targetSpeed);
    _motorRight.setTargetSpeed(_targetSpeed);
}

void Drive::updateBackward()
{
    if (getAverageMoveTicks() >= _moveTargetTicks)
    {
        finishAction();
        return;
    }

    _motorLeft.setTargetSpeed(-_targetSpeed);
    _motorRight.setTargetSpeed(-_targetSpeed);
}

void Drive::setDriveIdle()
{
    stopMotors();
    setTargetSpeed(0);
    setTargetTurn(0);
    setStatus(DRIVE_IDLE);
}

void Drive::setDriveLine()
{
    setStatus(DRIVE_LINE);
}

void Drive::setDriveFree()
{
    _targetHeading = 0.0f;
    setStatus(DRIVE_FREE);
}

void Drive::setDriveGame()
{
    _targetHeading = 0.0f;
    setStatus(DRIVE_GAME);
}

void Drive::setDriveTurn(float speed, long distanceMm, bool left, bool return2status)
{
    _nextStatus = return2status ? _status : DRIVE_IDLE;
    _targetSpeed = abs(speed);
    _moveTargetLeft = left;
    _moveTargetTicks = mmToTicks(distanceMm);
    _moveStartLeftTicks = _motorLeft.getTotalTicks();
    _moveStartRightTicks = _motorRight.getTotalTicks();
    setStatus(DRIVE_TURNING);
}

void Drive::setDriveForward(float speed, long distanceMm, bool return2status)
{
    _nextStatus = return2status ? _status : DRIVE_IDLE;
    _targetSpeed = abs(speed);
    _moveTargetTicks = mmToTicks(distanceMm);
    _moveStartLeftTicks = _motorLeft.getTotalTicks();
    _moveStartRightTicks = _motorRight.getTotalTicks();
    setStatus(DRIVE_FORWARD);
}

void Drive::setDriveBackward(float speed, long distanceMm, bool return2status)
{
    _nextStatus = return2status ? _status : DRIVE_IDLE;
    _targetSpeed = abs(speed);
    _moveTargetTicks = mmToTicks(distanceMm);
    _moveStartLeftTicks = _motorLeft.getTotalTicks();
    _moveStartRightTicks = _motorRight.getTotalTicks();
    setStatus(DRIVE_BACKWARD);
}

void Drive::setTargetHeading(float heading)
{
    _targetHeading = normalizeAngle(heading);
}

void Drive::setTargetSpeed(float speed)
{
    _targetSpeed = speed;
}

void Drive::setTargetTurn(float turn)
{
    _targetTurn = turn;
}

DriveStatus Drive::getStatus() const
{
    return _status;
}

String Drive::getStatusText() const
{
    switch (_status)
    {
        case DRIVE_IDLE: return "idle";
        case DRIVE_LINE: return "line";
        case DRIVE_LOST: return "lost";
        case DRIVE_FREE: return "free";
        case DRIVE_GAME: return "game";
        case DRIVE_TURNING: return "turn";
        case DRIVE_FORWARD: return "forward";
        case DRIVE_BACKWARD: return "backward";
        default: return "unknown";
    }
}

float Drive::getHeading() const
{
    return normalizeAngle(_currentHeading);
}

void Drive::calibrateImu()
{
    const int samples = 500;
    float sumZ = 0.0f;
    int16_t accelGyro[6] = {0};

    for (int i = 0; i < samples; i++)
    {
        _imu.getAccelGyroData(accelGyro);
        sumZ += accelGyro[5];
        delay(3);
    }

    _gyroZOffset = sumZ / samples;
}

void Drive::updateImu()
{
    unsigned long now = millis();

    if (_lastImuUpdate == 0)
    {
        _lastImuUpdate = now;
        return;
    }

    float dt = (now - _lastImuUpdate) / 1000.0f;
    _lastImuUpdate = now;

    int16_t accelGyro[6] = {0};
    _imu.getAccelGyroData(accelGyro);

    float rawGyroZ = accelGyro[5] - _gyroZOffset;
    float gyroZDegPerSec = rawGyroZ / 16.4f;
    _currentHeading = normalizeAngle(_currentHeading + gyroZDegPerSec * dt);
}

void Drive::resetHeading()
{
    _currentHeading = 0.0f;
    _mpuTargetHeading = 0.0f;
    _lastImuUpdate = millis();
}

void Drive::stopMotors()
{
    _motorLeft.setStop();
    _motorRight.setStop();
}

void Drive::setStatus(DriveStatus newStatus)
{
    _status = newStatus;
}

void Drive::finishAction()
{
    stopMotors();
    _status = _nextStatus;
}

float Drive::normalizeAngle(float angle) const
{
    while (angle > 180.0f) angle -= 360.0f;
    while (angle < -180.0f) angle += 360.0f;
    return angle;
}

long Drive::mmToTicks(long mm) const
{
    float mmPerTick = (float)M1_WHEEL / (float)M1_PULSE;
    return abs(mm) / mmPerTick;
}

long Drive::getAverageMoveTicks() const
{
    long left = abs(_motorLeft.getTotalTicks() - _moveStartLeftTicks);
    long right = abs(_motorRight.getTotalTicks() - _moveStartRightTicks);
    return (left + right) / 2;
}
