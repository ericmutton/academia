/**
 * In this lab, we will study polls and interrupts on the ESP32 and Zero devices.
 * You will need to write a program for each case resulting in 6 different programs.
 * 
 * In addition, measure the supply current for the routines in both the poll and interrupt programs.
 * This means you should have a total of 12 current measurements for the two ESP32 programs.
 * In order to be overly conservative, use the VIN pin to power the ESP32 development boards.
 * Of course, you should use a protoboard to make the connection to the development board.
 * 
 * |                      | I_{sPoll} (mA)             | I_{sInterrupt} (mA)        |
 * | -------------------- | -------------------------- | -------------------------- |
 * | Board                | Off    | Steady | Flashing | Off    | Steady | Flashing |
 * | -------------------- | ------ | ------ | -------- | ------ | ------ | -------- |
 * | EZSBC IOT Controller | -55.19 | -56.06 | -50.72   | -55.79 | -57.29 | -49.95   |
 * | ESP32-C3-DevKitM-1U  | -29.60 | -29.67 | -28.98   | -29.70 | -29.87 | -29.15   |     
 * | Arduino MKR Zero     | -58.19 | -61.15 | -59.59   | -58.48 | -60.61 | -58.74   |
 * 
 * What is the duty cycle of the flashing LED? (64ms)/1000s = 0.0064% = 6.4 millipercent.
*/

// #define DEBUG  // Enable serial print debugging
#define EnablePolling // Choose between polls or interrupts

/**
 * In addition, we are going to study storing variables in the EEPROM. You are to write into the
 * EEPROM information so that when the board powers up again, the color of the lights are reversed.
 * Steady should be the second color and flashing should be the first color given to you.
 */
#define EEPROM_SIZE sizeof(uint8_t)
// All boards other than the Arduino MKR ZERO can use arduino-esp32.
#ifndef ARDUINO_SAMD_MKRZERO
#include <ESP32TimerInterrupt.h>
#include <Preferences.h>
Preferences preferences;
typedef struct {
  bool scheme;
} choices_t;
#else  // On the Arduino MKR ZERO, will have to emulate an EEPROM.
#include <FlashStorage_SAMD.h>
#define EEPROM_EMULATION_SIZE EEPROM_SIZE
#endif
/**
 * Your unique color specification is given in the table below.
 * | Last Name | First Name | Username  | color0  | color1 | on_duration |
 * | --------- | ---------- | --------- | ------- | ------ | ----------- |
 * | Mutton    | Eric       | ejm190004 | red     | green  | 64          |
 * 
 * Since the Arduino Zero does not have colored LEDs, you will just turn
 * on the builtin LED. Use the same on duration as the ESP32 board data.
*/
// The EZSBC IOT Controller has RGB color channels on GPIO16, GPIO17, GPIO18.
#ifdef ARDUINO_ESP32_DEV
#define LED_BUILTIN_RED 16
#define LED_BUILTIN_GREEN 17
#define LED_BUILTIN_BLUE 18
#endif
uint8_t routine = 0;
struct ColorScheme {
  uint8_t off[3];
  uint8_t steady[3];
  uint8_t flashing[3];
} colorScheme;
// ColorScheme colorScheme /*= {.off = {LOW, LOW, LOW}}*/; // SAMD does not support non-intuitive designated initializers.
volatile bool flashing = false;
bool flashedLED = false;
unsigned long previousMillis = 0;
unsigned long currentMillis = 0;
const long onTime = 64;
const long offTime = 1000 - onTime;

void setColorScheme(bool colorChoice) {
  // Reverse the color of the lights
  if (colorChoice) {
    colorScheme = {
      .off = { LOW, LOW, LOW },
      .steady = { HIGH, LOW, LOW },   // color0
      .flashing = { LOW, HIGH, LOW }  // color1
    };
  } else {
    colorScheme = {
      .off = { LOW, LOW, LOW },
      .steady = { LOW, HIGH, LOW },   // color1
      .flashing = { HIGH, LOW, LOW }  // color0
    };
  }
}

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
/**
 * Opted for external push button for board agnosticism.
 */
volatile bool pressed = false;
#ifndef ARDUINO_ESP32C3_DEV
#define PUSH_BTN 0  // ADC1_0
#elif defined ARDUINO_ESP32_DEV
#define PUSH_BTN 25  // DAC0
#else
#define PUSH_BTN 0  // D0
#endif
struct Button {
  const uint8_t PIN;
  bool pressed;
};
Button button = { PUSH_BTN, true };
// Interrupt Subroutines
#ifndef EnablePolling
#ifndef ARDUINO_SAMD_MKRZERO
void ARDUINO_ISR_ATTR isr(void* arg) {
  Button* s = static_cast<Button*>(arg);
  s->pressed = true;
}
void ARDUINO_ISR_ATTR isr() {
  button.pressed = true;
}
#else
void isr() {
  button.pressed = true;
}
#endif
#endif
// Polling 
int value;
unsigned long start;
unsigned long end;

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

  pinMode(button.PIN, INPUT_PULLUP);

#ifndef EnablePolling
// 2) Use an interrupt to detect the EZSBC IOT Controller switch depression
// 4) Use an interrupt to detect the ESP32-C3-DevKitM-1U switch depression
#ifndef ARDUINO_SAMD_MKRZERO
  attachInterruptArg(button.PIN, isr, &button, FALLING);
#else
  // 6) Use an interrupt to detect the Arduino MKR Zero switch depression
  attachInterrupt(button.PIN, isr, FALLING);
#endif

#endif

  // *) Write into the EEPROM the color scheme
  volatile bool scheme;
#ifndef ARDUINO_SAMD_MKRZERO
  preferences.begin("choices", false);
  scheme = preferences.getBool("scheme", 0);
  preferences.putBool("scheme", !scheme);
  preferences.end();
#else
  EEPROM.get(0, scheme);
  EEPROM.put(0, (bool)!scheme);
  /*if (!EEPROM.getCommitASAP())*/ EEPROM.commit();
#endif
  setColorScheme(scheme);

#ifdef DEBUG
  Serial.begin(115200);
#ifdef ARDUINO_ESP32C3_DEV
  Serial.println("I am an ESP32-C3-DevKitM-1U!");
#endif
#ifdef ARDUINO_ESP32_DEV
  Serial.println("I am an EZSBC IOT Controller!");
#endif
#ifdef ARDUINO_SAMD_MKRZERO
  while (!Serial);  // wait for serial port to connect. Needed for native USB port only
  Serial.println("I am an Arduino MKR ZERO!");
#endif
  Serial.print("Using Steady: ");
  Serial.print(colorScheme.steady[0]);
  Serial.print(colorScheme.steady[1]);
  Serial.print(colorScheme.steady[2]);
  Serial.print(", Flashing: ");
  Serial.print(colorScheme.flashing[0]);
  Serial.print(colorScheme.flashing[1]);
  Serial.println(colorScheme.flashing[2]);
  Serial.println("Ready.");
#endif
}

void loop() {
// 1) Poll the switch using the EZSBC IOT Controller
// 3) Poll a switch using the ESP32-C3-DevKitM-1U
// 5) Poll the switch using the Arduino MKR Zero
#ifdef EnablePolling
  if (digitalRead(button.PIN) == LOW) button.pressed = true;
#endif

  if (button.pressed) {
    button.pressed = false;
    if (routine > 2) routine = 0;
#ifdef DEBUG
    Serial.print("Button has been pressed ");
    Serial.print(routine);
    Serial.println(" times");
#endif
    switch (routine) {
      default:
        // The device should boot up into the off routine, that is, no LEDs are lit.
        setLEDColor(colorScheme.off);
        flashing = false;
        break;
      case 1:
        // On the depression of a switch on the development board,
        // the first LED color should light and be steady.
        setLEDColor(colorScheme.steady);
        flashing = false;
        break;
      case 2:
        flashing = true;
        break;
    }
    routine++;
#ifdef EnablePolling
    delay(600);
#endif
  }
  // LED/RGB non-interrupting blinking logic
  if (flashing) {
    currentMillis = millis();
    // if the LED is off turn it on and vice-versa:
    if (!flashedLED && currentMillis - previousMillis >= offTime) {
      previousMillis = currentMillis;
      setLEDColor(colorScheme.flashing);
      flashedLED = true;
    } else if (flashedLED && currentMillis - previousMillis >= onTime) {
      previousMillis = currentMillis;
      setLEDColor(colorScheme.off);
      flashedLED = false;
    }
  }
}