#include <Arduino.h>
#include <sensor.h>
#include "sms.h"

#define LED 9
#define RELAY 6

#define RELAY_ON digitalWrite(RELAY, HIGH)
#define RELAY_OFF digitalWrite(RELAY, LOW)
#define IS_RELAY_ON digitalRead(RELAY)

#define RELAY_TIMEOUT 3610000UL //1 hour + 10 sec

#define MODE_OFF 0
#define MODE_ON 1
#define MODE_AUTO 2

void updateRelay();
Sensor sensor(false);
uint32_t ledOnTime;
bool ledOn;
int16_t temperature = 1000;
int16_t voltage = 0;
int16_t setTemperature;
uint32_t lastUpdateTime = 0;
SMS sms;
uint8_t mode = MODE_OFF;

#define EEPROM_SET_ADDR ((uint8_t *)0)
#define EEPROM_MODE_ADDR ((uint8_t *)1)

void onMessageReceived(const uint8_t *data, uint8_t length, uint8_t rssi)
{
    if (length != 4 || data[0] != 't')
        return;

    lastUpdateTime = millis();
    int16_t temp = data[1] | data[2] << 8;
    temperature = temp / 25.6;
    voltage = (data[3] + 100) / 10.0;
    digitalWrite(LED, HIGH);
    ledOnTime = millis();
    ledOn = true;

    updateRelay();
}

void onSmsReceived(char *from, char *text)
{
    if (strcasecmp_P(text, PSTR("on")) == 0)
    {
        mode = MODE_ON;
        eeprom_update_byte(EEPROM_MODE_ADDR, mode);
    }
    else if (strcasecmp_P(text, PSTR("off")) == 0)
    {
        mode = MODE_OFF;
        eeprom_update_byte(EEPROM_MODE_ADDR, mode);
    }
    else if (strcasecmp_P(text, PSTR("auto")) == 0)
    {
        mode = MODE_AUTO;
        eeprom_update_byte(EEPROM_MODE_ADDR, mode);
    }
    else
    {
        uint8_t temp = atoi(text);
        if (temp >= 4 && temp <= 40)
        {
            setTemperature = temp * 10;
            eeprom_update_byte(EEPROM_SET_ADDR, temp);
        }
    }

    updateRelay();

    uint8_t rssi = sms.getRSSI();

    char modeStr[15];
    const char *state = IS_RELAY_ON ? "ON" : "OFF";
    if (mode == MODE_AUTO)
        snprintf_P(modeStr, sizeof(modeStr), PSTR("AUTO (%s)"), state);
    else
        strcpy(modeStr, state);

    char message[100];
    int minutes = (millis() - lastUpdateTime) / 60000;
    snprintf_P(message, sizeof(message),
               PSTR("%s\nTemp %s%d.%d C\nSet %d.%d C\nBat %d.%d V\nAcum %d min\nRSSI %d"),
               modeStr,
               temperature < 0 ? "-" : "", temperature / 10, temperature % 10,
               setTemperature / 10, setTemperature % 10,
               voltage / 10, voltage % 10,
               minutes, rssi);
    sms.sendSMS(from, message);
}

void setup()
{
    setTemperature = eeprom_read_byte(EEPROM_SET_ADDR) * 10;
    mode = eeprom_read_byte(EEPROM_MODE_ADDR);

    Serial.begin(57600);
    pinMode(LED, OUTPUT);
    pinMode(RELAY, OUTPUT);
    RELAY_OFF;

    sensor.sleep(5);

    sms.beginSms();
    sms.onSmsReceived(onSmsReceived);

    sensor.init(1, 2, NULL, true, false);
    sensor.onMessage(onMessageReceived);

    updateRelay();
}

void updateRelay()
{
    switch (mode)
    {
    case MODE_OFF:
        RELAY_OFF;
        break;
    case MODE_ON:
        RELAY_ON;
        break;
    case MODE_AUTO:
        uint32_t now = millis();
        if (IS_RELAY_ON)
        {
            if (now - lastUpdateTime > RELAY_TIMEOUT ||
                temperature >= setTemperature + 5)
            {
                RELAY_OFF;
            }
        }
        else
        {
            if (temperature <= setTemperature - 5)
            {
                RELAY_ON;
            }
        }
        break;
    }
}

void loop()
{
    uint32_t now = millis();
    if (ledOn && now - ledOnTime > 100)
    {
        digitalWrite(LED, LOW);
        ledOn = false;
    }
    sensor.update();
    sms.loop();

    static uint32_t lastCheck = 0;
    if (now - lastCheck > 5000)
    {
        updateRelay();
        lastCheck = now;
    }
}
