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

#include "moving_average_filter.h"

/*
 * @brief Add new sample to moving average filter
 * @param filter Pointer to target filter
 * @param sample New sample
 */
void add_sample_to_MA_filter(MovingAverageFilter* filter, const int16 sample)
{
    // Determine the index for the current replacement in window
    uint8 index = filter->raw_sample_number % FILTER_LENGTH;
    filter->raw_sample_number++;
    
    // Update the sum and replace an old sample in sliding window
    filter->sum -= filter->samples_window[index];
    filter->sum += sample;
    filter->samples_window[index] = sample;
}

/*
 * @brief Get the filtered result based on collected samples
 * Contains integer rounding errors.
 * @param filter Pointer to target filter
 */
int16 get_MA_filtered_result(MovingAverageFilter* filter)
{
    // Note: There is no need to make this a critical section
    // because all the interrupts deffer calculations to main
    // Thus, no value can be altered
    
    // Sliding window is filled
    if (filter->raw_sample_number >= FILTER_LENGTH) {
        return filter->sum / FILTER_LENGTH; 
    }
    
    // Sliding window is not yet filled or samples number overflowed
    else {
        /* Calculate temporary sum based on already collected elements */
        int temporary_sum = 0;
        for (uint8 i = 0; i < filter->raw_sample_number; i++) {
            temporary_sum += filter->samples_window[i];
        }
        return temporary_sum / filter->raw_sample_number;
    }
}