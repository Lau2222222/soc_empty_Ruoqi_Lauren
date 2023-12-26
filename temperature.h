/*
 * temperature.h
 *
 *  Created on: 10 nov. 2023
 *      Author: Lauren
 */

#ifndef TEMPERATURE_H_
#define TEMPERATURE_H_

#include "sl_sensor_rht.h" //to have access to the sensor functions
#include "stdint.h" //ensures that the int16_t type is properly defined

//Initialization of the functions to convert the temperature in Celcius to BLE format

int16_t convTempBLE(float tempC); //will do the practical conversion and return a 16 bit integer
                                  // conform to the temperature specifications
sl_status_t getconvTemptoBLE(int16_t *TempBLE); //will return a status to indicate if the conversion is successful

#endif /* TEMPERATURE_H_ */
