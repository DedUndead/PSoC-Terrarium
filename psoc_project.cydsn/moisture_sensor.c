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

#include "moisture_sensor.h"

/*
 * @brief Initialize moisture sensor's hardware dependencies.
 * It includes ADC DeltaSigma.
 */
void initialize_soil_moisture_sensor()
{
    ADC_DelSig_Start();
    ADC_DelSig_StartConvert();
}

/*
 * @brief  Get moisute reading. Perform linear mapping.
 * @return Moisture in percents.
 */
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
