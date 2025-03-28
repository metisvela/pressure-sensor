#include "util.h"
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

void printData(LiquidCrystal_I2C &lcd, float pressure, float decay_rate) {
    if(pressure>-30000){
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("NO VOID");
        lcd.setCursor(0, 1);
        lcd.print("Pres > -30000 mB ");
        return;
    }
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Pres ");
    lcd.print(pressure, 1);
    lcd.print(" mB");
    
    lcd.setCursor(0, 1);
    lcd.print("Decay ");
    lcd.print(decay_rate, 1);
    lcd.print(" mB/h");
}

float decay_rate(float pressure, float current_time) {
    static float past_time = current_time;
    static float past_pressure = pressure;

    float delta_time = current_time - past_time;
    float rate = 0.0f;

    if (delta_time > 0) {
        rate = (pressure - past_pressure) * 3600.0f / delta_time;
    }

    past_time = current_time;
    past_pressure = pressure;

    return rate;
}

float total_decay(float pressure, float current_time) {
    static float min_pressure = 1e30;
    static float min_time = 0;

    if (pressure < min_pressure) {
        min_pressure = pressure;
        min_time = current_time;
        return 0.0f;
    }
    return pressure - min_pressure;
}