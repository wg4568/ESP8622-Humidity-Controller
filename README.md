# ESP8622 Humidity Controller

ESP8622 sketch for remotely controlling a humidifier using webhooks. Uses a DHT humidity sensor, NodeMCU ESP8622 microcontroller and an I2C LCD display. Humidifier is controlled with a Kasa smart switch and [IFTTT](https://ifttt.com/) webhooks. The sketch also hosts a simple webserver providing most recent readings from the sensor and a JSON api with the same information.

The LCD backlight can be turned on briefly by pressing the push button. By default, sensor readings will refresh every minute. If humidity readings have left the configured bounds, a webhook trigger will be called to either turn on or off the humidifier. The use of WiFi allows the controller to be placed a reasonable distance from the humidifier preventing inaccurate readings. It also allows the humidifier to be operated independantly of the controller.

Note: Please rename `settings.copy.h` to `settings.h` and populate it with appropriate configuration.

## Hardware

![Hardware Setup](/hardware.jpg)