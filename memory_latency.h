// OS 2025 EX1

#ifndef _MEMORY_LATENCY_H
#define _MEMORY_LATENCY_H

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <stdint.h>

typedef uint64_t array_element_t;


/**
 * Used as the return type for 'measure_latency'.
 */
struct measurement {
    double baseline;
    double access_time;
    uint64_t rnd;
};


/**
 * Converts the struct timespec to time in nano-seconds.
 * @param t - the struct timespec to convert.
 * @return - the value of time in nano-seconds.
 */
uint64_t nanosectime(struct timespec t);


/**
* Measures the average latency of accessing a given array in a sequential order.
* @param repeat - the number of times to repeat the measurement for and average on.
* @param arr - an allocated (not empty) array to preform measurement on.
* @param arr_size - the length of the array arr.
* @param zero - a variable containing zero in a way that the compiler doesn't "know" it in compilation time.
* @return struct measurement containing the measurement with the following fields:
*      double baseline - the average time (ns) taken to preform the measured operation without memory access.
*      double access_time - the average time (ns) taken to preform the measured operation with memory access.
*      uint64_t rnd - the variable used to randomly access the array, returned to prevent compiler optimizations.
*/
struct measurement measure_sequential_latency(uint64_t repeat, array_element_t* arr, uint64_t arr_size, uint64_t zero);

/**
 * checks the validity of the users input
 * @param max_size the maximum size of array to check
 * @param factor the factor to multiply by
 * @param repeat how many times to reapt
 */
void validate_input(int64_t max_size, float factor, int repeat);


/**
 * parses the input into a float and throwh an error if it is not a float
 * @param str to convert
 * @return the float value of the string
 */
float parse_float(const char *str);


/**
 * parses input into a 64 bit int and throws an error if it not compatable
 * @param str to convert
 * @return the 64 bit int value
 */
int64_t parse_int64(const char *str);

/**
 * parses input into a int and throws error if not compatable
 * @param str to convert
 * @return int
 */
int parse_int (const char *str);

#endif
