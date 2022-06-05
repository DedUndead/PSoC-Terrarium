/******************************************************************************
* 
* @author  Pavel Arefyev
* @name    TX00DB04 - Programmable System on Chip Design. Moving Average Filter
* @company Metropolia University of Applied Sciences
* @date    06.04.2022
*
* Moving Average Filter is a simple interface for quick calculations
* and saving of samples to sliding window. 
* Instead of storing any variables globally, this interface relies on a structure.
* Members
*   samples_window:    Sliding window for storing the samples
*   sum:               Current sum of the elements in the sliding window
*                      Allows to avoid heavy calcultions when getting the filter result
*   raw_sample_number: Number of samples that went through the filter.
*                      Used for quick calculation of the element that should be replaced in the window
*                      NOTE: The type of this value defines the upper limit for window length (uint32 MAX)
*
* FILTER_LENGTH defines the length of the filter. It is constantly defined,
* to ensure the memory safety when using the utility.
*
*******************************************************************************/

#ifndef MOVING_AVERAGE_FILTER_H
#define MOVING_AVERAGE_FILTER_H

    
#include "project.h"
    
#define FILTER_LENGTH 30

/* Moving average filter. 
   Length of the window is defined by FILTER_LENGTH */
typedef struct MovingAverageFilter {
    float  samples_window[FILTER_LENGTH];  // Filter window
    float  sum;                            // Current sum of the samples in the filter
    uint32 raw_sample_number;              // Number of samples that went through the filter
} MovingAverageFilter;

/* Function declarations */
void  add_sample_to_MA_filter(MovingAverageFilter* filter, const float sample);
float get_MA_filtered_result(MovingAverageFilter* filter);

 
#endif
