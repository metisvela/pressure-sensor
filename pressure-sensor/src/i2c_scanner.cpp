#include <Arduino.h>
#include <Wire.h>

#define SDA_PIN   25  // Pins used to control the LCD over the I2C bus
#define SCL_PIN   27

void setup()
{
    Wire.begin(SDA_PIN, SCL_PIN);
    Serial.begin(115200);
}

void loop()
{
    int error;
    int address;
    int devices = 0;

    Serial.println("Devices found:");

    for (address = 1; address < 127; address++) 
    {
        Wire.beginTransmission(address);
        error = Wire.endTransmission();
    
        if (error == 0)
        {
            Serial.print("0x");
            if (address<16) 
                Serial.print("0");
            Serial.println(address, HEX);
            devices++;
        }
        else if (error==4) 
        {
            Serial.print("Unknown error at address 0x");
            if (address<16) 
                Serial.print("0");
            Serial.println(address, HEX);
        }    
    }
    
    if (devices == 0)
        Serial.println("No I2C devices found");
  
    delay(5000);           
}
