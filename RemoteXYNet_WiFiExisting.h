#pragma once

#include <WiFi.h>
#include <RemoteXY.h>
#include "RemoteXYNet_WiFi.h"

class CRemoteXYNet_WiFiExisting : public CRemoteXYNet
{
public:
    CRemoteXYNet_WiFiExisting() : CRemoteXYNet()
    {
    }

    void handler() override
    {
        // Nichts machen.
        // WLAN wird komplett von deiner Connection-Klasse verwaltet.
    }

    uint8_t configured() override
    {
        return WiFi.status() == WL_CONNECTED;
    }

    uint8_t hasInternet() override
    {
        return WiFi.status() == WL_CONNECTED;
    }

    CRemoteXYServer * createServer(uint16_t port) override
    {
        return new CRemoteXYServer_WiFi(this, port);
    }

    CRemoteXYClient * newClient() override
    {
        return new CRemoteXYClient_WiFi();
    }
};
