#pragma once
#include <Arduino.h>

class Motor {
public:
  Motor(uint8_t fwPin, uint8_t bwPin, uint8_t pwmPin, uint8_t encoderPin, uint8_t pwmChannel, uint16_t wheelMm, uint16_t pulsesPerRev);
  void begin();
  void setTargetSpeed(float mmPerSec);
  void setStop();
  void update();
  void setStartBoostPwm(int pwm);
  float getCurrentSpeed() const;
  float getTargetSpeed() const;
  int getPwm() const;
  long getTotalTicks() const;
private:
  uint8_t _fwPin;
  uint8_t _bwPin;
  uint8_t _pwmPin;
  uint8_t _encoderPin;
  uint8_t _pwmChannel;
  uint16_t _wheelMm;
  uint16_t _pulsesPerRev;
  volatile long _tickCounter = 0;
  volatile long _totalTicks = 0;
  float _targetSpeed = 0.0f;
  float _currentSpeed = 0.0f;
  int _pwmValue = 0;
  int _startBoostPwm = 200;
  int _minPwm = 70;
  float _startBoostSpeed = 20.0f;
  unsigned long _lastUpdateMs = 0;
  float _rampedTargetSpeed = 0.0f;
  float _accelMmPerSec2 = 600.0f;
  float _kp = 0.10f;
  float _ki = 0.01f;
  float _integral = 0.0f;
  static void IRAM_ATTR encoderISR(void* arg);
  void IRAM_ATTR onEncoderTick();
  void setPwm(int pwm);
  void setDirection(float speed);
};
