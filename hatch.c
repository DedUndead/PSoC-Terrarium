/* ========================================
 *
 * @author  Pavel Arefyev
 * @name    Hatch simulator interface
 * @company Metropolia University of Applied Sciences
 * @date    14.05.2022
 *
 * This interface provides an adjustment functions for hatch simulator.
 * In this demo project, 9g servo is used as a simulator.
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
