/* ========================================
 *
 * @author  Pavel Arefyev
 * @name    Moisture sensor abstraction
 * @company Metropolia University of Applied Sciences
 * @date    14.05.2022
 * 
 * This abstraction provides interface for handling moisture sensor.
 * Datasheet: https://media.digikey.com/pdf/Data%20Sheets/DFRobot%20PDFs/SEN0193_Web.pdf
 *
 * It uses analog input on the microcontroller to provide moisture reading.
 * The analog input emulates Arduino's ADC converter because the sensor was originally
 * developed for Arduino UNO and DFRobot's shield.
 *
 * There is no transfer function for the sensor, thus, the oisture is obtained by
 * performing linear mapping from ADC range to percentage.
 *
 * NOTE: Moisture sensor required calibration.
 * MOIST_VALUE_MV is sensor's reading when exposed to water.
 * DRY_VALUE_MV is sensor's reading when exposed to air.
 *
 * ========================================
*/


#ifndef MOISTURE_SENSOR_H
#define MOISTURE_SENSOR_H
    
#include "project.h"

#define MOIST_VALUE_MV 1500  // Adjust this according to your sensor
#define DRY_VALUE_MV   2700  // Adjust this according to your sensor
#define HUMID_MIN      0
#define HUMID_MAX      100

void initialize_soil_moisture_sensor();
int  get_soil_moisture();

    
#endif

/* [] END OF FILE */
