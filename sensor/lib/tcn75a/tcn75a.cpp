#include <Arduino.h>
#include <Wire.h>
#include "tcn75a.h"

#define Addr 0x48

void Tcn75a::begin()
{
    // Initialise I2C communication as Master
    Wire.begin();

    Wire.beginTransmission(Addr);
    Wire.write(0x01);
    Wire.write(0x01); //shut down
    Wire.endTransmission();
}

int16_t Tcn75a::read()
{
    int16_t result;

    Wire.beginTransmission(Addr);
    Wire.write(0x01);
    Wire.write(0x81); //one conversion
    Wire.endTransmission();

    delay(35);

    Wire.beginTransmission(Addr);
    Wire.write(0x00); //select temp register
    Wire.endTransmission();

    Wire.requestFrom(Addr, 2);
    if (Wire.available() == 2)
    {
        result = Wire.read() << 8;
        result |= Wire.read();
    }

    if (result & 0x8000)
    {
        result = -(result & ~0x8000);
    }

    return result;
}