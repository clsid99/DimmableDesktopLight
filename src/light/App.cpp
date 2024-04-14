#include "App.h"

#define PIN 14    // LED
#define NUMLED 32 // LED

IPAddress g_apIP(192, 168, 1, 220);
IPAddress g_gateway(192, 168, 1, 1);
IPAddress g_subnet(255, 255, 255, 0);
IPAddress g_dns1(192, 168, 1, 1);

App g_app;

// DNSServer g_dnsServer;

App::App()
    : m_pixels(NUMLED, PIN, NEO_GRB + NEO_KHZ800),
      m_wserver(80),
      m_deg(0)
{
}

void App::handleNotFoundImpl()
{
    m_wserver.send(200, "text/html", "<html><body>notfound</body></html>");
}

void App::handleAPIImpl()
{
    const String &cmd = m_wserver.arg("cmd");
    const String &cmd2 = m_wserver.arg("cmd2");

    m_wserver.sendHeader ("Location", String("/"), true);
    m_wserver.send(200, "text/plain", String("<html><body>cmd=") + cmd + "<br/>cmd2=" + cmd2 + String ("</body></html>"));
}


void App::Initialize()
{
    Serial.begin(115200);
    Serial.println("*** started");

    m_pixels.begin();
    m_pixels.show();

    ConnectWiFi();

    m_wserver.onNotFound(&handleNotFound);
    m_wserver.on("/api", HTTP_POST, &handleAPI);
    m_wserver.begin();


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
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.print("Connected to ");
    Serial.println("Skynet2020-g");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
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
            delay(500);
        }
    }
#endif // ENABLE_WIFI
}


void App::DoEvents()
{
    CheckWiFi ();

    for (int i = 0; i < NUMLED; i++)
    {
        short deg = (m_deg + i * 8) % 360;
        short l = abs(deg - 128) * 2;
        l = l < 50 ? l : 50;
        ColorHLS hls(deg, 50 /*- l*/, 200);
        ColorRGB rgb(hls);

        m_pixels.setPixelColor(i, rgb.r, rgb.g, rgb.b);
    }
    m_pixels.show();

    m_deg = (m_deg + 4) % 360;

#ifdef ENABLE_WIFI
//  g_dnsServer.processNextRequest();
    m_wserver.handleClient();
#endif

    delay(10);
}
