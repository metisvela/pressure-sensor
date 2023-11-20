#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

// -------------------------- Configuration -------------------------- //

// #define BATTERY_ADC_PIN             35
// #define BATTERY_ADC_RESOLUTION      4095
// #define BATTERY_ADC_REF_VOLTAGE     1.1
// #define BATTERY_ESP32_REF_VOLTAGE   3.3
// #define BATTERY_NUM_READINGS        32
#define ADC_NUM_READINGS            20
// #define BATTERY_READING_DELAY_MS    20
#define ADC_READING_DELAY_MS        20

#define IN_PIN               34    // Change this to the pin you're using for the analog sensor
#define MAX_V                5.0   // The reference voltage used by the ADC
// The ADC has a 12-bits resolution: return values are in the range 0-4095
#define ADC_MIN_VALUE        0     // Min value returned by the ADC
#define ADC_MAX_VALUE        4095  // Max value returned by the ADC
#define MAX_VOLTAGE_MILLIS   4500  //??? non dovrebbe essere 4,5V e non 3,3V
#define MIN_VOLTAGE_MILLIS   500   // Minimum voltage
#define MAX_PRESSURE         0.0   // The maximum pressure value that the sensor can read
#define MIN_PRESSURE         -98070    // The minimum pressure value that the sensor can read
#define PRECISION            80
#define LCD_ADDRESS          0x27
#define LCD_COLUMS           16
#define LCD_ROWS             2

int pressureRead();
float readSecs();
void printData(LiquidCrystal_I2C& lcd, int pressure, float decay_rate);
float init_time = 0; // holds the time when minimum pressure is reached
float prev_time = 0;
float curr_time = 0;
int ADC_input = 0;
int sensorVoltage = 0;
int init_pressure = 0;
int prev_pressure = 0;
int pressure = 0;
int cumulative_loss = 0;
float cumulative_speed = 0;
float decay_rate = 0;
int min_press_actual = 0; // minima pressione realmente raggiunta
int loss = 0;
float avg = 0;

LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLUMS, LCD_ROWS);

// Debug: uncomment to print the ADC values
//int adc_reads[ADC_NUM_READINGS];

void setup() {
    prev_time = readSecs();
    init_pressure = pressureRead();
    prev_pressure = init_pressure;

    // Initialize the lcd
    lcd.init();
    lcd.backlight();
}

void loop() {
    curr_time = readSecs();
    float deltaTime = curr_time-prev_time;
    pressure = pressureRead();
    if (pressure < min_press_actual) {
        min_press_actual = pressure;
        init_time = curr_time;
    } 
    
    loss = pressure-prev_pressure; 

    if (loss < 0 && abs(loss) < PRECISION) {
        loss = 0;
    }

    decay_rate = loss/(deltaTime);  

    if (loss >= 0) {
        cumulative_loss += loss;
    } else {
        cumulative_loss = 0;
        min_press_actual = 0;
        init_time = readSecs();
    }

    cumulative_speed = cumulative_loss / (curr_time-init_time);
    
    prev_pressure = pressure;
    prev_time = curr_time;
    printData(lcd, pressure, decay_rate);
    delay(1000);
}

void printData(LiquidCrystal_I2C& lcd, int pressure, float decay_rate) {
    // LCD_I2C library: https://registry.platformio.org/libraries/marcoschwartz/LiquidCrystal_I2C
    // Print decay_rate and pressure values on the LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Pressure ");
    lcd.print(String(pressure));
    lcd.setCursor(0, 1);
    lcd.print("Decay r. ");
    lcd.print(String(decay_rate));
}

int pressureRead() {
    avg = 0;
		for (int i = 0; i < ADC_NUM_READINGS; i++) {
            ADC_input = analogRead(IN_PIN);
            //adc_reads[i] = ADC_input;  
            avg += ADC_input / ADC_NUM_READINGS;
			delay(ADC_READING_DELAY_MS);
		}
        sensorVoltage = map(avg, ADC_MIN_VALUE, ADC_MAX_VALUE, MIN_VOLTAGE_MILLIS, MAX_VOLTAGE_MILLIS); //[ADC to mV]
        pressure = map(sensorVoltage, MIN_VOLTAGE_MILLIS, MAX_VOLTAGE_MILLIS, MIN_PRESSURE, MAX_PRESSURE); // [mV to Pa]
    return pressure;
}

float readSecs() {
    float time = millis();
    return time/1000;
}
