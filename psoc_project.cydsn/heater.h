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

#ifndef HEATER_H
#define HEATER_H
    

#include "project.h"
    
#define HEATER_ON_TEMP_C  5
#define HEATER_OFF_TEMP_C 10
    
void adjust_heater(int16 temperature);
    

#endif

/* [] END OF FILE */
