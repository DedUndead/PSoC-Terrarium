/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/

#ifndef TEMPERATURE_SOIL_H
#define TEMPERATURE_SOIL_H

    
#include "onewire.h"
    
#define NUMBER_OF_SENSORS   2

/* Structures and types */
typedef struct slaves_information {
    uint64_t rom_codes[NUMBER_OF_SENSORS];
} slaves_info;

/* Function declarations */
void  initialize_soil_temp_sensors();
float get_soil_temperature(uint8 sensor_index);
void  start_conversion_soil_temp_sensor(uint8 sensor_index);
    

#endif /* [] END OF FILE */
