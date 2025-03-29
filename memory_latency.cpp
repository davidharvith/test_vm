// OS 2025 EX1

#include "memory_latency.h"
#include "measure.h"
/*
#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cstdint>
#include <cmath>
*/

#define GALOIS_POLYNOMIAL ((1ULL << 63) | (1ULL << 62) | (1ULL << 60) | (1ULL << 59))


/**
 * parses input into a 64 bit int and throws an error if it not compatable
 * @param str to convert
 * @return the 64 bit int value
 */
int64_t parse_int64 (const char *str)
{
  char *endptr;
  errno = 0;
  int64_t value = strtoll(str, &endptr, 10);

  if (errno != 0 || *endptr != '\0') {
    fprintf(stderr, "Invalid 64 bit int input '%s'.\n", str);
    exit(-1);
  }
  return value;
}


int parse_int (const char *str)
{
  char *endptr;
  errno = 0;
  int value = strtol(str, &endptr, 10);

  if (errno != 0 || *endptr != '\0') {
    fprintf(stderr, "Invalid 64 bit int input '%s'.\n", str);
    exit(-1);
  }
  return value;
}

/**
 * parses the input into a float and throwh an error if it is not a float
 * @param str to convert
 * @return the float value of the string
 */
float parse_float (const char *str)
{
  char *endptr;
  errno = 0;
  float value = strtof(str, &endptr);

  if (errno != 0 || *endptr != '\0') {
    fprintf(stderr, "Invalid float input '%s'.\n", str);
    exit(-1);
  }
  return value;
}

/**
 * checks the validity of the users input
 * @param max_size the maximum size of array to check
 * @param factor the factor to multiply by
 * @param repeat how many times to reapt
 */
void validate_input (int64_t max_size, float factor, int repeat)
{
  if (max_size < 100) {
    fprintf(stderr, "max_size must be more than 100.\n");
    exit(-1);
  }
  if (factor <= 1.0) {
    fprintf(stderr, "factor must be  more than 1.\n");
    exit(-1);
  }
  if (repeat <= 0) {
    fprintf(stderr, "Error: repeat must be more than 0.\n");
    exit(-1);
  }
}



/**
 * Converts the struct timespec to time in nano-seconds.
 * @param t - the struct timespec to convert.
 * @return - the value of time in nano-seconds.
 */
uint64_t nanosectime(struct timespec t)
{
  return (uint64_t)t.tv_sec * 1000000000ULL + (uint64_t)t.tv_nsec;
}

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
struct measurement measure_sequential_latency(uint64_t repeat, array_element_t* arr,
    uint64_t arr_size, uint64_t zero)
{
  repeat = arr_size > repeat ? arr_size:repeat; // Make sure repeat >= arr_size

  // Baseline measurement:
  struct timespec t0;
  timespec_get(&t0, TIME_UTC);
  register uint64_t rnd=12345;
  for (register uint64_t i = 0; i < repeat; i++)
  {
    register uint64_t index = rnd % arr_size;
    rnd ^= index & zero;
    rnd = (rnd >> 1) ^ ((0-(rnd & 1)) & GALOIS_POLYNOMIAL);  // Advance rnd pseudo-randomly (using Galois LFSR)
  }
  struct timespec t1;
  timespec_get(&t1, TIME_UTC);

  // Memory access measurement:
  struct timespec t2;
  timespec_get(&t2, TIME_UTC);
  rnd=(rnd & zero) ^ 12345;
  for (register uint64_t i = 0; i < repeat; i++)
  {
    register uint64_t index = i % arr_size;
    rnd ^= arr[index] & zero;
    rnd = (rnd >> 1) ^ ((0-(rnd & 1)) & GALOIS_POLYNOMIAL);  // Advance rnd pseudo-randomly (using Galois LFSR)
  }
  struct timespec t3;
  timespec_get(&t3, TIME_UTC);

  // Calculate baseline and memory access times:
  double baseline_per_cycle=(double)(nanosectime(t1)- nanosectime(t0))/(repeat);
  double memory_per_cycle=(double)(nanosectime(t3)- nanosectime(t2))/(repeat);
  struct measurement result;

  result.baseline = baseline_per_cycle;
  result.access_time = memory_per_cycle;
  result.rnd = rnd;
  return result;
}

/**
 * Runs the logic of the memory_latency program. Measures the access latency for random and sequential memory access
 * patterns.
 * Usage: './memory_latency max_size factor repeat' where:
 *      - max_size - the maximum size in bytes of the array to measure access latency for.
 *      - factor - the factor in the geometric series representing the array sizes to check.
 *      - repeat - the number of times each measurement should be repeated for and averaged on.
 * The program will print output to stdout in the following format:
 *      mem_size_1,offset_1,offset_sequential_1
 *      mem_size_2,offset_2,offset_sequential_2
 *              ...
 *              ...
 *              ...
 */
int main(int argc, char* argv[])
{
    // zero==0, but the compiler doesn't know it. Use as the zero arg of measure_latency and measure_sequential_latency.
    struct timespec t_dummy;
    timespec_get(&t_dummy, TIME_UTC);
    const uint64_t zero = nanosectime(t_dummy)>1000000000ull?0:nanosectime(t_dummy);

    // Your code here
  if (argc != 4) {
    fprintf(stderr, "must accept exactly 3 command line args");
    exit(-1);
  }

  int64_t max_size = parse_int64 (argv[1]);
  float factor = parse_float (argv[2]);
  int repeat = parse_int (argv[3]);
  validate_input (max_size, factor, repeat);
  int i = 1;
  int size = 100;
  while(size < max_size){
    auto* arr =(array_element_t*) malloc (size*sizeof(array_element_t));
    if (arr == NULL) {
      fprintf(stderr, "Memory allocation failed for size %d\n", size);
      exit(-1);
    }
    measurement rand_mes = measure_latency(repeat, arr, size, zero);
    measurement seq_mes = measure_sequential_latency(repeat,
                                                     arr, size, zero);
    printf("mem_size1(%d),offset1(random access %f, ns),offset1"
         "(sequential %f, ns)\n", size, rand_mes.access_time-rand_mes
         .baseline, seq_mes.access_time-seq_mes.baseline);
    free(arr);
    size = (int)(pow (factor,i)*100);


    i++;
  }

}

