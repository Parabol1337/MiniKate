#pragma once

#include <Arduino.h>
#include <WebServer.h>
#include <Update.h>
#include "Settings.h"
#include "Perception.h"

class Interface
{
public:
    void begin(Settings *settings, Perception *perception);
    void update();
private:
    WebServer server{80};
    Settings *settings = nullptr;
    Perception *perception = nullptr;
    void handleIndex();
    void handleConfig();
    void handleConfigSave();
    void handleUpload();
    void handleUploadDone();
    String htmlHeader(String title);
    String htmlFooter();
};
