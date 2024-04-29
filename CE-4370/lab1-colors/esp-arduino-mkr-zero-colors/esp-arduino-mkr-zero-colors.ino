/*
  

  | Last Name | First Name | Username  | color0  | color1 | color1On | color1Off | color2On | color2Off |
  | --------- | ---------- | --------- | ------- | ------ | -------- | --------- | -------- | --------- |
  | Mutton    | Eric       | ejm190004 | magenta | white  | 4        | 4         | 2        | 3         |

  Lab 1 Colors

  Demonstrates usage of onboard RGB LED on some ESP dev boards.

  Calling digitalWrite(RGB_BUILTIN, HIGH) will use hidden RGB driver.
    
  RGBLedWrite demonstrates control of each channel:
  void neopixelWrite(uint8_t pin, uint8_t red_val, uint8_t green_val, uint8_t blue_val)

  WARNING: After using digitalWrite to drive RGB LED it will be impossible to drive the same pin
    with normal HIGH/LOW level
*/
#ifdef ARDUINO_ESP32_DEV
#define LED_BUILTIN_RED 16
#define LED_BUILTIN_GREEN 17
#define LED_BUILTIN_BLUE 18
#endif
struct ColorScheme {
  uint8_t off[3];
  uint8_t magenta[3];
  uint8_t white[3];
} colorScheme;
void setLEDColor(uint8_t channels[]) {
  // ESP32-C3-DevKitM-1U
#ifdef ARDUINO_ESP32C3_DEV
  neopixelWrite(RGB_BUILTIN, channels[0], channels[1], channels[2]);
  // Arduino MKR Zero
#elif defined ARDUINO_SAMD_MKRZERO
  // Board has no onboard RGB, toggle LED with any channel
  digitalWrite(LED_BUILTIN, channels[0] | channels[1] | channels[2]);
  // EZSBC IOT Controller
#elif defined ARDUINO_ESP32_DEV
  digitalWrite(LED_BUILTIN_RED, !channels[0]);
  digitalWrite(LED_BUILTIN_GREEN, !channels[1]);
  digitalWrite(LED_BUILTIN_BLUE, !channels[2]);
#endif
}

void setup() {
  // EZSBC IOT Controller
#ifdef ARDUINO_ESP32_DEV
  pinMode(LED_BUILTIN_RED, OUTPUT);    // RED
  pinMode(LED_BUILTIN_GREEN, OUTPUT);  // GREEN
  pinMode(LED_BUILTIN_BLUE, OUTPUT);   // BLUE
  // ESP32-C3-DevKitM-1U RMT HAL handles Neopixel
  // Arduino MKR Zero
#elif defined ARDUINO_SAMD_MKRZERO
  pinMode(LED_BUILTIN, OUTPUT);  // AMBER
#endif
  colorScheme = {
    .off = { LOW, LOW, LOW },
    .magenta = { HIGH, LOW, HIGH },  // color0
    .white = { LOW, HIGH, LOW }      // color1
  };
}

// the loop function runs over and over again forever
void loop() {
  setLEDColor(colorScheme.magenta);
  delay(4000);
  setLEDColor(colorScheme.off);
  delay(4000);
  setLEDColor(colorScheme.white);
  delay(2000);
  setLEDColor(colorScheme.off);
  delay(3000);
}
