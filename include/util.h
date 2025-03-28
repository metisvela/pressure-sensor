#ifndef UTIL_H
#define UTIL_H

#include <LiquidCrystal_I2C.h>

void printData(LiquidCrystal_I2C &lcd, float pressure, float decay_rate);
float decay_rate(float pressure, float current_time);
float total_decay(float pressure, float current_time);

#endif