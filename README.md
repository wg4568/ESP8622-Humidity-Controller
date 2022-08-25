# ESP8622 Humidity Controller

ESP8622 sketch for remotely controlling a humidifier using webhooks. Uses a DHT humidity sensor, NodeMCU ESP8622 microcontroller and an I2C LCD display. Humidifier is controlled with a Kasa smart switch and [IFTTT](https://ifttt.com/) webhooks. The sketch also hosts a simple webserver providing most recent readings from the sensor and a JSON api with the same information.

Please rename `settings.copy.h` to `settings.h` and populate it with appropriate configuration.

![Hardware Setup](/hardware.jpg)