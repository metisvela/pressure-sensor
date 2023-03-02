#include <Arduino.h>
#include <SailtrackModule.h>
#include <freertos/queue.h>

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
#define MAX_VOLTAGE_MILLIS   3300 // Maximumm voltage
#define MIN_VOLTAGE_MILLIS   500 // Minimum voltage
#define maxPressure          0.0 // The maximum pressure value that the sensor can read
#define minPressure          -98070    // The minimum pressure value that the sensor can read

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


SailtrackModule pressSens;

class ModuleCallbacks: public SailtrackModuleCallbacks {
	void onStatusPublish(JsonObject status) {
		JsonObject battery = status.createNestedObject("battery");
		float avg = 0;
		for (int i = 0; i < BATTERY_NUM_READINGS; i++) {
			avg += analogRead(BATTERY_ADC_PIN) / BATTERY_NUM_READINGS;
			delay(BATTERY_READING_DELAY_MS);
		}
		battery["voltage"] = 2 * avg / BATTERY_ADC_RESOLUTION * BATTERY_ESP32_REF_VOLTAGE * BATTERY_ADC_REF_VOLTAGE;
	}
};

void setup() {
    pressSens.begin("pressure", IPAddress(192, 168, 42, 106), new ModuleCallbacks());
    prev_time = readSecs();
    //init_time = prev_time;
    init_pressure = pressureRead();
    prev_pressure = init_pressure;
}

void loop() {

    curr_time = readSecs();
    pressure = pressureRead();

    if (pressure < min_press_actual){
        min_press_actual = pressure;
        init_time = curr_time;
    }
        
    int loss = pressure-prev_pressure;
    float deltaTime = curr_time-prev_time;
    
    decay_rate = loss/(deltaTime);  // Pas/s
    
    loss > 0 ? cumulative_loss += loss : (cumulative_loss = 0, min_press_actual = 0, init_time = 0);  // debug
    
    cumulative_speed = cumulative_loss/(curr_time-init_time);
    
    prev_pressure = pressure;
    prev_time = curr_time;
    printData();
    delay(500);
}

void printData(){
    StaticJsonDocument<STM_JSON_DOCUMENT_MEDIUM_SIZE> pressureJson;
	pressureJson["pressure"] = pressure;
    decay_rate > 0 ? pressureJson["decay_rate"] = decay_rate : pressureJson["decay_rate"] = 0;
    pressureJson["cumulative_loss"] = cumulative_loss * 100 / (-min_press_actual+0.01);
    pressureJson["cumulative_speed"] = cumulative_speed;
	pressSens.publish("sensor/pressure0", pressureJson.as<JsonObjectConst>());
}

int pressureRead(){
    float avg = 0;
		for (int i = 0; i < ADC_NUM_READINGS; i++) {
            DAC_INPUT = analogRead(IN_PIN);
            avg += DAC_INPUT / ADC_NUM_READINGS;
			delay(ADC_READING_DELAY_MS);
		}
        sensorVoltage=map(avg, 0, ADC_VALUES, MIN_VOLTAGE_MILLIS, MAX_VOLTAGE_MILLIS); //[ADC to mV]
        pressure=map(sensorVoltage, MIN_VOLTAGE_MILLIS, MAX_VOLTAGE_MILLIS, minPressure, maxPressure); // [mV to Pa]
    return pressure;
}

float readSecs(){
    float time = millis();
    return time/1000;
}