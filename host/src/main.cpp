#include <Arduino.h>
#include <sensor.h>

#define LED 9

Sensor sensor(false);
uint32_t ledOnTime;
float temperature;
float voltage;
uint32_t lastUpdateTime;
bool ledOn;

void onMessageReceived(const uint8_t *data, uint8_t length, uint8_t rssi)
{
    if (length != 4 || data[0] != 't')
        return;

    lastUpdateTime = millis();
    int16_t temp = data[1] | data[2] << 8;
    temperature = temp / 256.0;
    voltage = (data[3] + 100) / 100.0;
    Serial.print("Temp:");Serial.println(temperature);
    Serial.print("Batt:");Serial.println(voltage);

    digitalWrite(LED, HIGH);
    ledOnTime = millis();
    ledOn = true;
}

void setup()
{
    Serial.begin(9600);
    pinMode(LED, OUTPUT);
    sensor.init(1, 2, NULL, true, false);
    sensor.onMessage(onMessageReceived);
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
}
