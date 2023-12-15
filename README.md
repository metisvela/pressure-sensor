# Pressure Sensor

This device utilizes an Arduino board to interface with a pressure sensor via an ADC pin and displays pressure and decay rate values on an LCD screen.

## Components Used

- **ESP32**
- **self built pressure sensor**: Analog sensor providing voltage readings.
- **LCD Display**

## Functions

- **`setup()`**: Initializes the LCD and serial communication for debugging.
- **`loop()`**: Reads pressure values, calculates decay rates, and updates LCD display.
- **`readPressure()`**: Reads ADC input, converts it to pressure values, and applies a moving average filter.
- **`readTime()`**: Returns time since board power-up in seconds.
- **`printData()`**: Prints pressure and decay rate values on the LCD or serial interface (for debugging).
- **`addToMovAverage()`, `getAverage()`**: Functions to maintain and calculate moving average of pressure values.

## Instructions

1. **Hardware Setup**: Connect the pressure sensor to the designated ADC pin.
2. **I2C Connection**: Wire the LCD using SDA and SCL pins for communication.
3. **Upload Script**: Use Arduino IDE or PlatformIO to upload the script to the board.
4. **Observation**: View pressure and decay rate values on the LCD screen.

## Additional Notes

- Adjust constants and configurations based on sensor specifications.
- Enable/disable debugging serial prints by commenting/uncommenting lines.
- Ensure proper power supply and sensor calibration for accurate readings.
  