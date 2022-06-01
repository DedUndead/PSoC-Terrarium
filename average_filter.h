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

#ifndef AVERAGE_FILTER_H
#define AVERAGE_FILTER_H


#include "project.h"

typedef struct AverageFilter {
    uint32 filter_length;  // Filter length
    uint32 sample_number;  // Number of recorded samples
    int64 running_sum;     // Running sum of the samples
} AverageFilter;

/* Function declarations */
void add_sample_to_filter(AverageFilter* filter, const int new_sample);
int get_filtered_result(AverageFilter* filter);

#endif