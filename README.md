# IoT Water Quality Monitoring System

An Internet of Things (IoT) project that monitors and displays multiple water quality parameters, including TDS, pH, Turbidity, and Temperature. The system uses an ESP8266 Wi-Fi module to send data to a local web server, which can be viewed on any device connected to the same network. The current readings are also displayed on a small OLED screen.

## 🛠️ Components Used

* **ESP8266 NodeMCU:** The brain of the project, handling all sensor readings and web server functionalities.
* **TDS Sensor (Total Dissolved Solids):** Measures the concentration of dissolved solid particles in water.
* **PH Sensor:** Measures the acidity or alkalinity of the water.
* **Turbidity Sensor:** Measures the cloudiness or haziness of water.
* **DS18B20 Temperature Sensor:** Measures the temperature of the water.
* **OLED Display (128x64 pixels):** A small screen to display real-time sensor data locally.
* **Dot PCB Board:** For a clean and organized circuit assembly.
* **USB Cable:** For programming the ESP8266 and providing power.
* **Jumper Wires:** To connect all the components.

## ⚙️ Software & Libraries

This project is built using the Arduino IDE. You will need to install the following libraries to compile the code successfully.

* **ESP8266 Board Package:** `File > Preferences > Additional Boards Manager URLs:`
    `http://arduino.esp8266.com/stable/package_esp8266com_index.json`
* **ESPAsyncWebServer:** For creating the asynchronous web server.
* **ESPAsyncTCP:** A dependency for the web server.
* **Adafruit GFX Library:** The core library for graphics on the OLED.
* **Adafruit SSD1306:** The specific driver for the OLED display.
* **DFRobot_PH_Sensor:** A library to simplify the pH sensor readings.
* **DallasTemperature:** For the DS18B20 temperature sensor.
* **OneWire:** A dependency for the temperature sensor.

You can install these libraries directly through the Arduino IDE's Library Manager (`Sketch > Include Library > Manage Libraries...`).

## 🔌 Circuit Diagram & Hardware Assembly

Connect the components as shown in the diagram in image. The analog sensors (TDS, pH, and Turbidity) should be connected to the analog input pin (A0) on the ESP8266, using a multiplexer or by switching between them in the code. The temperature sensor should be connected to a digital pin.



### Connections:

* **TDS Sensor:**
    * **A0** -> ESP8266 **A0**
* **PH Sensor:**
    * **A0** -> ESP8266 **A0**
* **Turbidity Sensor:**
    * **A0** -> ESP8266 **A0**
* **DS18B20 Temperature Sensor:**
    * **DATA** -> ESP8266 **D2**
* **OLED Display (I2C):**
    * **SDA** -> ESP8266 **D1**
    * **SCL** -> ESP8266 **D2**

---

## 💻 Code & Usage

### 1. **Setup**

* Open the `.ino` file in the Arduino IDE.
* Update the `ssid` and `password` variables with your Wi-Fi network credentials.
* Connect your ESP8266 to your computer via the USB cable.
* Select the correct board and port in the Arduino IDE.

### 2. **Upload**

* Click the "Upload" button to flash the code to the ESP8266.
* Once uploaded, the device will connect to your Wi-Fi network.

### 3. **Access the Web Server**

* Open the Arduino IDE's Serial Monitor to find the IP address of the ESP8266.
* Enter this IP address into a web browser on any device connected to the same Wi-Fi network.
* The web page will display real-time gauges for all the water quality parameters.

---

## 👨‍💻 Contribution

Feel free to fork this repository, suggest improvements, and contribute to this project. If you find any issues or have suggestions, please open a pull request.
