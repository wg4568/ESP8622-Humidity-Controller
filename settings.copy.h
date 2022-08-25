// WiFi SSID and password
const char *SSID = "";
const char *PASSWORD = "";

// Target humidity range
float MIN_HUMIDITY = 60;
float MAX_HUMIDITY = 62;

// URLs for enabling/disabling humidifier (HTTP GET)
const String WEBHOOK_ON = "";
const String WEBHOOK_OFF = "";

// Other settings
const bool USE_FAHRENHEIT = true;
const int MAIN_LOOP_DELAY = 1;
const int SENSOR_REFRESH = 60 * 1000;
const int LCD_REFRESH = 500;
const int BACKLIGHT_DELAY = 8000;