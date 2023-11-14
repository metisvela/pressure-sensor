#include <Arduino.h>

// -------------------------- Configuration -------------------------- //

#define BATTERY_ADC_PIN 		    35
#define BATTERY_ADC_RESOLUTION 		4095
#define BATTERY_ADC_REF_VOLTAGE 	1.1
#define BATTERY_ESP32_REF_VOLTAGE	3.3
#define BATTERY_NUM_READINGS 	  	32
#define ADC_NUM_READINGS 	  	    20
#define BATTERY_READING_DELAY_MS	20
#define ADC_READING_DELAY_MS	    20

#define IN_PIN               34 // Change this to the pin you're using for the analog sensor
#define MAX_V                5.0   // The reference voltage used by the ADC
#define ADC_VALUES           4095 // Number of values of the ADC
#define MAX_VOLTAGE_MILLIS   4500  //??? non dovrebbe essere 4,5V e non 3,3V
#define MIN_VOLTAGE_MILLIS   500 // Minimum voltage
#define maxPressure          0.0 // The maximum pressure value that the sensor can read
#define minPressure          -98070    // The minimum pressure value that the sensor can read
#define PRECISION            80

int pressureRead();
float readSecs();
void printData();
float init_time = 0; // holds the time when minimum pressure is reached
float prev_time = 0;
float curr_time = 0;
int DAC_INPUT = 0;
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


void setup() {
    prev_time = readSecs();
    init_pressure = pressureRead();
    prev_pressure = init_pressure;
}

void loop() {
    curr_time = readSecs();
    float deltaTime = curr_time-prev_time;
    pressure = pressureRead();
    if (pressure < min_press_actual){
        min_press_actual = pressure;
        init_time = curr_time;
    } 
    
    loss = pressure-prev_pressure; 

    if(loss < 0 && abs(loss) < PRECISION){
        loss = 0;
    }

    decay_rate = loss/(deltaTime);  

    if(loss >= 0){
        cumulative_loss += loss;
    }else{
        cumulative_loss = 0;
        min_press_actual = 0;
        init_time = readSecs();
    }

    cumulative_speed = cumulative_loss/(curr_time-init_time);
    
    prev_pressure = pressure;
    prev_time = curr_time;
    printData();
    delay(1000);
}

void printData(){
    
}

int pressureRead(){
    avg = 0;
		for (int i = 0; i < ADC_NUM_READINGS; i++) {
            DAC_INPUT = analogRead(IN_PIN);

            //adc_reads[i]=DAC_INPUT;  

            avg += DAC_INPUT;
			delay(ADC_READING_DELAY_MS);
		}
        avg /= ADC_NUM_READINGS;
        sensorVoltage=map(avg, 0, ADC_VALUES, MIN_VOLTAGE_MILLIS, MAX_VOLTAGE_MILLIS); //[ADC to mV]
        pressure=map(sensorVoltage, MIN_VOLTAGE_MILLIS, MAX_VOLTAGE_MILLIS, minPressure, maxPressure); // [mV to Pa]
    return pressure;
}

float readSecs(){
    float time = millis();
    return time/1000;
}