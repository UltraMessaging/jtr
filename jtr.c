/* jtr.c - tool infrastructrure for measuring jitter.
 * see https://github.com/UltraMessaging/jtr
 *
 * Copyright (c) 2005-2019 Informatica Corporation. All Rights Reserved.
 * Permission is granted to licensees to use or alter this software for
 * any purpose, including commercial applications, according to the terms
 * laid out in the Software License Agreement.
 *
 * This source code example is provided by Informatica for educational
 * and evaluation purposes only.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND INFORMATICA DISCLAIMS ALL WARRANTIES
 * EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY IMPLIED WARRANTIES OF
 * NON-INFRINGEMENT, MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
 * INFORMATICA DOES NOT WARRANT THAT USE OF THE SOFTWARE WILL BE UNINTERRUPTED
 * OR ERROR-FREE.  INFORMATICA SHALL NOT, UNDER ANY CIRCUMSTANCES, BE
 * LIABLE TO LICENSEE FOR LOST PROFITS, CONSEQUENTIAL, INCIDENTAL, SPECIAL OR
 * INDIRECT DAMAGES ARISING OUT OF OR RELATED TO THIS AGREEMENT OR THE
 * TRANSACTIONS CONTEMPLATED HEREUNDER, EVEN IF INFORMATICA HAS BEEN APPRISED
 * OF THE LIKELIHOOD OF SUCH DAMAGES.
 */

/* Allow setting thread affinity. */
#define _GNU_SOURCE
#include <sched.h>

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include "jtr.h"

/* Time the host takes to do various things. Start them at maximum value;
 * the jtr_calibrate() function will reduce them to the minimum measurement.
 */
long long jtr_gettime_cost = 0x7fffffffffffffff;
long long jtr_1000_loops_cost = 0x7fffffffffffffff;

/* Histogram, in HISTO_GRANULARITY ns increments. */
int jtr_histo[HISTO_BUCKETS];
int jtr_histo_overflows;
int jtr_histo_max_time;
int jtr_histo_min_time;
long long jtr_histo_tot_time;
int jtr_histo_num_samples;
int jtr_histo_average;

char jtr_results_buf[65536];


/* Pin calling thread to a CPU.
 */
void jtr_pin_cpu(int cpu_num)
{
  cpu_set_t cpu_set;

  memset(&cpu_set, 0, sizeof(cpu_set));
  CPU_SET(cpu_num, &cpu_set);
  SYSE(sched_setaffinity(getpid(), sizeof(cpu_set), &cpu_set));
}  /* jtr_pin_cpu */


void jtr_set_fifo_priority(int priority)
{
  struct sched_param sched_parameter;

  memset(&sched_parameter, 0, sizeof(sched_parameter));
  sched_parameter.sched_priority = priority;
  SYSE(sched_setscheduler(0, SCHED_FIFO, &sched_parameter));
}  /* jtr_set_fifo_priority */


/* Use busy looping to delay short periods of time. Resolution is limited
 * by the time consumed by clock_gettime(). On a good system, takes 15 ns.
 */
void jtr_spin_sleep_ns(long long sleep_ns)
{
  struct timespec ts;  /* tv_sec, tv_nsec */
  long long end_ns;
  long long cur_ns;

  if (sleep_ns > 0) {
    if (sleep_ns > 2*jtr_gettime_cost) {
      sleep_ns -= 2*jtr_gettime_cost;  /* Adjust for cost of gettime. */
    }
    SYSE(clock_gettime(CLOCK_MONOTONIC, &ts));
    cur_ns = ((long long)ts.tv_sec * NANOS_PER_SEC) + (long long)ts.tv_nsec;
    end_ns = cur_ns + sleep_ns;

    while (cur_ns < end_ns) {
      SYSE(clock_gettime(CLOCK_MONOTONIC, &ts));
      cur_ns = ((long long)ts.tv_sec * NANOS_PER_SEC) + (long long)ts.tv_nsec;
    }
  }
}  /* jtr_spin_sleep_ns */


/* See how long various things take on this system. Use those measurements
 * elsewhere.
 * This function is called 20 times, and the minimum times are kept.
 */
void jtr_calibrate()
{
  int i;
  struct timespec start_ts;  /* tv_sec, tv_nsec */
  struct timespec end_ts;  /* tv_sec, tv_nsec */
  long long start_ns;
  long long end_ns;
  long long difference_ns;

  /* How long does a clock_gettime() take? */
  SYSE(clock_gettime(CLOCK_MONOTONIC, &start_ts));
  for (i = 0; i < 1000; i++) {
    SYSE(clock_gettime(CLOCK_MONOTONIC, &end_ts));
  }
  start_ns = ((long long)start_ts.tv_sec * NANOS_PER_SEC)
             + (long long)start_ts.tv_nsec;
  end_ns = ((long long)end_ts.tv_sec * NANOS_PER_SEC)
           + (long long)end_ts.tv_nsec;
  difference_ns = (end_ns - start_ns)/1001;
  if (difference_ns < jtr_gettime_cost) {
    jtr_gettime_cost = difference_ns;
  }

  /* How many busy loops in a ns? */
  SYSE(clock_gettime(CLOCK_MONOTONIC, &start_ts));
  for (i = 0; i < 1000; i++) {
  }
  SYSE(clock_gettime(CLOCK_MONOTONIC, &end_ts));
  start_ns = ((long long)start_ts.tv_sec * NANOS_PER_SEC)
             + (long long)start_ts.tv_nsec;
  end_ns = ((long long)end_ts.tv_sec * NANOS_PER_SEC)
           + (long long)end_ts.tv_nsec;
  difference_ns = (end_ns - start_ns);
  difference_ns -= jtr_gettime_cost;  /* Correct for cost of gettime. */
  if (difference_ns < jtr_1000_loops_cost) {
    jtr_1000_loops_cost = difference_ns;
  }
}  /* jtr_calibrate */


/* Initialize the results.
 */
void jtr_histo_init()
{
  int i;

  for (i = 0; i < HISTO_BUCKETS; i++) {
    jtr_histo[i] = 0;
  }
  jtr_histo_overflows = 0;
  jtr_histo_min_time = 0x7fffffff;
  jtr_histo_max_time = 0;
  jtr_histo_tot_time = 0;
  jtr_histo_num_samples = 0;
  jtr_histo_average = 0;
}  /* jtr_histo_init */


void jtr_histo_accum(int sample_time)
{
  int bucket;

  bucket = sample_time / HISTO_GRANULARITY;
  if (bucket < HISTO_BUCKETS) {
    jtr_histo[bucket] ++;
  } else {
    jtr_histo_overflows ++;
  }

  if (sample_time < jtr_histo_min_time) {
    jtr_histo_min_time = sample_time;
  }
  if (sample_time > jtr_histo_max_time) {
    jtr_histo_max_time = sample_time;
  }

  jtr_histo_tot_time += sample_time;
  jtr_histo_num_samples ++;
  jtr_histo_average = jtr_histo_tot_time / (long long)jtr_histo_num_samples;
}  /* jtr_histo_accum */


void jtr_histo_print_summary()
{
  snprintf(&jtr_results_buf[strlen(jtr_results_buf)],
           sizeof(jtr_results_buf),
           "Minimum=%d, Maximum=%d, Average=%ld, Overflows=%d\n",
           jtr_histo_min_time,
           jtr_histo_max_time,
           jtr_histo_average,
           jtr_histo_overflows);
  SYSE(jtr_results_buf[sizeof(jtr_results_buf)-2] != '\0'); /* Mustn't be full. */
}  /* jtr_histo_print_summary */


/* Print histo percentiles.
 */
void jtr_histo_print_perc(double percentile)
{
  int i = 0;
  int tot_at_i_or_below = 0;
  int min_count;
  int highest_latency = 0;

  min_count = (int)((percentile / 100.0) * (double)jtr_histo_num_samples + 0.5);
  while (i < HISTO_BUCKETS && tot_at_i_or_below < min_count) {
    tot_at_i_or_below += jtr_histo[i];
    i++;
  }  /* while i */
  if (tot_at_i_or_below >= min_count) {
    snprintf(&jtr_results_buf[strlen(jtr_results_buf)],
             sizeof(jtr_results_buf),
             "%6.3lf%% are below %d ns\n",
             percentile,
             i * HISTO_GRANULARITY);
    SYSE(jtr_results_buf[sizeof(jtr_results_buf)-2] != '\0'); /* Mustn't be full. */
  } else {
    snprintf(&jtr_results_buf[strlen(jtr_results_buf)],
             sizeof(jtr_results_buf),
             "Warning, historgram overflow for %6.3lf%% (too many samples >= %d)\n",
             percentile,
             HISTO_BUCKETS * HISTO_GRANULARITY);
    SYSE(jtr_results_buf[sizeof(jtr_results_buf)-2] != '\0'); /* Mustn't be full. */
  }
}  /* jtr_histo_print_perc */


/* Print full jitter histo to results buf.
 */
void jtr_histo_print_details()
{
  int i;

  for (i = 0; i < HISTO_BUCKETS; i++) {
    if (jtr_histo[i] != 0) {
      snprintf(&jtr_results_buf[strlen(jtr_results_buf)],
               sizeof(jtr_results_buf),
               "bucket %d..%d: %d\n",
               i*HISTO_GRANULARITY,
               i*HISTO_GRANULARITY + HISTO_GRANULARITY - 1,
               jtr_histo[i]);
      SYSE(jtr_results_buf[sizeof(jtr_results_buf)-2] != '\0'); /* Mustn't be full. */
    }
  }
}  /* jtr_histo_print_details */


void jtr_histo_print_all(int verbose)
{
  if (verbose > 0) {
    jtr_histo_print_details();
  }
  jtr_histo_print_summary();
  jtr_histo_print_perc(90.0);
  jtr_histo_print_perc(99.0);
  jtr_histo_print_perc(99.9);
  jtr_histo_print_perc(99.99);
  jtr_histo_print_perc(99.999);
}  /* jtr_histo_print_all */


/* Use the calibration results to calculate how many times to busy loop
 * to wait a desired number of ns.
 */
int jtr_busy_loop_wait_count(long long wait_ns)
{
  return (int)((wait_ns * 1000ll) / jtr_1000_loops_cost);
}  /* jtr_busy_loop_wait_count */


/* Main measurement loop. */
void jtr_measure_calls(int warmup_loops, int measure_loops, int post_call_wait_ns,
                       app_cb_t app_cb, void *clientd)
{
  struct timespec start_call_ts;
  struct timespec end_call_ts;
  long long start_call_ns;
  long long end_call_ns;
  long long call_ns;
  int bucket;
  int i, j;
  int expected_sqn;

  /* Use negative values for "i" as warm-up loops. */
  for (i = -warmup_loops; i < measure_loops; i++) {
    SYSE(clock_gettime(CLOCK_MONOTONIC, &start_call_ts));
    app_cb(clientd);
    SYSE(clock_gettime(CLOCK_MONOTONIC, &end_call_ts));

    if (post_call_wait_ns >= 0) {
      jtr_spin_sleep_ns(post_call_wait_ns);
    } else {
      usleep(-post_call_wait_ns/1000);
    }

    start_call_ns = ((long long)start_call_ts.tv_sec * NANOS_PER_SEC)
                    + (long long)start_call_ts.tv_nsec;
    end_call_ns = ((long long)end_call_ts.tv_sec * NANOS_PER_SEC)
                  + (long long)end_call_ts.tv_nsec;
    call_ns = end_call_ns - start_call_ns;
    call_ns -= jtr_gettime_cost;  /* Correct for cost of timestamp. */
    if (call_ns < 0) {
      call_ns = 0;
    }

    if (i >= 0) {  /* Only accumulate results if past warmup period. */
      jtr_histo_accum(call_ns);
    }  /* if i >= 0 */
  }
}  /* jtr_measure_calls */
