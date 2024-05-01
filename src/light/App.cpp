#include "App.h"
#include <EEPROM.h>

IPAddress g_apIP(192, 168, 1, 220);
IPAddress g_gateway(192, 168, 1, 1);
IPAddress g_subnet(255, 255, 255, 0);
IPAddress g_dns1(192, 168, 1, 1);

// A2 00 0A
//ColorHLS g_natural (162, 0, 10);
// 20 00 C8
//ColorHLS g_relax (32, 0, 200);

App g_app;

App::App()
    : m_pixels(NUMLED, PIN, NEO_GRB + NEO_KHZ800),
      m_wserver(80),
      m_deg(0),
      m_power(false),
      m_autoOffSpan(-1),
      m_moveCounter(-1),
      m_prev_time (0)
{
    m_settings.mode = Modes::Natural;
    m_settings.luminance = 0.3f;
    m_settings.natural = ColorHLS (162, 0, 10);
    m_settings.relax = ColorHLS (32, 0, 200);
}

void App::handleNotFoundImpl()
{
    const char *body = "<html><body style=\"font-size=20px;\">"
    "<a href=\"http://192.168.1.220/api?cmd=power&param=on\">ON</a><br/>"
    "<a href=\"http://192.168.1.220/api?cmd=power&param=off\">OFF</a><br/>"
    "<a href=\"http://192.168.1.220/api?cmd=mode&param=natural\">natural</a><br/>"
    "<a href=\"http://192.168.1.220/api?cmd=mode&param=relax\">relax</a><br/>"
    "</body></html>";
    m_wserver.send(200, "text/html", body);
}

int XXtoInt (const char *p)
{
    char buff[2 + 1];
    buff[0] = *p;
    buff[1] = *(p + 1);
    buff[2] = 0;
    return strtoll (buff, nullptr, 16);
}

void App::handleAPIImpl()
{
    const String &cmd = m_wserver.arg("cmd");
    const String &param = m_wserver.arg("param");

    if (cmd == "power")
    {
        if (param == "on")
        {
            m_power = true;
            ModeUpdate ();
            m_wserver.send(200, "text/plain", "OK. power on.");
            return;
        }
        else if (param == "off")
        {
            m_power = false;
            ModeUpdate ();
            m_wserver.send(200, "text/plain", "OK. power off.");
            return;
        }
    }
    else if (cmd == "mode")
    {
        if (param == "natural")
        {
            m_power = true;
            m_settings.mode = Modes::Natural;
            SaveSettings ();
            ModeUpdate ();
            m_wserver.send(200, "text/plain", "OK. mode natural.");
            return;
        }
        else if (param == "relax")
        {
            m_power = true;
            m_settings.mode = Modes::Relax;
            SaveSettings ();
            ModeUpdate ();
            m_wserver.send(200, "text/plain", "OK. mode relax.");
            return;
        }
    }
    else if (cmd == "luminance")
    {
        double lum = atof (param.c_str ());
        m_power = true;
        m_settings.luminance = (float)(lum / 100.0);
        SaveSettings ();
        ModeUpdate ();
        m_wserver.send(200, "text/plain", "OK. luminance.");
        return;
    }
    else if (cmd == "manual")
    {
        int paramLen = (int)param.length ();
        for (int i = 0; i < NUMLED; i++)
        {
            uint8_t r = 0, g = 0, b = 0;
            if (i * 6 + 5 < paramLen)
            {
                const char *s = param.c_str () + i * 6;
                r = (uint8_t)XXtoInt (s + 0);
                g = (uint8_t)XXtoInt (s + 2);
                b = (uint8_t)XXtoInt (s + 4);
            }
            m_manualColors[i] = ColorRGB (r, g, b);
        }
        m_power = true;
        m_settings.mode = Modes::Manual;
        ModeUpdate ();
        m_wserver.send(200, "text/plain", "OK. manual colors.");
        return;
    }
    else if (cmd == "natural")
    {
        int paramLen = (int)param.length ();
        if (paramLen == 6)
        {
            const char *str = param.c_str ();
            uint8_t h = (uint8_t)XXtoInt (str + 0);
            uint8_t l = (uint8_t)XXtoInt (str + 2);
            uint8_t s = (uint8_t)XXtoInt (str + 4);
            m_settings.natural = ColorHLS (h, l, s);
            SaveSettings ();
            ModeUpdate ();
            m_wserver.send(200, "text/plain", "OK. set natural color.");
            return;
        }
    }
    else if (cmd == "relax")
    {
        int paramLen = (int)param.length ();
        if (paramLen == 6)
        {
            const char *str = param.c_str ();
            uint8_t h = (uint8_t)XXtoInt (str + 0);
            uint8_t l = (uint8_t)XXtoInt (str + 2);
            uint8_t s = (uint8_t)XXtoInt (str + 4);
            m_settings.relax = ColorHLS (h, l, s);
            SaveSettings ();
            ModeUpdate ();
            m_wserver.send(200, "text/plain", "OK. set relax color.");
            return;
        }
    }
    else if (cmd == "autooff")
    {
        int time = atoi (param.c_str ());
        if (time >= 0)
            m_autoOffSpan = time;
        else
            m_autoOffSpan = -1;
        m_wserver.send(200, "text/plain", "OK. set auto off timer.");
        return;
    }

    m_wserver.send(400, "text/plain", "NG");
}

void App::ModeUpdate ()
{
    if (m_power)
    {
        ColorHLS hls;
        switch (m_settings.mode)
        {
        case Modes::Natural:
            hls = m_settings.natural;
            break;
        
        case Modes::Relax:
            hls = m_settings.relax;
            break;

        case Modes::Manual:
            {
                for (int i = 0; i < NUMLED; i++)
                {
                    ColorHLS hls (m_manualColors[i]);
                    hls.l = (uint8_t)(hls.l * m_settings.luminance);
                    m_nextColors[i] = ColorRGB (hls);
                }
            }
            break;
        }

        if (m_settings.mode == Modes::Natural
            || m_settings.mode == Modes::Relax)
        {
            hls.l = (uint8_t)(m_settings.luminance * 255);
            ColorRGB rgb (hls);
            for (int i = 0; i < NUMLED; i++)
            {
                m_nextColors[i] = rgb;
            }
        }

        m_moveCounter = COUNTER_START;
    }
    else
    {
        for (int i = 0; i < NUMLED; i++)
        {
            m_nextColors[i] = ColorRGB (0, 0, 0);
        }
        m_moveCounter = COUNTER_START;
    }
}

void App::Initialize()
{
    Serial.begin(115200);
    Serial.println("*** started");

    m_pixels.begin();
    for (int i = 0; i < NUMLED; i++)
        m_pixels.setPixelColor (i, 0, 0, 0);
    m_pixels.show();

    LoadSettings ();

    ConnectWiFi();

    m_wserver.onNotFound(&handleNotFound);
    m_wserver.on("/api", HTTP_GET, &handleAPI);
    m_wserver.begin();

    for (int i = 0; i < NUMLED; i++)
    {
        m_lastColors[i] = ColorRGB (0, 0, 0);
        m_nextColors[i] = ColorRGB (0, 0, 0);
    }
}

void App::ConnectWiFi()
{
#ifdef ENABLE_WIFI
    if (!WiFi.config(g_apIP, g_gateway, g_subnet, g_dns1))
        Serial.println("Failed to configure!");

    WiFi.begin("Skynet2020-g", "718B81317279F");

    CheckWiFi (true);

    Serial.println("");
    Serial.print("Connected to ");
    Serial.println("Skynet2020-g");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    LightConneced ();

#endif // ENABLE_WIFI
}

void App::CheckWiFi (boolean init)
{
#ifdef ENABLE_WIFI
    if (WiFi.status() != WL_CONNECTED)
    {
        if (!init)
        {
            WiFi.reconnect ();
        }

        while (WiFi.status() != WL_CONNECTED)
        {
            LightConnecting ();
        }

        LightConneced ();
    }
#endif // ENABLE_WIFI
}

void App::LightConnecting ()
{
    for (int i = 0; i < NUMLED; i++)
    {
        short deg = (m_deg + i * 8) % 360;
        short l = abs(deg - 128) * 2;
        l = l < 50 ? l : 50;
        // orange
        ColorHLS hls(20, 50 - l, 200);
        ColorRGB rgb(hls);

        m_pixels.setPixelColor(i, rgb.r, rgb.g, rgb.b);
    }
    m_deg = (m_deg + 1) % 360;
    m_pixels.show();
    delay(10);
}

void App::LightConneced ()
{
    for (int i = 0; i < NUMLED; i++)
        m_pixels.setPixelColor (i, 0, 0, 0);
    m_pixels.show();

    for (int l = 0; l < 50; l++)
    {
        for (int i = 0; i < NUMLED; i++)
        {
            ColorHLS hls(150, l, 200);
            ColorRGB rgb(hls);
            m_pixels.setPixelColor(i, rgb.r, rgb.g, rgb.b);
        }
        m_pixels.show();
        delay(50);
    }
    for (int l = 0; l < 50; l++)
    {
        for (int i = 0; i < NUMLED; i++)
        {
            ColorHLS hls(150, 50 - l, 200);
            ColorRGB rgb(hls);
            m_pixels.setPixelColor(i, rgb.r, rgb.g, rgb.b);
        }
        m_pixels.show();
        delay(50);
    }
    for (int i = 0; i < NUMLED; i++)
        m_pixels.setPixelColor (i, 0, 0, 0);
    m_pixels.show();
}

void App::LightUpdate ()
{
    if (m_moveCounter < 0)
        return;

    for (int i = 0; i < NUMLED; i++)
    {
        double ratioS = (double)m_moveCounter / COUNTER_START;
        double ratioE = 1 - ratioS;

        double lr = (double)m_nextColors[i].r * ratioE + (double)m_lastColors[i].r * ratioS;
        double lg = (double)m_nextColors[i].g * ratioE + (double)m_lastColors[i].g * ratioS;
        double lb = (double)m_nextColors[i].b * ratioE + (double)m_lastColors[i].b * ratioS;

        uint8_t nr = (uint8_t)lr;
        uint8_t ng = (uint8_t)lg;
        uint8_t nb = (uint8_t)lb;
        
        m_pixels.setPixelColor(i, nr, ng, nb);
        m_lastColors[i] = ColorRGB (nr, ng, nb);
    }
    m_pixels.show();
    m_moveCounter--;
}

void App::DoEvents()
{
    yield ();
    m_time = ::time (nullptr);

    CheckWiFi ();

#ifdef ENABLE_WIFI
//  g_dnsServer.processNextRequest();
    m_wserver.handleClient();
#endif

    CheckAutoOff ();
    LightUpdate ();

    delay(10);
}

void App::LoadSettings ()
{
    Settings settings;
    EEPROM.begin (sizeof (Settings));
    EEPROM.get<Settings>(0, settings);
    if (settings.configMagic == CONFIG_MAGIC)
    {
        m_settings = settings;
    }
    else
    {
        SaveSettings ();
    }
}

void App::SaveSettings ()
{
    m_settings.configMagic = CONFIG_MAGIC;
    EEPROM.put<Settings> (0, m_settings);
    EEPROM.commit ();
}

void App::CheckAutoOff ()
{
    if (m_autoOffSpan >= 0)
    {
        if (m_time - m_prev_time > 0)
        {
            if (m_autoOffSpan == 0)
            {
                m_power = false;
                ModeUpdate ();
            }
            m_prev_time = m_time;
            m_autoOffSpan--;
        }

    }
}
