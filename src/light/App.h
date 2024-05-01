#ifndef APP_H
#define APP_H

#define ENABLE_WIFI
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "Color.h"

#define PIN 14    // LED
#define NUMLED 32 // LED
#define COUNTER_START 100
#define CONFIG_MAGIC 0x12190001

class App;
extern App g_app;

class App
{
private:
    /// @brief モード列挙
    enum Modes
    {
        Natural,
        Relax,
        Manual
    };
    struct Settings
    {
        long configMagic;
        /// @brief モード
        Modes mode;

        /// @brief 輝度
        float luminance;
        ColorHLS natural;
        ColorHLS relax;
    };

private:
    /// @brief 
    Settings m_settings;

    /// @brief ON/OFF
    boolean m_power;

    /// @brief 自動OFFまでの秒数
    int32_t m_autoOffSpan;

    time_t m_time;
    time_t m_prev_time;

private:
    Adafruit_NeoPixel m_pixels;
    ESP8266WebServer m_wserver;
    int m_deg;
    ColorRGB m_lastColors[NUMLED];
    ColorRGB m_nextColors[NUMLED];
    ColorRGB m_manualColors[NUMLED];
    int m_moveCounter;

public:
    App();

    void Initialize();
    void DoEvents();

    static void handleNotFound() {g_app.handleNotFoundImpl ();}
    static void handleAPI() {g_app.handleAPIImpl ();}

private:
    void ConnectWiFi();
    void CheckWiFi(boolean init = false);
    void handleNotFoundImpl();
    void handleAPIImpl();

private: 
    void LightConnecting ();
    void LightConneced ();
    void LightUpdate ();
    void ModeUpdate ();
    void CheckAutoOff ();

private:
    void LoadSettings ();
    void SaveSettings ();
};



#endif // APP_H