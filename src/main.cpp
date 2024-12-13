#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <stdexcept>
#include <vector>
#include <deque>
#include <utility>

// -------------------------- Configuration -------------------------- //

#define ADC_NUM_READINGS                200
#define ADC_READING_DELAY_MS            5
#define ADC_MIN                         490  // 1bar ADC reading
#define ADC_MAX                         4095

// We are showing dacay rate in DECAY_RATE_TIME ms future
#define DECAY_RATE_TIME                 3600000

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
void printData(LiquidCrystal_I2C &lcd, float pressure, float decay_rate);
float decay_rate(float pressure, float current_time);
float total_decay(float pressure, float current_time);

void setup() {
    Wire.begin(SDA_PIN, SCL_PIN);
    lcd.init();
    lcd.backlight();
}

void loop() {
    float pressure = readPressure()/100;
    float current_time = millis()/1000;

    float dr = decay_rate(pressure, current_time);
    float decay = total_decay(pressure, current_time); 

    printData(lcd, pressure, dr);
    delay(LOOP_TIME_MS);
}

void printData(LiquidCrystal_I2C &lcd, float pressure, float decay_rate) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Pres ");
    lcd.print(String(pressure));
    lcd.print(" mB");
    lcd.setCursor(0, 1);
    lcd.print("Decay ");
    lcd.print(String(decay_rate));
    lcd.print(" mB/h");
}

float readPressure() {
    float avg = 0;

	for (int i = 0; i < ADC_NUM_READINGS; i++) {
        avg += analogRead(SENSOR_PIN);
		delay(ADC_READING_DELAY_MS);
    }
    avg = avg/ADC_NUM_READINGS;

    return map(avg, ADC_MIN, ADC_MAX, MIN_PRESSURE, ATM_PRESSURE);
}

float decay_rate(float pressure, float current_time) {
    static float past_time = current_time;
    static float past_pressure = pressure;

    float decay = (pressure - past_pressure)*DECAY_RATE_TIME / (1000*(current_time - past_time)); 
    
    past_time = current_time;
    past_pressure = pressure;

    return decay;
}

float total_decay(float pressure, float current_time){
    static float min_pressure = INT_MAX;
    static float min_pressure_time = 0;

    if(pressure<min_pressure){
        min_pressure = pressure;
        min_pressure_time = current_time;
        return 0.0f;
    }else{
        return min_pressure - pressure;
    }
}