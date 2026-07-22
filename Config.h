#pragma once

#define FW_VERSION __DATE__ " " __TIME__
#define HW_VERSION "MK-01"


#define PWM_FREQ    100
#define PWM_RES     8


// ===================== MOTOR Rechts
#define M2_FW  14
#define M2_BW  13
#define M2_SPEED  12
#define M2_ENCODER 17
#define M2_PWM_CHANNEL  0
#define M2_WHEEL  210 //mm Umfang
#define M2_PULSE  20 //HIGH pulse pro Umdrehtung


// ===================== MOTOR Links

#define M1_FW   27
#define M1_BW  26
#define M1_SPEED  25
#define M1_ENCODER 16
#define M1_PWM_CHANNEL  1
#define M1_WHEEL  210 //mm Umfang
#define M1_PULSE  20 //HIGH pulse pro Umdrehtung

// ===================== LINE SENSOR
#define LINE_LEFT    33
#define LINE_CENTER  34
#define LINE_RIGHT   35

// ===================== ULTRASONIC
#define US_TRIG  5
#define US_ECHO  4

// ===================== I2C
#define I2C_SDA 21
#define I2C_SCL 22

// ===================== RFID
#define RFID_SS   2
#define RFID_RST  0
#define RFID_SCK  18
#define RFID_MISO 19
#define RFID_MOSI 23

// ===================== LED + SOUND
#define LED_PIN   15
#define LED_COUNT 6

#define ROOF_LED_COUNT 4
#define ROOF_LED_START 0

#define BLINKER_LEFT_LED 4
#define BLINKER_RIGHT_LED 5

#define PIEZO_PIN 32
#define PIEZO_CHANNEL 5

// ===================== BATTERY
#define BATTERY_PIN 39
