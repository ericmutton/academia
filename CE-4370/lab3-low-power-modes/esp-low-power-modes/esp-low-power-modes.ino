/**
 * In this lab, you will modify the interrupt version of Lab 2 and add two new states, light sleep and deep sleep
 * for both the EZSBC IOT Controller and ESP32-C3-DevKitM-1U devices and measure the currents.
 * Unfortunately, sleep modes are not a standard in the Arduino world and are MCU specific.
 * 
 * Your program should cycle through the following states:
 * off (high power), led steady, led flashing, light sleep, and deep sleep on a button press.
 * Measure the currents and calculate the power used in each state.
 *
 * |                      | I_{sHigh} (mA)              | I_{sLight} (mA) | I_{sDeep} (uA) |
 * | -------------------- | --------------------------- | --------------- | -------------- |
 * | Board                | Off    | Steady  | Flashing |                 |                |
 * | -------------------- | ------ | ------- | -------- |                 |                |
 * | EZSBC IOT Controller | -55.77 | -57.27  | -49.78   | -01.90          | -12.90         |
 * | ESP32-C3-DevKitM-1U  | -29.85 | -29.90  | -29.20   | -01.44          | -917           |
 * 
 * The deep sleep current for these devices is around 500 micro amps in contrast to the EZSBC IOT Controller which achieves 10 - 13 microamps.
 */
// #define DEBUG  // Enable serial print debugging
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

volatile bool flashing = false;
bool flashedLED = false;
unsigned long previousMillis = 0;
unsigned long currentMillis = 0;
const long onTime = 64;
const long offTime = 1000 - onTime;

void setLEDColor(uint8_t channels[]) {
// ESP32-C3-DevKitM-1U
#ifdef ARDUINO_ESP32C3_DEV
  neopixelWrite(RGB_BUILTIN, channels[0], channels[1], channels[2]);
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
#ifdef ARDUINO_ESP32_DEV
#define PUSH_BTN 4  // ADC1_0
#elif defined ARDUINO_ESP32C3_DEV
#define PUSH_BTN 4  // DAC0
#endif
struct Button {
  const uint8_t PIN;
  bool pressed;
};
Button button = { PUSH_BTN, true };

void ARDUINO_ISR_ATTR isr(void* arg) {
  Button* s = static_cast<Button*>(arg);
  s->pressed = true;
}
void ARDUINO_ISR_ATTR isr() {
  button.pressed = true;
}

/**
 * Sleep mode configuration
 */
#define BUTTON_PIN_BITMASK BIT(PUSH_BTN)  // 2^PUSH_BTN in hex
#define BUTTON_WAKEUP_LEVEL_DEFAULT 1
#define BUTTON_GPIO_NUM_DEFAULT 4

esp_err_t lightSleep() {
  // disable all other wakeup sources except GPIO.
  esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
  gpio_wakeup_enable(GPIO_NUM_4, GPIO_INTR_LOW_LEVEL);
  esp_sleep_enable_gpio_wakeup();
  return esp_light_sleep_start();
}

void deepSleep() {
#ifdef ARDUINO_ESP32_DEV
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_4, 0);
#elif defined ARDUINO_ESP32C3_DEV
  esp_deep_sleep_enable_gpio_wakeup(GPIO_NUM_4, ESP_GPIO_WAKEUP_GPIO_LOW);
#endif
  esp_deep_sleep_start();
}

//esp_sleep_enable_timer_wakeup(5 * 1000000);
//RTC_DATA_ATTR int sleepState = 0;
esp_sleep_wakeup_cause_t wakeup_reason;

void setup() {
#ifdef DEBUG
  Serial.begin(115200);
#endif
  // EZSBC IOT Controller
#ifdef ARDUINO_ESP32_DEV
  pinMode(LED_BUILTIN_RED, OUTPUT);    // RED
  pinMode(LED_BUILTIN_GREEN, OUTPUT);  // GREEN
  pinMode(LED_BUILTIN_BLUE, OUTPUT);   // BLUE
#endif
  // ESP32-C3-DevKitM-1U RMT HAL handles Neopixel
  colorScheme = {
    .off = { LOW, LOW, LOW },
    .steady = { HIGH, LOW, LOW },   // color0
    .flashing = { LOW, HIGH, LOW }  // color1
  };
  // Only RTC IO can be used as a source for external wake
  // source. They are pins: 0,2,4,12-15,25-27,32-39.
  pinMode(button.PIN, INPUT_PULLUP);
  attachInterruptArg(button.PIN, isr, &button, FALLING);
}

void loop() {
  if (button.pressed) {
    button.pressed = false;
    if (routine > 4) routine = 0;
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
      case 3:
        setLEDColor(colorScheme.off);
        flashing = false;
#ifdef DEBUG
        Serial.printf("Starting light sleep...");
        delay(100);
        Serial.println(lightSleep());
#else
        lightSleep();  //light sleep with configured wakeup options
#endif
        break;
      case 4:
#ifdef DEBUG
        // ESP_SLEEP_WAKEUP_GPIO,         //!< Wakeup caused by GPIO (light sleep only on ESP32, S2 and S3)
        wakeup_reason = esp_sleep_get_wakeup_cause();
        switch (wakeup_reason) {
          case ESP_SLEEP_WAKEUP_EXT0: Serial.println("Wakeup caused by external signal using RTC_IO"); break;
          default: Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason); break;
        }
        Serial.printf("Starting deep sleep...");
#endif
        deepSleep();  // deep sleep.
        break;
    }
#ifdef DEBUG
    Serial.print("Button has been pressed ");
    Serial.print(routine);
    Serial.println(" times");
#endif
    routine++;
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
