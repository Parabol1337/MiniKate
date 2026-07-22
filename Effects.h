#pragma once

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "Config.h"

#define PIEZO_CHANNEL 5

enum AnimationMode
{
    ANIM_IDLE,
    ANIM_ERROR,
    ANIM_LINE,
    ANIM_GAME,
    ANIM_FREE,
    ANIM_OBSTACLE,
    ANIM_STOP
};

enum OverlayMode
{
    OVERLAY_NONE,
    OVERLAY_RFID,
    OVERLAY_SUCCESS,
    OVERLAY_BOOT,
    OVERLAY_FLASH,
    OVERLAY_START,
    OVERLAY_STOP,
};

enum BlinkerMode
{
    BLINK_NONE,
    BLINK_LEFT,
    BLINK_RIGHT,
    BLINK_BOTH
};

class Effects
{
public:
    void begin();
    void update();
    void setAnimIdle(bool storeOld = true);
    void setAnimError(bool storeOld = true);
    void setAnimLine(bool storeOld = true);
    void setAnimGame(bool storeOld = true);
    void setAnimFree(bool storeOld = true);
    void setAnimObstacle(bool storeOld = true);
    void setAnimStop(bool storeOld = true);
    void resetAnim();
    void setOverlayRfid();
    void setOverlaySuccess();
    void setOverlayBoot();
    void setOverlayStart();
    void setOverlayStop();
    void setOverlayFlash(uint32_t color, unsigned long duration = 250);
    void setBlinker(BlinkerMode mode);
    void setBeep(int freq, unsigned long duration);
private:
    Adafruit_NeoPixel _pixels = Adafruit_NeoPixel(LED_COUNT, LED_PIN, NEO_RGB + NEO_KHZ800);
    AnimationMode _anim = ANIM_IDLE;
    AnimationMode _oldAnim = ANIM_IDLE;
    uint8_t _animStep = 0;
    unsigned long _animLastUpdate = 0;
    uint8_t _pulseBrightness = 20;
    int8_t _pulseDirection = 5;
    OverlayMode _overlay = OVERLAY_NONE;
    uint8_t _overlayStep = 0;
    unsigned long _overlayLastUpdate = 0;
    unsigned long _overlayUntil = 0;
    uint32_t _overlayFlashColor = 0;
    BlinkerMode _blinker = BLINK_NONE;
    bool _blinkerState = false;
    unsigned long _blinkerLastUpdate = 0;
    unsigned long _beepUntil = 0;
    void updateAnimation();
    void updateOverlay();
    void updateBlinker();
    void updateBeep();
    void render();
    void renderAnimation();
    void renderOverlay();
    void renderBlinker();
    void renderAnimIdle();
    void renderAnimError();
    void renderAnimLine();
    void renderAnimGame();
    void renderAnimFree();
    void renderAnimObstacle();
    void renderAnimStop();
    void renderOverlayRfid();
    void renderOverlaySuccess();
    void renderOverlayBoot();
    void renderOverlayFlash();
    void renderOverlayStart();
    void renderOverlayStop();
    void setAnim(AnimationMode mode, bool storeOld);
    void setRoof(uint32_t color);
    void setRoofPixel(uint8_t index, uint32_t color);
    void setBlinkerLeds(bool left, bool right);
    void clearAll();
    uint32_t colorIdle(uint8_t brightness = 255);
    uint32_t colorError(uint8_t brightness = 255);
    uint32_t colorLine();
    uint32_t colorGame();
    uint32_t colorFree();
    uint32_t colorObstacle();
    uint32_t colorStop();
    uint32_t colorSuccess();
    uint32_t colorRfid();
    uint32_t colorBlinker();
};
