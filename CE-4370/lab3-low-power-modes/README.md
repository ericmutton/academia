# Lab 3 Low Power Modes

Unfortunately, sleep modes are not a standard in the Arduino world and are MCU specific.

## Prompt
In this lab, you will modify the interrupt version of Lab 2 and add two new states, light sleep and deep sleep for both the ESP32 and ESP32C3 devices and measure the currents.

Your program should cycle through the following states off (high power), led steady, led flashing, light sleep, and deep sleep on a button press.

Measure the currents and calculate the power used in each state.


> As an aside, I have tried to get the RP2040 to go into a low sleep mode. The best that I could achieve is 0.84ma which isn't that great. This is one of the reasons we aren't using it this semester. The ESP32C3 board is improperly wired and the deep sleep mode does not turn off all the attached devices. The deep sleep current for this devices is around 500 micro amps in contrast to the ESP32 dev module which achieves 10 - 13 microamps.
I have made a custom board which can test the value of the ESP32C3 sleep.
-- Dr. Bill Swartz

## Resources

[Light Sleep Mode -- ESP32-C3 Book](https://espressif.github.io/esp32-c3-book-en/chapter_12/12.2/12.2.2.html)
[Deep Sleep Mode -- ESP32-C3 Book](https://espressif.github.io/esp32-c3-book-en/chapter_12/12.2/12.2.3.html)

[Sleep Wake Up Sources -- Random Nerd Tutorials](https://randomnerdtutorials.com/esp32-deep-sleep-arduino-ide-wake-up-sources/)