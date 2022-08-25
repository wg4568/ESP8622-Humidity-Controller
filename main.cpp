#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <DHT_U.h>
#include <DHT.h>
#include <ArduinoJson.h>
#include <LiquidCrystal_I2C.h>
#include "settings.h"

float LAST_HUMIDITY, LAST_TEMPERATURE, LAST_HEATINDEX;
int LAST_HOOK_STATUS = 999;
bool HUMIDIFIER_STATUS;

DHT dht(D7, DHT22);
LiquidCrystal_I2C lcd(0x27, 20, 4);
int lcdButton = D6;

ESP8266WebServer server(80);
WiFiClient wifiClient;
HTTPClient http;
uint frame = 0;
uint backlightTimer = 0;

byte CHAR_WIFI[] = {
    B00000,
    B01110,
    B10001,
    B00100,
    B01010,
    B00000,
    B00100,
    B00000};

byte CHAR_STEAM[] = {
    B10101,
    B01010,
    B10101,
    B01010,
    B00100,
    B01110,
    B01110,
    B01110};

byte CHAR_BELL[] = {
  B00100,
  B01110,
  B01110,
  B01110,
  B11111,
  B00000,
  B00100,
  B00000
};

void setup()
{
    dht.begin();

    lcd.init();
    lcd.noBacklight();
    lcd.createChar(0, CHAR_WIFI);
    lcd.createChar(1, CHAR_STEAM);
    lcd.createChar(2, CHAR_BELL);
    display();

    pinMode(lcdButton, INPUT);

    Serial.begin(115200);
    Serial.print("\nConnecting to WiFi...");

    WiFi.begin(SSID, PASSWORD);

    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(200);
    }

    Serial.print("\nConnected to ");
    Serial.println(SSID);
    Serial.print("Local IP: ");
    Serial.println(WiFi.localIP());

    server.on("/", _index);
    server.on("/data.json", _data);

    server.begin();
    Serial.println("HTTP server started");

    hygrometer();
}

void loop()
{
    if (frame % SENSOR_REFRESH == 0)
    {
        hygrometer();
        calculate();
    }

    if (frame % LCD_REFRESH == 0)
    {
        display();
    }

    if (digitalRead(lcdButton))
    {
        backlightTimer = frame;
    }

    server.handleClient();

    delay(MAIN_LOOP_DELAY);
    frame += MAIN_LOOP_DELAY;
}

void display()
{
    if (frame - backlightTimer < BACKLIGHT_DELAY)
    {
        lcd.backlight();
    }
    else
    {
        lcd.noBacklight();
    }

    // WiFi Status
    lcd.setCursor(0, 0);
    lcd.write(0);
    if (WiFi.status() == WL_CONNECTED)
    {
        lcd.print(" Online ");
    }
    else
    {
        lcd.print(" Offline");
    }

    // Humidifier Status
    lcd.setCursor(10, 0);
    lcd.write(1);

    if (HUMIDIFIER_STATUS) {
        lcd.printf(" On ");
    } else {
        lcd.printf(" Off");
    }

    // Humidity and Temp
    String units = "C";

    if (USE_FAHRENHEIT)
    {
        units = "F";
    }

    lcd.setCursor(0, 2);
    lcd.printf("%.2f%%   ", LAST_HUMIDITY);

    lcd.setCursor(0, 3);
    lcd.printf("%.2f%s   ", LAST_TEMPERATURE, units);
    
    // Webhook Status
    lcd.setCursor(13, 3);
    lcd.print(LAST_HOOK_STATUS);

    if (LAST_HOOK_STATUS == 200)
    {
        lcd.print(" OK ");
    }
    else
    {
        lcd.print(" ERR");
    }

    // Refresh countdown
    int cd = (SENSOR_REFRESH - (frame % SENSOR_REFRESH)) / 1000;
    lcd.setCursor(16, 0);
    lcd.write(2);
    lcd.printf(" %-2d", cd);
}

void hygrometer()
{
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature(USE_FAHRENHEIT);

    if (isnan(humidity) || isnan(temperature))
    {
        Serial.println("Could not read from DHT sensor");
        return;
    }

    float heatIndex = dht.computeHeatIndex(temperature, humidity, USE_FAHRENHEIT);

    Serial.print("Got readings: ");
    Serial.print(humidity);
    Serial.print("% ");
    Serial.print(temperature);

    if (USE_FAHRENHEIT)
    {
        Serial.println("°F");
    }
    else
    {
        Serial.println("°C");
    }

    LAST_HUMIDITY = humidity;
    LAST_TEMPERATURE = temperature;
    LAST_HEATINDEX = heatIndex;
}

void humidifier(bool newStatus)
{
    if (newStatus == HUMIDIFIER_STATUS)
    {
        return;
    }

    if (newStatus)
    {
        http.begin(wifiClient, WEBHOOK_ON);
        Serial.print("Enabling humidifier, status: ");
    }
    else
    {
        http.begin(wifiClient, WEBHOOK_OFF);
        Serial.print("Disabling humidifer, status: ");
    }

    int code = http.GET();
    Serial.println(code);

    if (code != 200)
    {
        Serial.println(http.getString());
    }

    LAST_HOOK_STATUS = code;
    HUMIDIFIER_STATUS = newStatus;

    http.end();
}

void calculate()
{
    if (LAST_HUMIDITY < MIN_HUMIDITY)
    {
        humidifier(true);
    }

    if (LAST_HUMIDITY > MAX_HUMIDITY)
    {
        humidifier(false);
    }
}

void makeIndex(char *buffer, String units, int interval)
{
    sprintf(buffer,
            "<!DOCTYPE html> <html lang=en-US > <title>Hygrometer Server</title> <meta name=v"
            "iewport  content=\"width=device-width, initial-scale=1\" /> <meta charset=utf-8 "
            " /> <script src=\"https://code.jquery.com/jquery-3.6.0.min.js\" integrity=\"sha2"
            "56-/xUj+3OJU5yExlq6GSYGSHk7tPXikynS7ogEvDej/m4=\" crossorigin=anonymous  ></scri"
            "pt> <style> .red { color: red; } .nm { margin: 0px; } </style> <h3>Hygrometer Se"
            "rver</h3> <p class=nm > Temperature: <span class=red > <span id=temperature >--."
            "--</span>&#176;%s </span> </p> <p class=nm > Humidity: <span class=red ><span id"
            "=humidity >--.--</span>%%</span> </p> <p> Heat Index: <span class=red > <span id"
            "=heatIndex >--.--</span>&#176;%s </span> </p> <p>View raw data: <a href=\"/data."
            "json\">data.json</a></p> <script> var interval = %u; function update() { $.get(\""
            "/data.json\", (data) => { console.log(data); $(\"#temperature\").text(data.tempe"
            "rature.toFixed(2)); $(\"#heatIndex\").text(data.heatIndex.toFixed(2)); $(\"#humi"
            "dity\").text(data.humidity.toFixed(2)); }); } $.get(\"/data.json\", (data) => { "
            "console.log(data); $(\"#temperature\").text(data.temperature.toFixed(2)); $(\"#h"
            "eatIndex\").text(data.heatIndex.toFixed(2)); $(\"#humidity\").text(data.humidity"
            ".toFixed(2)); }); console.log(`Initialized with refresh interval of ${interval}m"
            "s`); setInterval(update, interval); update(); </script>",
            units, units, interval);
}

void makeJsonData(char *buffer)
{
    DynamicJsonDocument message(1024);

    message["humidity"] = LAST_HUMIDITY;
    message["temperature"] = LAST_TEMPERATURE;
    message["heatIndex"] = LAST_HEATINDEX;
    message["refresh"] = SENSOR_REFRESH - (frame % SENSOR_REFRESH);
    message["humidifier"] = HUMIDIFIER_STATUS;

    if (USE_FAHRENHEIT)
    {
        message["units"] = "FAHRENHEIT";
    }
    else
    {
        message["units"] = "CELCIUS";
    }

    serializeJson(message, buffer, 1024);
}

void _index()
{
    String unit = "C";
    if (USE_FAHRENHEIT)
        unit = "F";

    char buffer[1500];
    makeIndex(buffer, unit, SENSOR_REFRESH);

    server.send(200, "text/html", buffer);
}

void _data()
{
    char buffer[200];
    makeJsonData(buffer);

    server.send(200, "application/json", buffer);
}