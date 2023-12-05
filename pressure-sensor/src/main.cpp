#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <stdexcept>
#include <vector>

// -------------------------- Configuration -------------------------- //

#define ADC_NUM_READINGS            20
#define ADC_READING_DELAY_MS        20

#define IN_PIN              34    // Pin used for the ADC
#define SDA_PIN             25    // Pins used to control the LCD over the I2C bus
#define SCL_PIN             27
#define V_REF               3300  // Reference voltage [in mV] of the ADC
#define N_BIT               12    // Resolution of the ADC: 12-bit on ESP32-based boards
// #define ADC_MIN_VOLTAGE     0     // Min voltage [in mV] read by the ADC
// #define ADC_MAX_VOLTAGE     3300  // Max voltage [in mV] read by the ADC
#define SENS_ATM_VOLTAGE    4460  // Voltage [in mV] returned by the pressure sensor, at atmospheric pressure
#define SENS_MIN_VOLTAGE    500   // Min voltage [in mV] returned by the sensor
#define SENS_MAX_VOLTAGE    5000  // Max voltage [in mV] returned by the sensor
#define ATM_PRESSURE        0       // Pressure value [in Pa] read by the sensor at atmospheric pressure
#define MIN_PRESSURE        -98070  // Minimum pressure value [in Pa] read by the sensor
#define PRECISION           80
#define MOV_AVE_CAPACITY    5     // Number of samples used in the moving average
#define LCD_ADDRESS         0x27  // I2C address of the LCD
#define LCD_COLUMS          16
#define LCD_ROWS            2

const int adc_min_value = 0;
const int adc_max_value = pow(2, N_BIT) - 1;
const int adc_min_voltage = 0;      // [mV]
const int adc_max_voltage = V_REF;  // [mV]

float readPressure();
float readTime();
void addToMovAverage(float val);
float getAverage();
void printData(LiquidCrystal_I2C &lcd, int pressure, int decay_rate);
void printData(int pressure, int decay_rate);

float init_time = 0; // time when minimum pressure is reached
float prev_time = 0;
float curr_time = 0;
// int init_pressure = 0;
float prev_pressure = 0;
float pressure = 0;
float min_actual_pressure = 0; // minimum pressure that is actually reached
float loss = 0;
float cumulative_loss = 0;
float cumulative_speed = 0;
float decay_rate = 0;
// float avg = 0;
std::vector<float> moving_average;

LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLUMS, LCD_ROWS);


void setup()
{
    prev_time = readTime();
    prev_pressure = readPressure();

    // Initialize the LCD
    Wire.begin(SDA_PIN, SCL_PIN);
    lcd.init();
    lcd.backlight();

    Serial.begin(115200); // DEBUG
}

void loop()
{
    curr_time = readTime();
    float delta_time = curr_time - prev_time;

    pressure = readPressure();
    if (pressure < min_actual_pressure)
    {
        min_actual_pressure = pressure;
        init_time = curr_time;
    }

    loss = pressure - prev_pressure;

    if (loss < 0 && abs(loss) < PRECISION)
    {
        loss = 0;
    }

    decay_rate = loss / delta_time; 
    if (loss >= 0)
    {
        cumulative_loss += loss;
    }
    else
    {
        cumulative_loss = 0;
        min_actual_pressure = 0;
        init_time = readTime();
    }

    cumulative_speed = cumulative_loss / (curr_time - init_time);
    
    prev_pressure = pressure;
    prev_time = curr_time;
    printData(lcd, pressure, decay_rate);
    // printData(pressure, decay_rate); // DEBUG
    delay(1000);
}

/**
 * Print decay_rate and pressure values on the LCD
 * 
 * LCD_I2C library: https://registry.platformio.org/libraries/marcoschwartz/LiquidCrystal_I2C
 */
void printData(LiquidCrystal_I2C &lcd, int pressure, int decay_rate)
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Pressure ");
    lcd.print(String(pressure));
    lcd.setCursor(0, 1);
    lcd.print("Decay r. ");
    lcd.print(String(decay_rate));
}

/**
 * Used for debugging purposes only.
 */
void printData(int pressure, int decay_rate)
{
    Serial.print("Pressure   ");
    Serial.println(String(pressure));
    Serial.print("Decay rate ");
    // if (decay_rate > 0)
    // {
    //     Serial.println(String(decay_rate));
    // }
    // else
    // {
    //     Serial.println("0");
    // }
    Serial.println(String(decay_rate));
}

/**
 * Read the pressure and return an average value
 * 
 * Prima, si fanno 20 letture molto ravvicinate del valore di tensione restituito dal sensore,
 * e si prende la media di queste letture; quindi si converte questo valore medio nel
 * corrispondente valore di pressione in Pa, salvato in `pressure`.
 * Il valore di `pressure` viene aggiunto alla media mobile.
 * Infine, la funzione restituisce il valore di pressione ottenuto dalla media mobile.
 * 
 * Il filtro a media mobile permette di ridurre gli eventuali errori, sia dovuti al sensore 
 * sia all'ADC, ottenendo un segnale più pulito, da cui poi si calcola il segnale `decay_rate`
 * (che è la sua derivata discreta).
 */
float readPressure()
{
    int ADC_input = 0;
    int in_pin_voltage = 0;
    float avg = 0;
    float pressure = 0;

	for (int i = 0; i < ADC_NUM_READINGS; i++)
    {
        ADC_input = analogRead(IN_PIN); 
        avg += ADC_input;
		delay(ADC_READING_DELAY_MS);
	}
    avg = avg / ADC_NUM_READINGS;
    in_pin_voltage = map(avg, adc_min_value, adc_max_value, adc_min_voltage, adc_max_voltage); //[ADC to mV]
    pressure = map(in_pin_voltage, SENS_MIN_VOLTAGE, SENS_ATM_VOLTAGE, MIN_PRESSURE, ATM_PRESSURE); // [mV to Pa]
    
    addToMovAverage(pressure);
    pressure = getAverage(); 
    // DEBUG
    // Serial.print("avg=");
    // Serial.print(avg);
    // Serial.print("  in_pin_voltage=");
    // Serial.print(in_pin_voltage);
    // Serial.print("  pressure=");
    // Serial.println(pressure);
    return pressure;
}

/**
 * Return the time since the board was turned on, in seconds
*/
float readTime()
{
    float time = millis();
    return time/1000;
}

void addToMovAverage(float val)
{
    if (moving_average.size() < MOV_AVE_CAPACITY)
    {
        moving_average.push_back(val);
    }
    else
    {
        moving_average.erase(moving_average.begin());
        moving_average.push_back(val);
    }
}

float getAverage()
{
    if (moving_average.size() == 0) 
    {
        throw std::runtime_error("No values in the moving average");
    }

    float sum = 0;
    for(float val : moving_average)
    {
        sum += val;
    }
    return sum / moving_average.size();
}
