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

#ifndef HATCH_H
#define HATCH_H


#include "project.h"

#define HATCH_OPEN_TEMP_C 20   // Temperature in Celcius for which the hatch should open
#define HATCH_POS_MIN     1000 // Minimum position of the hatch in compare value units
#define HATCH_POS_MAX     2000 // Maximum position of the hatch in compare value units
#define HATCH_STEP        200

void inititialize_hatch();
void adjust_hatch(int16 temperature);


#endif /* [] END OF FILE */
