/* ========================================
 *
 * @author  Pavel Arefyev
 * @name    Heater simulator interface
 * @company Metropolia University of Applied Sciences
 * @date    14.05.2022
 *
 * This interface provides an adjustment functions for heater simulator.
 * In this demo project, simple neon-red LED is used as a simulator.
 *
 * The heater is turned on when HEATER_ON_TEMP_C is reached or below.
 * The heater is turned back off once the temperature is above HEATER_OFF_TEMP_C.
 *
 * ========================================
*/

#include "heater.h"

static uint8 heater_on = 0;

/*
 * @brief Adjust heater according to the temperature
 * @param temperature Current temperature
 */
void adjust_heater(int16 temperature)
{
    if (temperature < HEATER_ON_TEMP_C && !heater_on) LED_Overheat_Write(1);
    if (temperature > HEATER_ON_TEMP_C && heater_on)  LED_Overheat_Write(0);
}


/* [] END OF FILE */
