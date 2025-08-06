#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen.h>

// ====== ซ่อนข้อมูลสำคัญ ======
const char* ssid     = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
// ============================

// TFT and Touch Pins
#define TFT_CS    15
#define TFT_DC    2
#define TFT_RST   4
#define TOUCH_CS  21
#define LED_PIN   32

Adafruit_ILI9341 tft(TFT_CS, TFT_DC, TFT_RST);
XPT2046_Touchscreen ts(TOUCH_CS);
WebServer server(80);

bool ledState = false;

void connectWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println("\nConnected! IP: " + WiFi.localIP().toString());
}

void drawButton(bool state) {
  uint16_t color = state ? ILI9341_GREEN : ILI9341_RED;
  tft.fillRect(60, 100, 120, 60, color);
  tft.drawRect(60, 100, 120, 60, ILI9341_WHITE);

  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.setCursor(80, 120);
  tft.println(state ? "LED ON " : "LED OFF");
}

void toggleLED(bool state) {
  ledState = state;
  digitalWrite(LED_PIN, ledState ? HIGH : LOW);
  drawButton(ledState);
}

void handleRoot() {
  String html = R"rawliteral(
    <html><head>
    <meta name='viewport' content='width=device-width, initial-scale=1.0'>
    <style>button {width: 150px; height: 60px; font-size: 20px;}</style>
    </head><body>
    <h2>ESP32 LED Control</h2>
    <p>Status: <span id="status">%STATE%</span></p>
    <button onclick="toggleLED()">Toggle LED</button>
    <script>
      function toggleLED() {
        fetch("/toggle").then(() => setTimeout(() => location.reload(), 300));
      }
    </script>
    </body></html>
  )rawliteral";

  html.replace("%STATE%", ledState ? "ON" : "OFF");
  server.send(200, "text/html", html);
}

void handleToggle() {
  toggleLED(!ledState);
  server.send(200, "text/plain", ledState ? "ON" : "OFF");
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(ILI9341_BLACK);
  ts.begin();
  ts.setRotation(1);
  drawButton(ledState);

  connectWiFi();

  server.on("/", handleRoot);
  server.on("/toggle", handleToggle);
  server.begin();
}

void loop() {
  server.handleClient();

  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    int16_t touchX = map(p.x, 200, 3800, 0, tft.width());
    int16_t touchY = map(p.y, 200, 3800, 0, tft.height());

    if (touchX > 60 && touchX < 180 && touchY > 100 && touchY < 160) {
      toggleLED(!ledState);
      delay(300);
    }
  }
}
