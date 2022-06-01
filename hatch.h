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
