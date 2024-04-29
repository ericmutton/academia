/*
 * In this lab, you will get familiar with I2C communication for the Arduino ESP32 development board and make use of the WIFI capabilities of the ESP32.
 * Specifically, you are expected to implement I2C protocol to read temperature from the TMP100 sensor ( http://www.ti.com/product/TMP100) and display the results in a browser.
 * Due to problem with the ESP32C3 antenna, you will only need to use the ESP32 Dev Module in this lab..
 * 
 * Lab Objectives
 * 1. Configure the slave address for the TMP100 sensor.
 * 2. Set up the required circuitry using the recommended pull-up resistors (internal pullup resistors also work) and capacitors for the sensor. (Page 19 of the sensor datasheet should be helpful).
 * 3. Read temperature data from the sensor (in bytes).
 * 4. Calculate and display the temperature in degree Celsius and degree Fahrenheit on a local machine using the Serial monitor for the Arduino.
 * 5. Display the temperature on a remote device using the WIFI capabilities of the ESP32 device. At the minimum, you should be able to connect using telnet or HTTP from a laptop.
 * 
 * Extra credit for being able to acquire the data and display it on your phone, that is, write a phone App to communicate with the device.
 * 
 * You should upload the code for each of the two programs to eLearning. You have two weeks to complete this lab.  
 */
#define DEBUG  // Enable print debugging
// #define TELNET  // Enable telnet temperature display
#define HTTP  // Enable http temperature display

#include <Temperature_LM75_Derived.h>  // Generic_LM75_9_to_12Bit_OneShot
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiServer.h>

#include <WebServer.h>

/* TI TMP100 setup */

// 2. Set up the required circuitry using the recommended pull-up resistors (internal pullup resistors also work)
// and capacitors for the sensor. (Page 19 of the sensor datasheet should be helpful).
// Internal pull-ups are 45-kΩ, datasheet recommends 5kΩ.
#define I2C_SDA 33  // 5-kΩ pull-up resistor required.
#define I2C_SCL 32  // 5-kΩ pull-up resistor required.
// 1. Configure the slave address for the TMP100 sensor.
// 3. Read temperature data from the sensor (in bytes).
Generic_LM75 TMP100(0x48);  // Typical I2C address is 0x48. 0b01001000

/* LAN Network Information */

// 1. Ensure WiFi Network Access enables LAN communication.
const char *ssid = "";    // Change this to your WiFi SSID
const char *password = "";  // Change this to your WiFi password
IPAddress ip_addr;
#define MAX_SRV_CLIENTS 2  // Maximum number of clients.

#ifdef TELNET
#define TELNET_SRV_PORT 23  // Default telnet port.
WiFiServer telnetServer(TELNET_SRV_PORT);
WiFiClient telnetClients[MAX_SRV_CLIENTS];
#endif
#ifdef HTTP
#define HTTP_SRV_PORT 80  // Default http port.
WebServer webServer(HTTP_SRV_PORT);
String header;
// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0;
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;
#endif

/* WiFi Connectivity Configuration */

// When in verbose mode, the ESP32 should report the meaning of each code to the Serial monitor. Otherwise, it should only report the code itself.
/**
 * Decodes integer status codes for verbosity.
 * @param status a status code of 255, 0, 1, 2, 3, 4, 5, 6.
 * @returns human-readable form of wl_status_t or integer unknown status.
 */
String printStatus(wl_status_t status) {
  switch (status) {
    case WL_NO_SHIELD:
      return "No WiFi shield";
    case WL_IDLE_STATUS:
      return "Idle";
    case WL_NO_SSID_AVAIL:
      return "No SSID available";
    case WL_SCAN_COMPLETED:
      return "Scan completed";
    case WL_CONNECTED:
      return "Connected";
    case WL_CONNECT_FAILED:
      return "Connection failed";
    case WL_CONNECTION_LOST:
      return "Connection lost";
    case WL_DISCONNECTED:
      return "Disconnected";
    default:
      return String(status);  // report the code itself.
  }
}

/**
 * Will initialize the WIFI connection.
 *
 * @param ssid is the SSID (Service Set IDentifier) of the wireless network
 * @param pass is the required password for wireless network
 * @param verbose is a Boolean which if true will output messages about the status of the connection as it is attempted
 * @param ip_address is the returned IP address from the DHCP WIFI server
 * @param tries is the number of attempts to contact the WIFI server before giving up
 * @returns integer status code specific to the Arduino WIFI code
 */
extern int wifi_startup(const char *ssid, const char *pass, bool verbose, IPAddress *ip_address, int tries = 5) {
  // The code should continue to make attempts until it either exhausts it tries or it successfully receives the WL_CONNECTED code.
  for (int attempts = 0; attempts < tries; attempts++) {
    WiFi.begin(ssid, password);
    if (verbose) {
      Serial.printf("Attempt #%d: Connection Status: %s\n",
                    attempts, printStatus(WiFi.status()).c_str());
    } else {
      Serial.println(WiFi.status());
    }
    if (WiFi.status() == WL_CONNECTED) {
      *ip_address = WiFi.localIP();
      break;
    }
    delay(5000);
  }
  return WiFi.status();
}

void setup() {
  Serial.begin(115200);
  /* TI TMP100 setup */
  // 2. Set up the required I2C interface.
  Wire.setPins(I2C_SDA, I2C_SCL);
  Wire.begin();
#ifdef DEBUG
  Serial.println("******************************************************");
  Serial.print("Connecting to ");
  Serial.println(ssid);
#endif
  int networkStatus = wifi_startup(ssid, password, true, &ip_addr);
  if (networkStatus != WL_CONNECTED) {
#ifdef DEBUG
    wl_status_t status = static_cast<wl_status_t>(networkStatus);
    Serial.printf("WiFi Startup Error: %s\n", printStatus(status).c_str());
    delay(60000);  // Wait one minute before retrying connection attempts.
    ESP.restart();
#endif
  } else {
#ifdef DEBUG
    Serial.printf("WiFi connected, DHCP released IP address %s\n", WiFi.localIP().toString().c_str());
    Serial.println("******************************************************");
#endif
// At the minimum, you should be able to connect using telnet or HTTP from a laptop.
#ifdef TELNET
    telnetServer.begin();
    telnetServer.setNoDelay(true);
    Serial.printf("Ready! Use 'telnet %s %d' to connect \n", WiFi.localIP().toString().c_str(), TELNET_SRV_PORT);
#endif
#ifdef HTTP
    webServer.on("/", handle_OnConnect);
    webServer.begin();
#endif
  }
}
#ifdef TELNET
void acceptClients() {
  //check if there are any new clients
  uint8_t client;
  if (telnetServer.hasClient()) {
    for (client = 0; client < MAX_SRV_CLIENTS; client++) {
      //find free/disconnected spot
      if (!telnetClients[client] || !telnetClients[client].connected()) {
        if (telnetClients[client]) {
          telnetClients[client].stop();
        }
        telnetClients[client] = telnetServer.accept();
        if (!telnetClients[client]) {
          Serial.println("available broken");
        }
        Serial.printf("New client: %d %s\n", client, telnetClients[client].remoteIP().toString());
        break;
      }
    }
    if (client >= MAX_SRV_CLIENTS) {
      //no free/disconnected spot so reject
      telnetServer.accept().stop();
    }
  }
}
#endif
#ifdef HTTP
void handle_OnConnect() {
  float celsius = TMP100.readTemperatureC();
  float fahrenheit = TMP100.readTemperatureF();
  webServer.send(200, "text/html", SendHTML(celsius, fahrenheit));
}
void handle_NotFound() {
  webServer.send(404, "text/plain", "Not found");
}
String SendHTML(uint8_t celsius, uint8_t fahrenheit) {
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr += "<title>LED Control</title>\n";
  ptr += "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr += "body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr += "p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr += "</style>\n";
  ptr += "</head>\n";
  ptr += "<body>\n";
  ptr += "<h1>ESP32 Web Server</h1>\n";
  // ptr += "<h3>Using Station(STA) Mode</h3>\n";
  ptr += "<p>" + String(celsius) + " °C, " + String(fahrenheit) + "°F </p>";
  ptr += "</body>\n";
  ptr += "</html>\n";
  return ptr;
}
#endif

#define MAX_STRING_LENGTH 23

void loop() {
  // put your main code here, to run repeatedly:
  // 4. Calculate and display the temperature in degree Celsius and degree Fahrenheit on a local machine using the Serial monitor for the Arduino.
  //if (/*Wire.available()*/) {
  float celsius = TMP100.readTemperatureC();
  float fahrenheit = TMP100.readTemperatureF();
  // size_t len = 2;
  // uint8_t sbuf[len];
  // Wire.readBytes(sbuf, len);

  uint8_t sbuf[MAX_STRING_LENGTH];
  // Format the string into the buffer
  int len = snprintf((char *)sbuf, MAX_STRING_LENGTH, "%.2lf °C, %.2lf °F\n", celsius, fahrenheit);
  //Serial.printf("%.2lf °C, %.2lf °F\n", celsius, fahrenheit);
  //}
  // 5. Display the temperature on a remote device using the WIFI capabilities of the ESP32 device.
  // At the minimum, you should be able to connect using telnet or HTTP from a laptop.
#ifdef HTTP
  webServer.handleClient();
#endif
#ifdef TELNET
  uint8_t client;
  if (WiFi.status() == WL_CONNECTED) {
    acceptClients();
    //check clients for data
    for (client = 0; client < MAX_SRV_CLIENTS; client++) {
      if (telnetClients[client] && telnetClients[client].connected()) {
        //if (telnetClients[client].available()) {
        // get data from the telnet client and push it to Serial Monitor
        //   while (serverClients[client].available()) {
        //     Serial.print(serverClients[client].read());
        //   }
        //   Serial.println();
        // }
      } else {
        if (telnetClients[client]) {
          telnetClients[client].stop();
        }
      }
    }
    //check I2C for data
    if (Wire.available()) {
      //push I2C data to all connected telnet clients
      for (client = 0; client < MAX_SRV_CLIENTS; client++) {
        if (telnetClients[client] && telnetClients[client].connected()) {
          telnetClients[client].write(sbuf, len);
          delay(1);
        }
      }
    }
  } else {
    Serial.println("WiFi not connected!");
    for (client = 0; client < MAX_SRV_CLIENTS; client++) {
      if (telnetClients[client]) {
        telnetClients[client].stop();
      }
    }
    delay(1000);
  }
#endif
#ifdef HTTP

#endif
  // Extra credit for being able to acquire the data and display it on your phone, that is, write a phone App to communicate with the device.
  //}
}
