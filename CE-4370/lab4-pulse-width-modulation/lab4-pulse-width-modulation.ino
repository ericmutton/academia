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
 * You should use the ledcXXXX routines to control the pulse-width modulation. 
 * You should use the hardware timers to control the time (see hw_timer_t data structures for ESP32).
 * What is the PWM frequency at which you can no longer see the LED flashing but looks "steady"?
 * Experiment with different frequencies.
 * You should implement the design on one of the ESP32 Arduino development boards (your choice).
 */
#define DEBUG

#define LED_BUILTIN_RED 16    // IO16
#define LED_BUILTIN_GREEN 17  // IO17
#define LED_BUILTIN_BLUE 18   // IO18
#define PUSH_BTN 4            // IO4

/* Push Button */

struct Button {
  const uint8_t PIN;
  bool pressed;
};
Button button = { PUSH_BTN, false };

void ARDUINO_ISR_ATTR isr(void* arg) {
  Button* s = static_cast<Button*>(arg);
  s->pressed = true;
}
void ARDUINO_ISR_ATTR isr() {
  button.pressed = true;
}

/* Pulse Width Modulation */

const int PWM_CHANNEL = 0;
const int PWM_FREQ = 5000;  // Recall that Arduino Uno is ~490 Hz. Official ESP32 example uses 5,000Hz
const int PWM_RESOLUTION = 8;

// The max duty cycle value based on PWM resolution (will be 255 if resolution is 8 bits)
const int MAX_DUTY_CYCLE = (int)(pow(2, PWM_RESOLUTION) - 1);

hw_timer_t* timer = NULL;
volatile int dutyCycle = 0;

void IRAM_ATTR fadeIn() {
  ledcWrite(PWM_CHANNEL, MAX_DUTY_CYCLE - dutyCycle);
  if (dutyCycle == MAX_DUTY_CYCLE) {
    dutyCycle = 0;
    timerStop(timer);
  } else {
    dutyCycle++;
  }
}
void IRAM_ATTR fadeOut() {
  ledcWrite(PWM_CHANNEL, MAX_DUTY_CYCLE - dutyCycle);
  if (dutyCycle == 0) {
    dutyCycle = 0;
    timerStop(timer);
  } else {
    dutyCycle--;
  }
}
void IRAM_ATTR hold() {
  if (dutyCycle == MAX_DUTY_CYCLE) {
    dutyCycle = 0;
    timerStop(timer);
    Serial.println("held for 6 seconds.");
  } else {
    dutyCycle++;
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(button.PIN, INPUT_PULLUP);
  attachInterruptArg(button.PIN, isr, &button, FALLING);
  // LEDCXXXX
  ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(LED_BUILTIN_GREEN, PWM_CHANNEL);
  ledcWrite(PWM_CHANNEL, MAX_DUTY_CYCLE);  //
  // Use a prescaler of 80 to match the APB_CLK = 80 MHz.
  timer = timerBegin(0, 80, true);  // use timer0, TickTime = 1us divisions, count up.
}

void loop() {
  if (button.pressed) {
    button.pressed = false;
    Serial.println("button pressed");
    // fade in linearly for 3 seconds
    timerAttachInterrupt(timer, &fadeIn, true);
    timerSetCountUp(timer, true);
    dutyCycle = 0;
    timerAlarmWrite(timer, 3000 * 1000 / MAX_DUTY_CYCLE, true);  // 1ms alarm to ISR
    timerAlarmEnable(timer);
    timerStart(timer);
#ifdef DEBUG
    Serial.println("fading in for 3 seconds...");
#endif
    while (timerStarted(timer))
      ;
    timerAlarmDisable(timer);
    timerDetachInterrupt(timer);

    // stay at max brightness for 6 seconds
    timerAttachInterrupt(timer, &hold, true);
    timerSetCountUp(timer, true);
    dutyCycle = 0;
    timerAlarmWrite(timer, (6000 * 1000) / MAX_DUTY_CYCLE, true);  // 6s alarm to ISR
    timerAlarmEnable(timer);
    ledcWrite(PWM_CHANNEL, 0);
    timerStart(timer);
#ifdef DEBUG
    Serial.println("holding for 6 seconds...");
#endif
    while (timerStarted(timer))
      ;
    timerAlarmDisable(timer);
    timerDetachInterrupt(timer);

    // fade out linearly for 3 seconds
    timerAttachInterrupt(timer, &fadeOut, true);
    dutyCycle = MAX_DUTY_CYCLE;
    timerSetCountUp(timer, true);
    timerAlarmWrite(timer, 3000 * 1000 / MAX_DUTY_CYCLE, true);  // 1ms alarm to ISR
    timerAlarmEnable(timer);                                     //Just Enable
    timerStart(timer);
    Serial.println("fading out for 3 seconds...");
    while (timerStarted(timer))
      ;
    timerDetachInterrupt(timer);
  }
}
