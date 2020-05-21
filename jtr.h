/* jtr.h - External definitions for jitter measurements.
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

#ifndef JTR_H
#define JTR_H
#include <errno.h>
#include <inttypes.h>

#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)

/* 10G runs at .8 ns per byte.
 * Enet frame has 38 bytes of overhead (includes interpacket gap).
 * An ipv4 UDP packet has 20 bytes IP header and 8 bytes of udp header.
 * LBM has 20 bytes of datagram header plus 12 bytes of LBMC header.
 * Total overhead=38+20+8+20+12=98 bytes.
 * Packet time = 78.4 ns + .8 ns per application byte.
 * 1024 application bytes: 78.4+819.2=897.6 ns. (1.11 million pkt/sec)
 */
#define JTR_10G_XMIT_1024_PKT_NS 898  /* Time for NIC to send 1K LBM msg. */

/* Macro to sample the RDTSC register. */
#define RDTSC(hi, lo) do { \
  asm volatile ("rdtsc" : "=a" (lo), "=d" (hi)); \
} while (0)

/* Convenience constant so I don't accidentally miscount zeros. */
#define NANOS_PER_SEC 1000000000ll  /* Long long constant for 10**9. */

typedef void (*app_cb_t)(void *clientd);

/* Time the host takes to do various things. Start them at maximum value;
 * the jtr_calibrate() function will reduce them to the minimum measurement.
 */
extern long long jtr_gettime_cost;
extern long long jtr_rdtsc_cost;
extern long long jtr_1000_loops_cost;
extern long long jtr_ticks_per_sec;  /* RDTSC ticks per second. */

/* Histogram, in HISTO_GRANULARITY ns increments. */
#define HISTO_GRANULARITY 10  /* Histogram ns per bucket. */
extern int jtr_histo_num_buckets;
extern int *jtr_histo_buckets;
extern int jtr_histo_overflows;
extern int jtr_histo_max_time;
extern int jtr_histo_min_time;
extern long long jtr_histo_tot_time;
extern int jtr_histo_num_samples;
extern int jtr_histo_average;
extern int jtr_x_low;
extern int jtr_x_high;
extern int jtr_y_high;

extern char jtr_results_buf[65536];
extern char jtr_gnuplot_buf[65536];

/* Very simplistic error handling macro for LBM functions.  Pass in return
 * value. Tests for error, prints error, and aborts (core dump). */
#define LBME(_call) do {\
  int _e = _call;\
  if ((_e) != LBM_OK) {\
    int _errnum = lbm_errnum();\
    fprintf(stderr, "LBME: failed at %s:%d %s\n  errnum=%d (%s)\n",\
            __FILE__, __LINE__, #_call, _errnum, lbm_errmsg());\
    fflush(stderr);\
    abort();\
  }\
} while (0)

/* Very simplistic error handling macro for system functions that return 0
 * on success. Pass in return value. Tests for error, prints, and aborts. */
#define SYSE(_call) do {\
  int _e = _call;\
  if ((_e) != 0) {\
    int _errnum = errno;\
    fprintf(stderr, "SYSE: failed at %s:%d %s\n  errno=%u (%s)\n",\
            __FILE__, __LINE__, #_call, _errnum, strerror(_errnum));\
    fflush(stderr);\
    abort();\
  }\
} while (0)

/* Very simplistic error handling macro for internal error checking.
 * Pass in a condition that must be true. If false, prints, and aborts. */
#define ASSRT(_cond) do {\
  int _e = _cond;\
  if (! _e) {\
    fprintf(stderr, "ASSRT: failed at %s:%d %s\n",\
            __FILE__, __LINE__, #_cond);\
    fflush(stderr);\
    abort();\
  }\
} while (0)

void jtr_pin_cpu(int cpu_num);
void jtr_set_fifo_priority(int priority);
void jtr_spin_sleep_ns(long long sleep_ns, int timebase);
void jtr_calibrate(void);
void jtr_histo_init(int num_buckets);
void jtr_histo_accum(int sample_time);
void jtr_histo_print_summary(void);
void jtr_histo_print_perc(double percentile);
void jtr_histo_print_details(void);
void jtr_histo_print_all(int verbose, char *title);
int jtr_busy_loop_wait_count(long long wait_ns);
void jtr_measure_one(int timebase, int accum,
                     app_cb_t app_cb, void *clientd);
void jtr_measure_calls(int warmup_loops, int measure_loops,
                       int post_call_wait_ns, int timebase,
                       app_cb_t app_cb, void *clientd);

#endif  /* JTR_H */
