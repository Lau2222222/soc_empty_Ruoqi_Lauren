/*
 * temperature.c
 *
 *  Created on: 10 nov. 2023
 *      Author: Lauren
 */

#include "temperature.h"
#include "math.h"

int16_t convTempBLE(float tempC){
  if (tempC < -273.15 || tempC > 327.67){ //Test if the tempC is in the allowed range of temperatures
      return 0x8000; //Code for unknown values
  }
  return (int16_t)(tempC*100);// tempbl =tempC*pow(10,-2)
}


sl_status_t getconvTemptoBLE(int16_t *TempBLE){
  uint32_t rh; //relative humidity value
  int32_t t;  // temperature value
  sl_status_t status = sl_sensor_rht_get(&rh, &t);
  if (status == SL_STATUS_OK){ //check if the reading of the temperature is successful
      *TempBLE=convTempBLE((float)t/1000.0); //converts the temperature to BLE format thanks to convTempBLE function
                                            // And assigns it to the value at the address pointed by *TempBLE pointer
  }
  else{
      *TempBLE=0x8000; //if the reading isn't successful assigns "value unknown" to TempBLE
  }
  return status;
}


