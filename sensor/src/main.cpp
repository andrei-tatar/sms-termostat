#include <Arduino.h>
#include <sensor.h>
#include "tcn75a.h"

#define LED 9

Sensor sensor(false);
Tcn75a temp;

#define SLEEP_TIME_SEC (60 * 10UL)
#define MAX_TIME_MS (3UL * SLEEP_TIME_SEC * 1000)

void setup()
{
    sensor.sleep(1);
    temp.begin();
    sensor.init(2, 1, NULL, true, false);
    pinMode(LED, OUTPUT);
}

void loop()
{
    static int16_t lastTemperature = 0xFFFF;
    static uint32_t lastSend;

    int16_t temperature = temp.read();
    uint32_t now = millis();

    if (temperature != lastTemperature || now - lastSend >= MAX_TIME_MS)
    {
        lastTemperature = temperature;
        lastSend = now;

        uint8_t msg[4];
        msg[0] = 't';
        msg[1] = temperature;
        msg[2] = temperature >> 8;
        msg[3] = sensor.readVoltage() / 10 - 100;

        sensor.powerUp();
        if (sensor.sendAndWait(msg, sizeof(msg)))
        {
            digitalWrite(LED, HIGH);
            delay(100);
            digitalWrite(LED, LOW);
        }
        sensor.powerDown();
    }

    sensor.sleep(SLEEP_TIME_SEC);
    lastSend -= SLEEP_TIME_SEC * 1000; //compensate sleep time
}
