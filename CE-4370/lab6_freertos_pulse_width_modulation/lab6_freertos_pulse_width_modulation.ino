/*
 * | x | y | z | color |
 * | 3 | 6 | 3 | green |
 In this lab, we will investigate pulse-width modulation and interrupts.

 * Our design intent, is to push a button on the MCU which triggers an interrupt. 
 * Once triggered, the specified color LED on the development board must fade-in
 * linearly (min to max intensity) during an interval of x seconds,
 * stay at maximum brightness for y seconds, and then fade-out 
 * linearly (max brightness to off) with a duration of z seconds.   
 
 * Your personalized values for x, y, z, and color  are given in the attached PDF.
 * In this lab, you cannot make use of "delay" or equivalent routines (millis) to implement
 * the intervals but must make use of the timers incorporated into MCU. Furthermore, you must
 * implement LED intensity variations through the use of pulse-width modulation.
 * You should use the ledcXXXX routines to  control the pulse-width modulation. 
 * You should use the hardware timers to control the time (see hw_timer_t data structures for ESP32).
 * What is the PWM frequency at which you can no longer see the LED flashing but looks "steady"?
 * Experiment with different frequencies.
 * You should implement the design on one of the ESP32 Arduino development boards (your choice).
 */
#include "driver/ledc.h"
#include <esp32-hal-ledc.h>
#define LED_BUILTIN_RED 16
#define LED_BUILTIN_GREEN 17
#define LED_BUILTIN_BLUE 18
#define PUSH_BTN 4

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

//hw_timer_t* timer = NULL;



uint8_t channels[3] = { LOW, HIGH, LOW };  // color is green


int PWM_FREQUENCY = 1000;  // this variable is used to define the time period
int PWM_CHANNEL = 1;       // this variable is used to select the channel number
int PWM_RESOLUTION = 8;    // this will define the resolution of the signal which is 8 in this case
int dutyCycle = 127;       // it will define the width of signal or also the one time

// void IRAM_ATTR onTimer() {
//   //ledcWrite(PWM_CHANNEL, dutyCycle);
// }

void setup() {
  Serial.begin(115200);

  pinMode(LED_BUILTIN_RED, OUTPUT);
  pinMode(LED_BUILTIN_GREEN, OUTPUT);
  pinMode(LED_BUILTIN_BLUE, OUTPUT);
  digitalWrite(LED_BUILTIN_RED, !channels[0]);
  digitalWrite(LED_BUILTIN_BLUE, !channels[2]);

  pinMode(button.PIN, INPUT_PULLUP);
  attachInterruptArg(button.PIN, isr, &button, FALLING);
  // timer0, prescaler 80, count up.
  // timer = timerBegin(0, 80, true);

  ledc_timer_config_t ledc_timer = {
    .speed_mode = LEDC_LOW_SPEED_MODE,     // timer mode
    .duty_resolution = LEDC_TIMER_13_BIT,  // resolution of PWM duty
    .timer_num = LEDC_TIMER_1,             // timer index
    .freq_hz = 4000,                       // frequency of PWM signal
    .clk_cfg = LEDC_AUTO_CLK,              // Auto select the source clock
  };
  // Set configuration of timer0 for high speed channels
  ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));
  ledc_channel_config_t ledc_channel[1] = {
    {
      .gpio_num = LED_BUILTIN_GREEN,
      .speed_mode = LEDC_LOW_SPEED_MODE,
      .channel = LEDC_CHANNEL_0,
      // .intr_type = LEDC_INTR_DISABLE,
      .timer_sel = LEDC_TIMER_1,
      .duty = 0,  // 0->8192
      .hpoint = 0,
      // .flags.output_invert = 0
    }
  };

  ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel[1]));

  ledcAttachPin(LED_BUILTIN_GREEN, PWM_CHANNEL);
  ledcSetup(PWM_CHANNEL, PWM_FREQUENCY, PWM_RESOLUTION);
  ledc_fade_func_install(0);
}

void loop() {
  if (button.pressed) {
    button.pressed = false;
    Serial.println("button pressed");
    // timerAttachInterrupt(timer, &onTimer, true);
    // timerAlarmWrite(timer, 1000000, true);
    // timerAlarmEnable(timer);  //Just Enable
    // timerSetCountUp(timer, true);
    // timerStart(timer);
    // ledc_fade_func_install(LEDC_LOW_SPEED_MODE,); ledc_set_fade_with_time(); ledc_fade_start();
    //ledc_fade_mode_t;
    ledc_set_fade_with_time(LEDC_HIGH_SPEED_MODE, (ledc_channel_t)PWM_CHANNEL, 255, 3000);
    ledc_fade_start(LEDC_HIGH_SPEED_MODE, (ledc_channel_t)PWM_CHANNEL, LEDC_FADE_NO_WAIT);


    // fade in linearly for 3 seconds
    // stay at max brightness for 6 seconds
    // timerStop(timer);
    // fade out linearly for 3 seconds
    //timerDetachInterrupt(timer);
  }
}
