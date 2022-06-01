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

#include "project.h"
#include "hatch.h"

/*
 * @brief Initialize hatch and related hardware
 */
void inititialize_hatch()
{
    PWM_Start();
}

/*
 * @brief Adjust hatch according to the current air temperature
 * @param temperature Current air temperature
 */
void adjust_hatch(int16 temperature)
{
    uint16_t new_compare_value;
    
    new_compare_value = HATCH_POS_MIN + (temperature - HATCH_OPEN_TEMP_C) * HATCH_STEP;
    
    // Safety checks to prevent breaking of hatch
    if (new_compare_value > HATCH_POS_MAX)      new_compare_value = HATCH_POS_MAX;
    else if (new_compare_value < HATCH_POS_MIN) new_compare_value = HATCH_POS_MIN;
    
    PWM_WriteCompare(new_compare_value);
}

/* [] END OF FILE */
