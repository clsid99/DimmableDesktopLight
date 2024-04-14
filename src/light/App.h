#ifndef APP_H
#define APP_H

#define ENABLE_WIFI
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "Color.h"

// #include <DNSServer.h>

class App;
extern App g_app;

class App
{
private:
    Adafruit_NeoPixel m_pixels;
    ESP8266WebServer m_wserver;
    int m_deg;

public:
    App();

    void Initialize();
    void DoEvents();

    static void handleNotFound() {g_app.handleNotFoundImpl ();}
    static void handleAPI() {g_app.handleAPIImpl ();}

private:
    void ConnectWiFi();
    void CheckWiFi();
    void handleNotFoundImpl();
    void handleAPIImpl();
};



#endif // APP_H