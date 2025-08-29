#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const char* ssid = "OPPO A31";
const char* password = "SHANTANU";

const int TDS_PIN = A0;
const float VREF = 3.3;
const float ADC_RESOLUTION = 1024.0;

AsyncWebServer server(80);
float tdsValue = 0;
String quality = "Unknown";

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <title>TDS Gauge</title>
  <script src="https://cdn.plot.ly/plotly-2.30.0.min.js"></script>
  <style>
    body { text-align: center; font-family: Arial; background: #f0f0f0; }
    #chart { width: 400px; height: 300px; margin: auto; }
    #value { font-size: 20px; margin-top: 20px; }
  </style>
</head>
<body>
  <h2>Water Quality TDS Meter</h2>
  <div id="chart"></div>
  <p id="value">Loading...</p>

  <script>
    function drawGauge(tds) {
      var data = [{
        type: "indicator",
        mode: "gauge+number",
        value: tds,
        title: { text: "TDS (ppm)", font: { size: 24 } },
        gauge: {
          axis: { range: [null, 1200], tickwidth: 1, tickcolor: "darkgray" },
          bar: { color: tds <= 250 ? "green" : tds <= 500 ? "orange" : "red" },
          steps: [
            { range: [0, 250], color: "#d4edda" },
            { range: [250, 500], color: "#fff3cd" },
            { range: [500, 1200], color: "#f8d7da" }
          ]
        }
      }];

      var layout = {
        width: 400,
        height: 300,
        margin: { t: 50, r: 25, l: 25, b: 25 },
        paper_bgcolor: "#f0f0f0",
        font: { color: "black", family: "Arial" }
      };

      Plotly.newPlot("chart", data, layout);
      document.getElementById("value").innerText = "TDS: " + tds + " ppm";
    }

    setInterval(() => {
      fetch("/tds")
        .then(response => response.json())
        .then(data => {
          drawGauge(data.tds);
        });
    }, 2000);
  </script>
</body>
</html>
)rawliteral";

void setup() {
  Serial.begin(9600);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("OLED failed"));
    while (true);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Connecting WiFi...");
  display.display();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting...");
  }

  Serial.println("Connected: " + WiFi.localIP().toString());

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("WiFi Connected!");
  display.setCursor(0, 10);
  display.print("IP: ");
  display.println(WiFi.localIP());
  display.display();
  delay(2000);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  server.on("/tds", HTTP_GET, [](AsyncWebServerRequest *request){
    String json = "{\"tds\":" + String((int)tdsValue) + "}";
    request->send(200, "application/json", json);
  });

  // Handle favicon request
  server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request){
  static const char blankFavicon[] = {
    0x00, 0x00, 0x01, 0x00, 0x01, 0x00,
    0x10, 0x10, 0x10, 0x00, 0x01, 0x00,
    0x04, 0x00, 0x28, 0x01, 0x00, 0x00,
    0x16, 0x00, 0x00, 0x00, 0x28, 0x00
  };
  request->send_P(200, "image/x-icon", (const uint8_t*)blankFavicon, sizeof(blankFavicon));
});


  server.begin();
}

void loop() {
  int analogValue = analogRead(TDS_PIN);
  float voltage = analogValue * VREF / ADC_RESOLUTION;
  tdsValue = (133.42 * voltage * voltage * voltage 
             - 255.86 * voltage * voltage 
             + 857.39 * voltage) * 0.5;

  if (tdsValue <= 50) quality = "Excellent";
  else if (tdsValue <= 150) quality = "Good";
  else if (tdsValue <= 250) quality = "Fair";
  else if (tdsValue <= 500) quality = "Poor";
  else if (tdsValue <= 1200) quality = "Very Poor";
  else quality = "Unsafe";

  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 10);
  display.print("TDS: ");
  display.print((int)tdsValue);
  display.println("ppm");

  display.setTextSize(1);
  display.setCursor(0, 50);   
  display.print("Quality: ");
  display.print(quality);
  display.display();

  Serial.println("TDS: " + String((int)tdsValue) + " ppm - " + quality);
  delay(1000);
}
