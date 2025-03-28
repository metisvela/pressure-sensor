#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include "util.h"

// -------------------------- Configuration -------------------------- //
#define ADC_NUM_READINGS                200
#define ADC_READING_DELAY_MS            5
#define ADC_MIN                         490
#define ADC_MAX                         4095

#define SENSOR_PIN                      34
#define ATM_PRESSURE                    0     
#define MIN_PRESSURE                    -98070

#define SDA_PIN                         25
#define SCL_PIN                         27
#define LCD_ADDRESS                     0x27
#define LCD_COLUMNS                     16
#define LCD_ROWS                        2

#define LOOP_TIME_MS                    20
// ------------------------------------------------------------------- //

LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLUMNS, LCD_ROWS);

float readPressure();

void setup() {
    Wire.begin(SDA_PIN, SCL_PIN);
    lcd.init();
    lcd.backlight();
}

void loop() {
    float pressure = readPressure() / 100.0;
    float current_time = millis() / 1000.0;

    float dr = decay_rate(pressure, current_time);
    float total = total_decay(pressure, current_time);

    printData(lcd, pressure, dr);
    delay(LOOP_TIME_MS);
}

float readPressure() {
    float avg = 0;
    for (int i = 0; i < ADC_NUM_READINGS; i++) {
        avg += analogRead(SENSOR_PIN);
        delay(ADC_READING_DELAY_MS);
    }
    avg /= ADC_NUM_READINGS;

    return (avg - ADC_MIN) * (ATM_PRESSURE - MIN_PRESSURE) 
            / (ADC_MAX - ADC_MIN) + MIN_PRESSURE;
}