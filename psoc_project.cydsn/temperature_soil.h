/* ========================================
 *
 * @author  Pavel Arefyev
 * @name    Soil temperature sensors interface abstraction
 * @company Metropolia University of Applied Sciences
 * @date    26.05.2022
 *
 * This abstraction is built on top of OneWire interface.
 * It is meant for DS18B20 temperature sensor devices, however, it can smoothly
 * operate with any OneWire enabled devices added to the bus.
 * Datasheet: https://datasheets.maximintegrated.com/en/ds/DS18B20.pdf
 *
 * During initialization, all the sensor found on the bus are stored in
 * devices_on_bus structure that is later accessed by public interface functions.
 * To alter number of sensor present on OneWire bus, change NUMBER_OF_SOIL_TEMP_SENSORS parameter.
 *
 * ========================================
*/

#ifndef TEMPERATURE_SOIL_H
#define TEMPERATURE_SOIL_H

    
#include "onewire.h"
    
#define NUMBER_OF_SOIL_TEMP_SENSORS 2

/* Structures and types */
typedef struct slaves_information {
    uint64_t rom_codes[NUMBER_OF_SOIL_TEMP_SENSORS];
} slaves_info;

/* Function declarations */
void  initialize_soil_temp_sensors();
float get_soil_temperature(uint8 sensor_index);
void  start_conversion_soil_temp_sensor(uint8 sensor_index);
    

#endif /* [] END OF FILE */
