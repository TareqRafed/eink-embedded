# ESP8266 E-ink Embedded Part

The main file calls these functions

```c
    wifi_init(); // init the wifi module
    GPIO_Init(); // init gpio module
    Init_SPI(); // init spi 
    EPD_Init(); // init eink display
    mqtt_connect_to_server(); // subscribes to mqtt and publish fetch command (fetch command is one of the custom protocol commands between client and the embedded system) 
```

in the components folder, you will find the `eink` folder, which holds the display driver, not all features of the display were written only the one I needed to do the MVP.

in the `mqtt_local` which was called this because of conflict with the sdk library `mqtt`, you'll find a hardcoded (should be changed) mqtt topics, starts with `device/${current-device-serialnumber}`, the serial number is crucial since it is the only way the server can differentiate between the embedded devices. you can also change the mqtt uri, mqtt broker user and password from `mqtt_local.c` but it would be better if this changes.

you can change wifi ssid and password from `wifi.h`



# Contributing

First, you have to setup [ESP8266_RTOS_SDK](https://github.com/espressif/ESP8266_RTOS_SDK), just follow the instructions, and make sure you understand the build system before you add code, as it enhance your embedded device performance

to flash code 
`make flash`

to flash and monitor
`make flash monitor`
