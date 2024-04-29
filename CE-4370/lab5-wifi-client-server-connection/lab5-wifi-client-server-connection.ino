/*
 * In order to understand how WIFI works on the ESP32, you will first write code to talk to a WEB server, 
 * namely Google, and display the result of searching your name. You do not need to format the resulting HTML
 * but display the raw data into the serial monitor of the Arduino IDE. The first part of the lab insures that
 * you are correctly connected to the Internet and that you can implement a client connection.
 * 
 * You should upload the code for each of the two programs to eLearning. You have two weeks to complete this lab.  
 */
#define DEBUG

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>

/* LAN Network Information */

// 1. Ensure WiFi Network Access enables LAN communication.
const char *ssid = "";    // Change this to your WiFi SSID
const char *password = "";  // Change this to your WiFi password
IPAddress ip_addr;

/* URL Parameters and HTTP Response */

// 2. Make a [Programmable Search Engine](https://programmablesearchengine.google.com/controlpanel/all)
String cx = "";  // Change this to your Programmable Search Engine ID
// 3. Then get a [Custom Search API Key](https://developers.google.com/custom-search/v1/introduction)
String apiKey = "";  // Change this to your Custom Search API Key

String query = "";
String url = "https://customsearch.googleapis.com/customsearch/v1?key=" + apiKey + "&cx=" + cx + "&q=";

/**
 * Encodes unmapped characters into naive [RFC 3986 Percent-Encoding](https://www.rfc-editor.org/rfc/rfc3986#section-2.1).
 * @param str string of characters to encode.
 * @returns encoded octets.
 */
String pctEncode(String unmappedChars) {
  String pctEncoded = "";
  char currentChar;
  char encodedChar0;
  char encodedChar1;
  for (int i = 0; i < unmappedChars.length(); i++) {
    currentChar = unmappedChars.charAt(i);
    if (currentChar == ' ') {
      pctEncoded += '+';
    } else if (isalnum(currentChar)) {
      pctEncoded += currentChar;
    } else {
      // Extracting the lower 4 bits of the character and converting to hexadecimal
      encodedChar1 = (currentChar & 0xf) + '0';
      // If the value is greater than 9, convert to corresponding hexadecimal character ('A' to 'F')
      if ((currentChar & 0xf) > 9) {
        encodedChar1 = (currentChar & 0xf) - 10 + 'A';
      }
      // Shifting the character to get the higher 4 bits and converting to hexadecimal
      currentChar = (currentChar >> 4) & 0xf;
      encodedChar0 = currentChar + '0';
      // If the value is greater than 9, convert to corresponding hexadecimal character ('A' to 'F')
      if (currentChar > 9) {
        encodedChar0 = currentChar - 10 + 'A';
      }
      // Appending '%' followed by the two hexadecimal characters
      pctEncoded += '%';
      pctEncoded += encodedChar0;
      pctEncoded += encodedChar1;
    }
    yield();
  }
  return pctEncoded;
}

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
#endif
    query = "Eric Mutton";
    url += pctEncode(query);
    String response;
#ifdef DEBUG
    Serial.printf("Sending HTTP GET Request to URL: %s\n", String(url).c_str());
    Serial.println("******************************************************");
#endif
    HTTPClient http;
    http.useHTTP10(true);
    http.begin(url);
    http.GET();
    response = http.getString();
    http.end();
    // display the raw data into the serial monitor of the Arduino IDE
    Serial.println(response);  // Headers and Response JSON with 10 search items
  }
}

void loop() {
  // put your main code here, to run repeatedly:
}
