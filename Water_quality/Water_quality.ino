#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DallasTemperature.h>
#include <OneWire.h>

// WiFi Configuration
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// OLED Display Configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Sensor Pin Definitions
#define SENSOR_ANALOG_PIN A0   // Analog pin for TDS, pH, and Turbidity (requires multiplexing or sequential reading)
#define ONE_WIRE_BUS D2        // Digital pin for the DS18B20 temperature sensor

// TDS Sensor Parameters
const float VREF = 3.3;        // Analog Reference Voltage (ESP8266 is 3.3V)
const float ADC_RESOLUTION = 1024.0;
float tdsValue = 0;

// pH Sensor Parameters (needs calibration)
const float phCalibrationOffset = 0.00; // Calibrate this value
float phValue = 0;

// Turbidity Sensor Parameters
float turbidityVoltage = 0;
float turbidityValue = 0;

// Temperature Sensor Setup
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
float temperatureC = 25.0; // Default to 25C for initial compensation

// Web Server Setup
AsyncWebServer server(80);

void setup() {
  Serial.begin(9600);

  // Initialize OLED display
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

  // Initialize temperature sensor
  sensors.begin();

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting...");
  }
  Serial.println("Connected: " + WiFi.localIP().toString());

  // Display connection success on OLED
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("WiFi Connected!");
  display.setCursor(0, 10);
  display.print("IP: ");
  display.println(WiFi.localIP());
  display.display();
  delay(2000);

  // Set up web server routes
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request){
    String json = "{\"tds\":" + String((int)tdsValue) + ", "
                  "\"ph\":" + String(phValue, 2) + ", "
                  "\"turbidity\":" + String(turbidityValue, 2) + ", "
                  "\"temp\":" + String(temperatureC, 1) + "}";
    request->send(200, "application/json", json);
  });

  server.begin();
}

void loop() {
  // Read Temperature Sensor
  sensors.requestTemperatures();
  temperatureC = sensors.getTempCByIndex(0);
  if (temperatureC == -127.00) { // check for temperature sensor error
      Serial.println("Error reading temperature sensor!");
      temperatureC = 25.0; // default to a safe value
  }

  // Read TDS Sensor
  int analogTds = analogRead(SENSOR_ANALOG_PIN);
  float voltageTds = analogTds * VREF / ADC_RESOLUTION;
  tdsValue = (133.42 * voltageTds * voltageTds * voltageTds
              - 255.86 * voltageTds * voltageTds
              + 857.39 * voltageTds) * 0.5;

  // Read pH Sensor (simulated with a simple linear model, requires calibration for accuracy)
  int analogPh = analogRead(SENSOR_ANALOG_PIN);
  float voltagePh = analogPh * VREF / ADC_RESOLUTION;
  phValue = 3.5 * voltagePh + phCalibrationOffset; // A simple linear relationship, a real sensor needs more complex logic

  // Read Turbidity Sensor
  int analogTurbidity = analogRead(SENSOR_ANALOG_PIN);
  turbidityVoltage = analogTurbidity * VREF / ADC_RESOLUTION;
  // A simple linear mapping for a generic turbidity sensor. Calibrate this for your specific sensor.
  turbidityValue = (-1120.4 * turbidityVoltage * turbidityVoltage + 5742.3 * turbidityVoltage - 4352.8);
  if (turbidityValue < 0) turbidityValue = 0; // Ensure value is not negative

  // Display data on OLED
  display.clearDisplay();
  display.setTextSize(1);

  display.setCursor(0, 0);
  display.print("TDS: ");
  display.print((int)tdsValue);
  display.println(" ppm");

  display.setCursor(0, 15);
  display.print("PH: ");
  display.println(phValue, 2);

  display.setCursor(0, 30);
  display.print("Temp: ");
  display.print(temperatureC, 1);
  display.println(" C");

  display.setCursor(0, 45);
  display.print("Turbidity: ");
  display.print((int)turbidityValue);
  display.println(" NTU");

  display.display();

  // Print to Serial Monitor for debugging
  Serial.print("TDS: "); Serial.print((int)tdsValue); Serial.print(" ppm | ");
  Serial.print("pH: "); Serial.print(phValue, 2); Serial.print(" | ");
  Serial.print("Temp: "); Serial.print(temperatureC, 1); Serial.print(" C | ");
  Serial.print("Turbidity: "); Serial.print((int)turbidityValue); Serial.println(" NTU");

  delay(5000); // Wait 5 seconds before next reading
}

// HTML and JavaScript for the Web Server
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <title>Water Quality Monitor</title>
  <script src="https://cdn.plot.ly/plotly-2.30.0.min.js"></script>
  <style>
    body { font-family: Arial, sans-serif; background-color: #f4f4f9; color: #333; margin: 0; padding: 20px; text-align: center; }
    h1 { color: #5c6773; }
    .container { display: flex; flex-wrap: wrap; justify-content: center; gap: 20px; margin-top: 20px; }
    .gauge-container { background: #fff; padding: 20px; border-radius: 10px; box-shadow: 0 4px 8px rgba(0,0,0,0.1); width: 300px; }
    .value-display { font-size: 24px; font-weight: bold; margin-top: 10px; color: #4CAF50; }
  </style>
</head>
<body>
  <h1>IoT Water Quality Monitoring</h1>
  <div class="container">
    <div class="gauge-container">
      <h3>TDS (ppm)</h3>
      <div id="chartTDS"></div>
      <p class="value-display" id="valueTDS"></p>
    </div>
    <div class="gauge-container">
      <h3>pH</h3>
      <div id="chartPH"></div>
      <p class="value-display" id="valuePH"></p>
    </div>
    <div class="gauge-container">
      <h3>Temperature (°C)</h3>
      <div id="chartTemp"></div>
      <p class="value-display" id="valueTemp"></p>
    </div>
    <div class="gauge-container">
      <h3>Turbidity (NTU)</h3>
      <div id="chartTurbidity"></div>
      <p class="value-display" id="valueTurbidity"></p>
    </div>
  </div>

  <script>
    function drawGauge(id, value, title, range, steps, suffix) {
      var data = [{
        type: "indicator",
        mode: "gauge+number",
        value: value,
        title: { text: title, font: { size: 18 } },
        gauge: {
          axis: { range: range, tickwidth: 1, tickcolor: "darkgray" },
          bar: { color: "#5c6773" },
          steps: steps
        }
      }];
      var layout = { width: 280, height: 200, margin: { t: 25, b: 25, l: 25, r: 25 } };
      Plotly.newPlot(id, data, layout);
      document.getElementById("value" + id.replace("chart", "")).innerText = value.toFixed(1) + " " + suffix;
    }

    function updateSensors() {
      fetch("/data")
        .then(response => response.json())
        .then(data => {
          // Update TDS
          drawGauge("chartTDS", data.tds, "TDS", [0, 1200], [
            { range: [0, 250], color: "#d4edda" },
            { range: [250, 500], color: "#fff3cd" },
            { range: [500, 1200], color: "#f8d7da" }
          ], "ppm");

          // Update pH
          drawGauge("chartPH", data.ph, "pH", [0, 14], [
            { range: [0, 6.5], color: "#f8d7da" },
            { range: [6.5, 7.5], color: "#d4edda" },
            { range: [7.5, 14], color: "#fff3cd" }
          ], "");

          // Update Temperature
          drawGauge("chartTemp", data.temp, "Temperature", [0, 50], [
            { range: [0, 15], color: "#add8e6" },
            { range: [15, 30], color: "#d4edda" },
            { range: [30, 50], color: "#f8d7da" }
          ], "°C");

          // Update Turbidity
          drawGauge("chartTurbidity", data.turbidity, "Turbidity", [0, 1000], [
            { range: [0, 5], color: "#d4edda" },
            { range: [5, 50], color: "#fff3cd" },
            { range: [50, 1000], color: "#f8d7da" }
          ], "NTU");
        })
        .catch(error => console.error('Error fetching data:', error));
    }

    setInterval(updateSensors, 5000);
    updateSensors(); // Initial call
  </script>
</body>
</html>
)rawliteral";
