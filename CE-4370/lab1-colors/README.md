# Lab 1 Colors

Discover how to interface to all of the boards in Arduino by uploading and configuring the proper drivers and software.

## Prompt
 In this lab, you will use the Arduino environment to blink three different development boards.
 
 The Arduino Zero has a simple on-board LED. The two ESP32 development boards have RGB LEDs.
 
 While the Zero has a single LED, your blink times should correspond to the times in the specification.
 
 Each student will have a different specification and your specification is found in the table below.

| Last Name | First Name | Username  | color0  | color1 | color1On | color1Off | color2On | color2Off |
| --------- | ---------- | --------- | ------- | ------ | -------- | --------- | -------- | --------- |
| Mutton    | Eric       | ejm190004 | magenta | white  | 4        | 4         | 2        | 3         |

 You are to turn on LED color 1 for the specifed on time (in seconds),  wait for the specified off time, and then turn on the LED color 2 for its specified on and off times. All times are in seconds.
 
 Not all colors are primary colors; some need mixing. 

 ## Resources


### Tooling
<details>
<summary>Arduino Additional Boards</summary>

```
https://adafruit.github.io/arduino-board-index/package_adafruit_index.json
https://dl.espressif.com/dl/package_esp32_index.json
https://files.seeedstudio.com/arduino/package_seeeduino_boards_index.json
https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_dev_index.json
```

</details>

### Boards
- [EZSBC IOT Controller](https://www.ezsbc.com/product/esp32-breakout-and-development-board/)
- [ESP32-C3-DevKitM-1U](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c3/hw-reference/esp32c3/user-guide-devkitm-1.html)
- [Arduino MKR Zero](https://docs.arduino.cc/hardware/mkr-zero/)

### Datasheets

- [ESP32-WROOM-32E](https://www.espressif.com/sites/default/files/documentation/esp32-wroom-32e_esp32-wroom-32ue_datasheet_en.pdf)
- [ESP32-C3-WROOM-02](https://www.espressif.com/sites/default/files/documentation/esp32-c3-wroom-02_datasheet_en.pdf)
- [Atmel ATSAMD 21G18A-U](https://ww1.microchip.com/downloads/en/DeviceDoc/SAM-D21DA1-Family-Data-Sheet-DS40001882G.pdf)