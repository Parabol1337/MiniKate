#include "Motor.h"

Motor::Motor(uint8_t fwPin,
             uint8_t bwPin,
             uint8_t pwmPin,
             uint8_t encoderPin,
             uint8_t pwmChannel,
             uint16_t wheelMm,
             uint16_t pulsesPerRev)
: _fwPin(fwPin),
  _bwPin(bwPin),
  _pwmPin(pwmPin),
  _encoderPin(encoderPin),
  _pwmChannel(pwmChannel),
  _wheelMm(wheelMm),
  _pulsesPerRev(pulsesPerRev)
{
}

void Motor::begin() {
  pinMode(_fwPin, OUTPUT);
  pinMode(_bwPin, OUTPUT);
  pinMode(_encoderPin, INPUT_PULLUP);

  ledcSetup(_pwmChannel, 20000, 8);
  ledcAttachPin(_pwmPin, _pwmChannel);

  digitalWrite(_fwPin, LOW);
  digitalWrite(_bwPin, LOW);
  ledcWrite(_pwmChannel, 0);

  _lastUpdateMs = millis();

  attachInterruptArg(
    digitalPinToInterrupt(_encoderPin),
    Motor::encoderISR,
    this,
    RISING
  );
}

void IRAM_ATTR Motor::encoderISR(void* arg) {
  Motor* motor = static_cast<Motor*>(arg);
  motor->onEncoderTick();
}

void IRAM_ATTR Motor::onEncoderTick() {
  _tickCounter++;
  _totalTicks++;
}

void Motor::setTargetSpeed(float mmPerSec)
{
    if (mmPerSec > 0 && mmPerSec < 150) mmPerSec = 150;
    if (mmPerSec < 0 && mmPerSec > -150) mmPerSec = -150;
    _targetSpeed = mmPerSec;
    if (_targetSpeed == 0.0f)
    {
        setStop();
        return;
    }
}

void Motor::setStop() {
  _targetSpeed = 0.0f;
  _rampedTargetSpeed = 0.0f;
  _currentSpeed = 0.0f;
  _integral = 0.0f;
  _pwmValue = 0;
  setDirection(0);
  ledcWrite(_pwmChannel, 0);
}

void Motor::setStartBoostPwm(int pwm)
{
    _startBoostPwm = constrain(pwm, 0, 255);
}

void Motor::update() {
  unsigned long now = millis();
  unsigned long dtMs = now - _lastUpdateMs;
  if (dtMs < 300) return;
  _lastUpdateMs = now;

  noInterrupts();
  long ticks = _tickCounter;
  _tickCounter = 0;
  interrupts();

  float dt = dtMs / 1000.0f;
  float mmPerTick = (float)_wheelMm / (float)_pulsesPerRev;
  _currentSpeed = (ticks * mmPerTick) / dt;

  if (_targetSpeed == 0.0f) {
      setStop();
      return;
  }

  _rampedTargetSpeed = _targetSpeed;
  setDirection(_rampedTargetSpeed);
  float error = abs(_rampedTargetSpeed) - abs(_currentSpeed);
  float correction = (_kp * error) + (_ki * _integral);
  _pwmValue += (int)correction;

  if (abs(_currentSpeed) < _startBoostSpeed)
  {
      _pwmValue = max(_pwmValue, _startBoostPwm);
  }
  else
  {
      _pwmValue = max(_pwmValue, _minPwm);
  }

  _pwmValue = constrain(_pwmValue, 0, 255);
  setPwm(_pwmValue);
}

void Motor::setDirection(float speed)
{
    if (speed > 0)
    {
        digitalWrite(_fwPin, HIGH);
        digitalWrite(_bwPin, LOW);
    }
    else if (speed < 0)
    {
        digitalWrite(_fwPin, LOW);
        digitalWrite(_bwPin, HIGH);
    }
    else
    {
        digitalWrite(_fwPin, LOW);
        digitalWrite(_bwPin, LOW);
    }
}

void Motor::setPwm(int pwm) {
  pwm = constrain(pwm, 0, 255);
  ledcWrite(_pwmChannel, pwm);
}

float Motor::getCurrentSpeed() const {
  return _currentSpeed;
}

float Motor::getTargetSpeed() const {
  return _targetSpeed;
}

int Motor::getPwm() const {
  return _pwmValue;
}

long Motor::getTotalTicks() const {
  return _totalTicks;
}
