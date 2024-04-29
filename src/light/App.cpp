#include "App.h"


IPAddress g_apIP(192, 168, 1, 220);
IPAddress g_gateway(192, 168, 1, 1);
IPAddress g_subnet(255, 255, 255, 0);
IPAddress g_dns1(192, 168, 1, 1);

ColorHLS g_natural (162, 0, 10);
ColorHLS g_relax (32, 0, 200);

App g_app;
// DNSServer g_dnsServer;

App::App()
    : m_pixels(NUMLED, PIN, NEO_GRB + NEO_KHZ800),
      m_wserver(80),
      m_deg(0),
      m_mode(Modes::Natural),
      m_luminance(0.3f),
      m_power(false),
      m_autoOffSpan(-1),
      m_moveCounter(-1)
{
}

void App::handleNotFoundImpl()
{
    m_wserver.send(200, "text/html", "<html><body>notfound</body></html>");
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
            m_mode = Modes::Natural;
            ModeUpdate ();
            m_wserver.send(200, "text/plain", "OK. mode natural.");
            return;
        }
        else if (param == "relax")
        {
            m_mode = Modes::Relax;
            ModeUpdate ();
            m_wserver.send(200, "text/plain", "OK. mode relax.");
            return;
        }
    }

    m_wserver.send(400, "text/plain", "NG");
    //m_wserver.send(200, "text/plain", String("<html><body>cmd=") + cmd + "<br/>cmd2=" + cmd2 + String ("</body></html>"));
}

void App::ModeUpdate ()
{
    if (m_power)
    {
        ColorHLS hls;
        switch (m_mode)
        {
        case Modes::Natural:
            hls = g_natural;
            Serial.println("Modes::Natural");
            break;
        
        case Modes::Relax:
            hls = g_relax;
            Serial.println("Modes::relax");
            break;
        }
        hls.l = (uint8_t)(m_luminance * 255);
        ColorRGB rgb (hls);

        for (int i = 0; i < NUMLED; i++)
        {
            m_nextColors[i] = rgb;
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

    ConnectWiFi();

    m_wserver.onNotFound(&handleNotFound);
    m_wserver.on("/api", HTTP_POST, &handleAPI);
    m_wserver.begin();

    for (int i = 0; i < NUMLED; i++)
    {
        m_lastColors[i] = ColorRGB (0, 0, 0);
        m_nextColors[i] = ColorRGB (0, 0, 0);
    }

/*
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(g_apIP, g_apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP("test-AP");
  g_dnsServer.start(53, "*", g_apIP);
*/
}

void App::ConnectWiFi()
{
#ifdef ENABLE_WIFI
    if (!WiFi.config(g_apIP, g_gateway, g_subnet, g_dns1))
        Serial.println("Failed to configure!");

    WiFi.begin("Skynet2020-g", "718B81317279F");
    while (WiFi.status() != WL_CONNECTED)
    {
        LightConnecting ();
        Serial.print(".");
    }

    Serial.println("");
    Serial.print("Connected to ");
    Serial.println("Skynet2020-g");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    LightConneced ();

#endif // ENABLE_WIFI
}

void App::CheckWiFi ()
{
#ifdef ENABLE_WIFI
    if (WiFi.status() != WL_CONNECTED)
    {
        WiFi.reconnect ();
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
    CheckWiFi ();

#ifdef ENABLE_WIFI
//  g_dnsServer.processNextRequest();
    m_wserver.handleClient();
#endif

    LightUpdate ();

    delay(10);
}
