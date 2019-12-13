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
long long jtr_rdtsc_cost = 0x7fffffffffffffff;
long long jtr_1000_loops_cost = 0x7fffffffffffffff;
long long jtr_ticks_per_sec = 0x7fffffffffffffff;

/* Histogram, in HISTO_GRANULARITY ns increments. */
int jtr_histo_num_buckets;
int *jtr_histo_buckets = NULL;
int jtr_histo_overflows;
int jtr_histo_max_time;
int jtr_histo_min_time;
long long jtr_histo_tot_time;
int jtr_histo_num_samples;
int jtr_histo_average;

char jtr_results_buf[65536];
char jtr_gnuplot_buf[65536];


/* Pin calling thread to a CPU.
 */
void jtr_pin_cpu(int cpu_num)
{
  cpu_set_t cpu_set;

  memset(&cpu_set, 0, sizeof(cpu_set));
  CPU_SET(cpu_num, &cpu_set);
  SYSE(sched_setaffinity(getpid(), sizeof(cpu_set), &cpu_set));
}  /* jtr_pin_cpu */


/* Set real-time priority. Requires root.
 */
void jtr_set_fifo_priority(int priority)
{
  struct sched_param sched_parameter;

  memset(&sched_parameter, 0, sizeof(sched_parameter));
  sched_parameter.sched_priority = priority;
  SYSE(sched_setscheduler(0, SCHED_FIFO, &sched_parameter));
}  /* jtr_set_fifo_priority */


/* Use busy looping to delay short periods of time.
 * Note that it can use either RDTSC or clock_gettime().
 */
void jtr_spin_sleep_ns(long long sleep_ns, int timebase)
{
  uint32_t start_ticks_hi, start_ticks_lo;
  uint32_t cur_ticks_hi, cur_ticks_lo;
  long long start_ticks;
  long long cur_ticks;
  struct timespec ts;  /* tv_sec, tv_nsec */
  long long end_ns;
  long long cur_ns;
    
  if (sleep_ns <= 0) {
    return;
  }

  if (likely(timebase == 1)) {
    RDTSC(start_ticks_hi, start_ticks_lo);
    start_ticks = ((long long)start_ticks_hi << 32) + (long long)start_ticks_lo;
    
    do {
      RDTSC(cur_ticks_hi, cur_ticks_lo);
      cur_ticks = ((long long)cur_ticks_hi << 32) + (long long)cur_ticks_lo;
    } while (cur_ticks < start_ticks);
  }  /* timebase == 1 */
  else {
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
  uint32_t start_ticks_hi, start_ticks_lo;
  uint32_t end_ticks_hi, end_ticks_lo;
  long long start_ticks;
  long long end_ticks;
  long long diff_ticks;
  long long ticks_per_sec;
  struct timespec start_ts;  /* tv_sec, tv_nsec */
  struct timespec end_ts;  /* tv_sec, tv_nsec */
  long long start_ns;
  long long end_ns;
  long long diff_ns;

  /* How long does a clock_gettime() take? */
  clock_gettime(CLOCK_MONOTONIC, &start_ts);
  for (i = 0; i < 100; i++) {
    clock_gettime(CLOCK_MONOTONIC, &end_ts);
    clock_gettime(CLOCK_MONOTONIC, &end_ts);
    clock_gettime(CLOCK_MONOTONIC, &end_ts);
    clock_gettime(CLOCK_MONOTONIC, &end_ts);
    clock_gettime(CLOCK_MONOTONIC, &end_ts);
    clock_gettime(CLOCK_MONOTONIC, &end_ts);
    clock_gettime(CLOCK_MONOTONIC, &end_ts);
    clock_gettime(CLOCK_MONOTONIC, &end_ts);
    clock_gettime(CLOCK_MONOTONIC, &end_ts);
    clock_gettime(CLOCK_MONOTONIC, &end_ts);
  }

  start_ns = ((long long)start_ts.tv_sec * NANOS_PER_SEC)
             + (long long)start_ts.tv_nsec;
  end_ns = ((long long)end_ts.tv_sec * NANOS_PER_SEC)
           + (long long)end_ts.tv_nsec;
  diff_ns = end_ns - start_ns;
  diff_ns /= 1001;  /* Called it 1001 times. */
  if (diff_ns < jtr_gettime_cost) {
    jtr_gettime_cost = diff_ns;
  }

  /* Determine tics/sec; measure over 1/10 second. */
  RDTSC(start_ticks_hi, start_ticks_lo);
  clock_gettime(CLOCK_MONOTONIC, &start_ts);
  jtr_spin_sleep_ns(NANOS_PER_SEC / 10, 2);  /* use clock_gettime() timebase. */
  RDTSC(end_ticks_hi, end_ticks_lo);
  clock_gettime(CLOCK_MONOTONIC, &end_ts);

  start_ticks = ((long long)start_ticks_hi << 32) + (long long)start_ticks_lo;
  end_ticks = ((long long)end_ticks_hi << 32) + (long long)end_ticks_lo;
  diff_ticks = end_ticks - start_ticks;

  start_ns = ((long long)start_ts.tv_sec * NANOS_PER_SEC)
             + (long long)start_ts.tv_nsec;
  end_ns = ((long long)end_ts.tv_sec * NANOS_PER_SEC)
           + (long long)end_ts.tv_nsec;
  diff_ns = end_ns - start_ns;

  ticks_per_sec = (diff_ticks * NANOS_PER_SEC) / diff_ns;
  if (ticks_per_sec < jtr_ticks_per_sec) {
    jtr_ticks_per_sec = ticks_per_sec;
  }

  /* Cost of TDTSC. */
  RDTSC(start_ticks_hi, start_ticks_lo);
  for (i = 0; i < 100; i++) {
    RDTSC(end_ticks_hi, end_ticks_lo);
    RDTSC(end_ticks_hi, end_ticks_lo);
    RDTSC(end_ticks_hi, end_ticks_lo);
    RDTSC(end_ticks_hi, end_ticks_lo);
    RDTSC(end_ticks_hi, end_ticks_lo);
    RDTSC(end_ticks_hi, end_ticks_lo);
    RDTSC(end_ticks_hi, end_ticks_lo);
    RDTSC(end_ticks_hi, end_ticks_lo);
    RDTSC(end_ticks_hi, end_ticks_lo);
    RDTSC(end_ticks_hi, end_ticks_lo);
  }

  start_ticks = ((long long)start_ticks_hi << 32) + (long long)start_ticks_lo;
  end_ticks = ((long long)end_ticks_hi << 32) + (long long)end_ticks_lo;
  diff_ticks = end_ticks - start_ticks;
  diff_ns = (diff_ticks * NANOS_PER_SEC) / jtr_ticks_per_sec;
  diff_ns /= 1001;  /* Called it 1001 times. */
  if (diff_ns < jtr_rdtsc_cost) {
    jtr_rdtsc_cost = diff_ns;
  }

  /* Cost of a 1000-cycle null for loop. */
  RDTSC(start_ticks_hi, start_ticks_lo);
  for (i = 0; i < 1000; i++) {
  }
  RDTSC(end_ticks_hi, end_ticks_lo);

  start_ticks = ((long long)start_ticks_hi << 32) + (long long)start_ticks_lo;
  end_ticks = ((long long)end_ticks_hi << 32) + (long long)end_ticks_lo;
  diff_ticks = end_ticks - start_ticks;
  diff_ns = (diff_ticks * NANOS_PER_SEC) / jtr_ticks_per_sec;
  diff_ns -= jtr_rdtsc_cost;  /* Correct for duration of rdtsc itself. */
  if (diff_ns < jtr_1000_loops_cost) {
    jtr_1000_loops_cost = diff_ns;
  }
}  /* jtr_calibrate */


/* Initialize the results.
 */
void jtr_histo_init(int num_buckets)
{
  int i;

  if (jtr_histo_buckets == NULL) {
    jtr_histo_buckets = (int *)malloc(num_buckets * sizeof(int));
    jtr_histo_num_buckets = num_buckets;
  }
  else {
    SYSE(jtr_histo_num_buckets != num_buckets);  /* no resizing allowed. */
  }

  for (i = 0; i < jtr_histo_num_buckets; i++) {
    jtr_histo_buckets[i] = 0;
  }

  jtr_histo_overflows = 0;
  jtr_histo_min_time = 0x7fffffff;
  jtr_histo_max_time = 0;
  jtr_histo_tot_time = 0;
  jtr_histo_num_samples = 0;
  jtr_histo_average = 0;
}  /* jtr_histo_init */


/* Add a sample to the histogram.
 */
void jtr_histo_accum(int sample_time)
{
  int bucket;

  bucket = sample_time / HISTO_GRANULARITY;
  if (likely(bucket < jtr_histo_num_buckets)) {
    jtr_histo_buckets[bucket] ++;
  } else {
    jtr_histo_overflows ++;
  }

  if (unlikely(sample_time < jtr_histo_min_time)) {
    jtr_histo_min_time = sample_time;
  }
  if (unlikely(sample_time > jtr_histo_max_time)) {
    jtr_histo_max_time = sample_time;
  }

  jtr_histo_tot_time += sample_time;
  jtr_histo_num_samples ++;
  jtr_histo_average = jtr_histo_tot_time / (long long)jtr_histo_num_samples;
}  /* jtr_histo_accum */


/* Print min/max/avg/overflows. (Overflow means samples beyond the histogram.)
 */
void jtr_histo_print_summary(void)
{
  snprintf(&jtr_results_buf[strlen(jtr_results_buf)],
           sizeof(jtr_results_buf) - strlen(jtr_results_buf),
           "Minimum=%d, Maximum=%d, Average=%d, Overflows=%d\n",
           jtr_histo_min_time,
           jtr_histo_max_time,
           jtr_histo_average,
           jtr_histo_overflows);
  SYSE(jtr_results_buf[sizeof(jtr_results_buf)-2] != '\0'); /* Don't fill. */
}  /* jtr_histo_print_summary */


/* Print histo percentiles.
 */
void jtr_histo_print_perc(double percentile)
{
  int i = 0;
  int tot_at_i_or_below = 0;
  int min_count;

  min_count = (int)((percentile / 100.0) * (double)jtr_histo_num_samples + 0.5);
  while (i < jtr_histo_num_buckets && tot_at_i_or_below < min_count) {
    tot_at_i_or_below += jtr_histo_buckets[i];
    i++;
  }  /* while i */
  if (tot_at_i_or_below >= min_count) {
    snprintf(&jtr_results_buf[strlen(jtr_results_buf)],
             sizeof(jtr_results_buf) - strlen(jtr_results_buf),
             "%6.3lf%% are below %d ns\n",
             percentile,
             i * HISTO_GRANULARITY);
    SYSE(jtr_results_buf[sizeof(jtr_results_buf)-2] != '\0'); /* Don't fill. */
  } else {
    snprintf(&jtr_results_buf[strlen(jtr_results_buf)],
             sizeof(jtr_results_buf) - strlen(jtr_results_buf),
             "Warning, historgram overflow for %6.3lf%% (too many samples >= %d)\n",
             percentile,
             jtr_histo_num_buckets * HISTO_GRANULARITY);
    SYSE(jtr_results_buf[sizeof(jtr_results_buf)-2] != '\0'); /* Don't fill. */
  }
}  /* jtr_histo_print_perc */


/* Print full jitter histo to results buf.
 */
void jtr_histo_print_details(void)
{
  int i;

  for (i = 0; i < jtr_histo_num_buckets; i++) {
    if (jtr_histo_buckets[i] != 0) {
      snprintf(&jtr_results_buf[strlen(jtr_results_buf)],
               sizeof(jtr_results_buf) - strlen(jtr_results_buf),
               "bucket %d..%d: %d\n",
               i*HISTO_GRANULARITY,
               i*HISTO_GRANULARITY + HISTO_GRANULARITY - 1,
               jtr_histo_buckets[i]);
      SYSE(jtr_results_buf[sizeof(jtr_results_buf)-2] != '\0'); /* Don't fill. */
    }
  }
}  /* jtr_histo_print_details */


int jtr_png_filenum = 0;
int jtr_x_low = 0x7fffffff;
int jtr_x_high = 0;
int jtr_y_high = 1000000;

/* Print GnuPlot commands and data to plot the results.
 */
void jtr_histo_gnuplot(char *title)
{
  int i;
  char gnuplot_title[1024];

  /* gnuprint doesn't like quoted strings with actual newlines or underscores.
   * Make copy, converting disliked characters.
   */
  i = 0;
  while (*title != '\0') {
    if (*title == '\n') {
      gnuplot_title[i] = '\\'; i++;
      gnuplot_title[i] = 'n';  i++;
    }
    else if (*title == '_') {
      gnuplot_title[i] = '\\'; i++;
      gnuplot_title[i] = *title;  i++;
    }
    else {
      gnuplot_title[i] = *title; i++;
    }
    title++;
    SYSE(i >= (sizeof(gnuplot_title)-1)); /* Don't fill. */
  }
  gnuplot_title[i] = '\0';

  /* Add summary to plot title. */
  snprintf(&gnuplot_title[strlen(gnuplot_title)],
           sizeof(gnuplot_title) - strlen(gnuplot_title),
           "\\nMinimum=%d, Maximum=%d, Average=%d, Overflows=%d",
           jtr_histo_min_time,
           jtr_histo_max_time,
           jtr_histo_average,
           jtr_histo_overflows);

  /* Set gnuplot variables according to the test run parameters. */
  jtr_png_filenum ++;
  snprintf(&jtr_gnuplot_buf[strlen(jtr_gnuplot_buf)],
           sizeof(jtr_gnuplot_buf) - strlen(jtr_results_buf),
           "# title_%d = \"%s\"\n# xrange_%d = %d\n",
           jtr_png_filenum, gnuplot_title,
           jtr_png_filenum, jtr_histo_num_buckets*10);
  SYSE(jtr_gnuplot_buf[sizeof(jtr_gnuplot_buf)-2] != '\0'); /* Don't fill. */

  for (i = 0; i < jtr_histo_num_buckets; i++) {
    if (jtr_histo_buckets[i] != 0) {
      snprintf(&jtr_gnuplot_buf[strlen(jtr_gnuplot_buf)],
               sizeof(jtr_gnuplot_buf) - strlen(jtr_results_buf),
               "%d %d\n",
               i*HISTO_GRANULARITY,
               jtr_histo_buckets[i]);
      SYSE(jtr_gnuplot_buf[sizeof(jtr_gnuplot_buf)-2] != '\0'); /* Don't fill. */
      if ((i*HISTO_GRANULARITY) > jtr_x_high) {
        jtr_x_high = i*HISTO_GRANULARITY;
        jtr_x_high -= jtr_x_high % 100;
        jtr_x_high += 100;
      }
    }
  }

  if (jtr_histo_min_time < jtr_x_low) {
    jtr_x_low = jtr_histo_min_time;
  }
  jtr_x_low -= jtr_x_low % 10;

  snprintf(&jtr_gnuplot_buf[strlen(jtr_gnuplot_buf)],
           sizeof(jtr_gnuplot_buf) - strlen(jtr_results_buf),
           "\n\n");
  SYSE(jtr_gnuplot_buf[sizeof(jtr_gnuplot_buf)-2] != '\0'); /* Don't fill. */
}  /* jtr_histo_gnuplot */


/* Print all results desired by user to memory buffers for later display.
 */
void jtr_histo_print_all(int verbose, char *title)
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

  jtr_histo_gnuplot(title);
}  /* jtr_histo_print_all */


/* Use the calibration results to calculate how many times to busy loop
 * to wait a desired number of ns.
 */
int jtr_busy_loop_wait_count(long long wait_ns)
{
  return (int)((wait_ns * 1000ll) / jtr_1000_loops_cost);
}  /* jtr_busy_loop_wait_count */


/* Main measurement loop. */
void jtr_measure_calls(int warmup_loops, int measure_loops,
                       int post_call_wait_ns, int timebase,
                       app_cb_t app_cb, void *clientd)
{
  uint32_t start_ticks_hi, start_ticks_lo;
  uint32_t end_ticks_hi, end_ticks_lo;
  long long start_ticks;
  long long end_ticks;
  long long diff_ticks;
  struct timespec start_ts;  /* tv_sec, tv_nsec */
  struct timespec end_ts;  /* tv_sec, tv_nsec */
  long long start_ns;
  long long end_ns;
  long long diff_ns;
  int i;

  /* Use negative values for "i" as warm-up loops. */
  for (i = -warmup_loops; i < measure_loops; i++) {
    if (likely(timebase == 1)) {
      RDTSC(start_ticks_hi, start_ticks_lo);
      app_cb(clientd);
      RDTSC(end_ticks_hi, end_ticks_lo);

      start_ticks = ((long long)start_ticks_hi << 32) + (long long)start_ticks_lo;
      end_ticks = ((long long)end_ticks_hi << 32) + (long long)end_ticks_lo;
      diff_ticks = end_ticks - start_ticks;
      diff_ns = (diff_ticks * NANOS_PER_SEC) / jtr_ticks_per_sec;
      diff_ns -= jtr_rdtsc_cost;  /* Correct for measurement cost. */
    }  /* timebase == 1 */
    else {
      clock_gettime(CLOCK_MONOTONIC, &start_ts);
      app_cb(clientd);
      clock_gettime(CLOCK_MONOTONIC, &end_ts);
      start_ns = ((long long)start_ts.tv_sec * NANOS_PER_SEC)
                 + (long long)start_ts.tv_nsec;
      end_ns = ((long long)end_ts.tv_sec * NANOS_PER_SEC)
               + (long long)end_ts.tv_nsec;
      diff_ns = end_ns - start_ns;
      diff_ns -= jtr_gettime_cost;  /* Correct measurement cost. */
    }

    if (unlikely(diff_ns < 0)) {
      diff_ns = 0;
    }

    if (likely(post_call_wait_ns >= 0)) {
      jtr_spin_sleep_ns(post_call_wait_ns, timebase);
    } else {
      usleep(-post_call_wait_ns/1000);
    }

    if (likely(i >= 0)) {  /* Only accumulate results if past warmup period. */
      jtr_histo_accum(diff_ns);
    }  /* if i >= 0 */
  }
}  /* jtr_measure_calls */
