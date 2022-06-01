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


#ifndef MOISTURE_SENSOR_H
#define MOISTURE_SENSOR_H
    
#include "project.h"

#define MOIST_VALUE_MV 500
#define DRY_VALUE_MV   1500
#define HUMID_MIN      0
#define HUMID_MAX      100

void initialize_soil_moisture_sensor();
int get_soil_moisture();

    
#endif

/* [] END OF FILE */
