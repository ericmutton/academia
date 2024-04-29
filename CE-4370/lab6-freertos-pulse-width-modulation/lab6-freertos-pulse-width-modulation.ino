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
#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

typedef enum {
  LED_OFF_T = HIGH,
  LED_ON_T = LOW
} LED_STATE_T ;

#ifndef LED_BUILTIN
#define LED_BUILTIN 17
#endif
#define LED2        19
#define LED3        18
const byte switch_pinG = 0 ;// 0 corresponds to GPIO0
// define three tasks for Blink & AnalogRead
extern void FadeIn( void *pvParameters );
extern void FadeOut( void *pvParameters );
extern void Steady( void *pvParameters );

static void IRAM_ATTR button_pressed( void ) ;

// the setup function runs once when you press reset or power the board
void setup() {
  
  // initialize serial communication at 115200 bits per second:
  Serial.begin(115200);
  attachInterrupt(digitalPinToInterrupt(switch_pinG), button_pressed, FALLING);
  xTaskCreatePinnedToCore(
    Steady
    ,  "Steady"
    ,  1024  // Stack size
    ,  NULL
    ,  1  // Priority
    ,  NULL 
    ,  ARDUINO_RUNNING_CORE);

    Serial.println("scheduler starts") ;
    // vTaskStartScheduler() ;
  // Now the task scheduler, which takes over control of scheduling individual tasks, is automatically started.
}

void loop()
{
  // Empty. Things are done in Tasks.
}

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/

void FadeOut(void *pvParameters)  // This is a task.
{
  (void) pvParameters;

/*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.
    
  If you want to know what pin the on-board LED is connected to on your ESP32 model, check
  the Technical Specs of your board.
*/
  Serial.println("Now in FadeOut") ;
  TickType_t times_up = pdMS_TO_TICKS( 10000 );
  TickType_t time50ms = pdMS_TO_TICKS( 50 );
  TickType_t time950ms = pdMS_TO_TICKS( 950 ); 
  // initialize digital LED_BUILTIN on defined pin as an output.
  pinMode(LED2, OUTPUT);
  
  for ( TickType_t count = 1 ; count <= times_up ; count++ ) // A Task shall never return or exit.
  {
    digitalWrite(LED2, LED_ON_T);   // turn the LED on (HIGH is the voltage level)
    count += time50ms ;
    vTaskDelay(time50ms);  // one tick delay (15ms) in between reads for stability
    digitalWrite(LED2, LED_OFF_T);    // turn the LED off by making the voltage LOW
    count += time950ms ;
    vTaskDelay(time950ms);  // one tick delay (15ms) in between reads for stability
  }
  Serial.println("done blinking") ;
  vTaskDelete(NULL) ;
}

void vProcessInterface( void *pvParameter1, uint32_t ulParameter2 )
{
    BaseType_t xInterfaceToService;

    /* The interface that requires servicing is passed in the second
    parameter.  The first parameter is not used in this case. */
    xInterfaceToService = ( BaseType_t ) ulParameter2;

    Serial.println("about to start a steady process.") ;
    /* ...Perform the processing here... */
    xTaskCreatePinnedToCore(
    Steady
    ,  "Steady"
    ,  1024  // Stack size
    ,  NULL
    ,  1  // Priority
    ,  NULL 
    ,  ARDUINO_RUNNING_CORE);

}

static void IRAM_ATTR button_pressed( void )
{
     BaseType_t xHigherPriorityTaskWoken;

    /* The actual processing is to be deferred to a task.  Request the
    vProcessInterface() callback function is executed, passing in the
    number of the interface that needs processing.  The interface to
    service is passed in the second parameter.  The first parameter is
    not used in this case. */
    xHigherPriorityTaskWoken = pdFALSE;
    xTimerPendFunctionCallFromISR( vProcessInterface,
                               NULL,
                               ( uint32_t ) 0,
                               &xHigherPriorityTaskWoken );

    /* If xHigherPriorityTaskWoken is now set to pdTRUE then a context
    switch should be requested.  The macro used is port specific and will
    be either portYIELD_FROM_ISR() or portEND_SWITCHING_ISR() - refer to
    the documentation page for the port being used. */
    // portYIELD_FROM_ISR( /* xHigherPriorityTaskWoken */ );
    portYIELD_FROM_ISR();
}


void Steady(void *pvParameters)  // This is a task.
{
  (void) pvParameters;

/*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.
    
  If you want to know what pin the on-board LED is connected to on your ESP32 model, check
  the Technical Specs of your board.
*/
  /* My steady is 4 seconds */
  
  Serial.println("Entered Steady") ;
  TickType_t xTimeInTicks = pdMS_TO_TICKS( 4000 );    

  // initialize digital LED_BUILTIN on defined pin as an output.
  pinMode(LED3, OUTPUT);

  for (;;) // A Task shall never return or exit.
  {
    digitalWrite(LED3, LED_ON_T);   // turn the LED on (HIGH is the voltage level)
    vTaskDelay(xTimeInTicks);  // a delay of 4 seconds
    digitalWrite(LED3, LED_OFF_T); 
    
    xTaskCreatePinnedToCore(
     FadeOut
    ,  "FadeOut"
    ,  1024  // Stack size
    ,  NULL
    ,  1  // Priority
    ,  NULL 
    ,  ARDUINO_RUNNING_CORE);

    vTaskDelete(NULL) ;

  }
}
