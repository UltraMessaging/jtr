/* jtr_null.c - tool to measure system jitter.
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
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>

#include "jtr.h"

/* Options and their defaults. See jtr_getopts(). */
int opt_cpu_num = 3;
int opt_fifo_priority = -1;
int opt_loops = 3;
int opt_msg_size = 1024;
int opt_num_msgs = 1000000;
int opt_pkt_delay = JTR_10G_XMIT_1024_PKT_NS;
int opt_warmup_loops = 500;
int opt_verbose = 0;

/* Options specific to the null test. */
int opt_Busy_spins = 100;


void usage()
{
  fprintf(stderr, "Usage: jtr_null -B busy_spins [-c cpu_num] [-f fifo_priority] [-l loops] [-m msg_size] [-n num_msgs] [-p pkt_delay] [-w warmup_loops] [-v verbosity]\n");
  exit(1);
}  /* usage */


/* Parse command-line options.
 */
void jtr_getopts(int argc, char **argv)
{
  int opt;

  while ((opt = getopt(argc, argv, "B:c:f:l:m:n:p:w:v:")) != EOF) {
    switch (opt) {
      case 'B': opt_Busy_spins  = atoi(optarg); break;
      case 'c': opt_cpu_num = atoi(optarg); break;
      case 'f': opt_fifo_priority = atoi(optarg); break;
      case 'l': opt_loops = atoi(optarg); break;
      case 'm': opt_msg_size = atoi(optarg); break;
      case 'n': opt_num_msgs = atoi(optarg); break;
      case 'p': opt_pkt_delay = atoi(optarg); break;
      case 'w': opt_warmup_loops = atoi(optarg); break;
      case 'v': opt_verbose = atoi(optarg); break;
      default: usage();
    }  /* switch opt */
  }  /* while getopt */
}  /* jtr_getopts */


void null_send_cb(void *clientd)
{
  int i;

  for (i = 0; i < opt_Busy_spins; i++) { } 
}  /* null_send_cb */


int main(int argc, char **argv)
{
  int i;

  /* Parse command-line options. */
  jtr_getopts(argc, argv);

  if (opt_cpu_num >= 0) {
    jtr_pin_cpu(opt_cpu_num);
  }

  if (opt_fifo_priority >= 0) {
    jtr_set_fifo_priority(opt_fifo_priority);
  }

  for (i = 0; i < 20; i++) {
    jtr_calibrate();
  }

  snprintf(&jtr_results_buf[strlen(jtr_results_buf)],
           sizeof(jtr_results_buf),
           "Busy_spins=%d, cpu_num=%d, msg_size=%d, num_msgs=%d, pkt_delay=%d warmup_loops=%d, gettime_cost=%lld, jtr_1000_loops_cost=%lld\n",
           opt_Busy_spins, opt_cpu_num, opt_msg_size, opt_num_msgs, opt_pkt_delay, opt_warmup_loops, jtr_gettime_cost, jtr_1000_loops_cost);
  SYSE(jtr_results_buf[sizeof(jtr_results_buf)-2] != '\0'); /* Mustn't be full. */

  /* Conduct the timing tests! */

  for (i = 0; i < opt_loops; i++) {
    jtr_histo_init();
    jtr_measure_calls(opt_warmup_loops, opt_num_msgs, opt_pkt_delay, null_send_cb, NULL);
    jtr_histo_print_all(opt_verbose);
  }

  if (opt_verbose >= 0) {
    printf("%s", jtr_results_buf);
  }

  return 0;
}  /* main */
