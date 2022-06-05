/******************************************************************************
* 
* @author  Pavel Arefyev
* @name    TX00DB04 - Programmable System on Chip Design. Average Filter
* @company Metropolia University of Applied Sciences
* @date    07.04.2022
*
* This file provides basic interface for the simplest filter: average filter.
* The samples are saved to the running sum and the filtered result is an average of saved samples.
* The filter clears running sum once .filter_length is reached.
*
*******************************************************************************/

#include "average_filter.h"

/*
 * @brief Update the filter with new sample
 * @param filter     Target filter
 * @param new_sample Target sample
 */
void add_sample_to_filter(AverageFilter* filter, const int new_sample)
{
    // Reset the filter if the window is fully filled
    if (filter->sample_number == filter->filter_length) {
        filter->running_sum = 0;
        filter->sample_number = 0;
    }

    // Update the filter
    filter->running_sum += new_sample;
    filter->sample_number++;
}

/*
 * @brief  Get filtered result (average) of the current samples collected
 * @param  filter Target filter
 * @return        Filtered result (average)
 */
int get_filtered_result(AverageFilter* filter)
{
    // Avoid zero-division if filter is empty
    if (filter->sample_number == 0) return 0;
    
    return filter->running_sum / filter->sample_number;
}