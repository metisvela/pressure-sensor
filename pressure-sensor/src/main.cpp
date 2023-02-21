#include <Arduino.h>
#include <SailtrackModule.h>
#include <freertos/queue.h>
#include <string>

// -------------------------- Configuration -------------------------- //
#define EEPROM_SIZE					4096

#define MQTT_PUBLISH_FREQ_HZ		1.5

#define BATTERY_ADC_PIN 		    35
#define BATTERY_ADC_RESOLUTION 		4095
#define BATTERY_ADC_REF_VOLTAGE 	1.1
#define BATTERY_ESP32_REF_VOLTAGE	3.3
#define BATTERY_NUM_READINGS 	  	32
#define BATTERY_READING_DELAY_MS	20

#define HX711_DOUT_PIN				25
#define HX711_SCK_PIN				27
// TODO: Adjust value
//taken from grafana, so no more
//#define LOADCELL_BASE_LOAD_KG		36.1 
#define LOADCELL_NUM_READING		10
//passed by grafana if diffent
#define DEFAULT_TIME				30000
#define g							9.80
#define DEFAULT_OFFSET				-31000
#define DEFAULT_DIVIDER				1
#define LOOP_TASK_INTERVAL_MS		1000 / MQTT_PUBLISH_FREQ_HZ

#define IN_PIN 34 // Change this to the pin you're using for the analog sensor
#define MAX_V 5.0   // The reference voltage used by the ADC
#define ADC_VALUES 4095 // Number of values of the ADC
#define MAX_VOLTAGE_MILLIS 3300 // Maximumm voltage
#define MIN_VOLTAGE_MILLIS 500 // Minimum voltage
#define maxPressure 0.0 // The maximum pressure value that the sensor can read
#define minPressure -98070    // The minimum pressure value that the sensor can read

int pressureRead();
float readSecs();
float init_time = 0;
float prev_time = 0;
float curr_time = 0;
int DAC_INPUT = 0;
int sensorVoltage = 0;
int init_pressure = 0;
int prev_pressure = 0;
int pressure = 0;
int cumulative_loss = 0;
float cumulative_speed = 0;



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
	//parso il json per prendere peso e tempo per la calibrazione
	/*void onMqttMessage(const char * topic,JsonObject payload) {
		if(strcmp(topic,"sensor/strain0/calibration")){
			StaticJsonDocument<STM_JSON_DOCUMENT_MEDIUM_SIZE> doc;
			//non serve deserialize
			deserializeJson(doc,payload);
			//come sono i mex mqtt??
			load_kg=doc["load"];
			cal_time=doc["time"];
			cal_status=true;
		}
	}*/

};


void setup() {
    pressSens.begin("pressure", IPAddress(192, 168, 42, 106), new ModuleCallbacks());
    prev_time = readSecs();
    init_time = prev_time;
    init_pressure = pressureRead();
    prev_pressure = init_pressure;
}

void loop() {
    pressure = pressureRead();
    curr_time = readSecs();
    int loss = pressure-prev_pressure;
    float deltaTime = curr_time-prev_time;
    float decay_rate = loss/(deltaTime);
    if (deltaTime < 0.0000001)
    {
        decay_rate = 0;
    }
    cumulative_loss += loss;
    cumulative_speed = cumulative_loss/(curr_time-init_time);
    prev_pressure = pressure;
    prev_time = curr_time;
    DynamicJsonDocument payload(500);
    payload["pressure"] = std::to_string(pressure);
    payload["decay_rate"] = std::to_string(decay_rate);
    payload["cumulative_loss"] = std::to_string(cumulative_loss);
    payload["cumulative_speed"] = std::to_string(cumulative_speed);
    pressSens.publish("sensor/pressure", payload.as<JsonObjectConst>());
    delay(1/MQTT_PUBLISH_FREQ_HZ*1000);
}

int pressureRead(){
    DAC_INPUT = analogRead(IN_PIN);
    sensorVoltage=map(DAC_INPUT, 0, ADC_VALUES, MIN_VOLTAGE_MILLIS, MAX_VOLTAGE_MILLIS); //[ADC to mV]
    pressure=map(sensorVoltage, MIN_VOLTAGE_MILLIS, MAX_VOLTAGE_MILLIS, minPressure, maxPressure); // [mV to Pa]
    return pressure;
}

float readSecs(){
    float time = millis();
    return time/1000;
}