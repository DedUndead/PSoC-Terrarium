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

#include "moisture_sensor.h"

void initialize_soil_moisture_sensor()
{
    ADC_DelSig_Start();
    ADC_DelSig_StartConvert();
}

int get_soil_moisture()
{
    int16 raw = ADC_DelSig_CountsTo_mVolts(ADC_DelSig_GetResult16());
    
    /* Sanity check to filter extreme values */
    if      (raw > DRY_VALUE_MV)   raw = DRY_VALUE_MV;
    else if (raw < MOIST_VALUE_MV) raw = MOIST_VALUE_MV;
    
    /* Perform linear mapping */
    int moisture = (float)(raw - MOIST_VALUE_MV) / (DRY_VALUE_MV - MOIST_VALUE_MV) * HUMID_MAX;
    /* Reverse mapped value */
    moisture = HUMID_MAX - moisture;
    
    return moisture;
}

/* [] END OF FILE */
