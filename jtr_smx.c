/* jtr_smx.c - tool to measure Ultra Messaging jitter.
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

#include <lbm/lbm.h>
#include "jtr.h"

#define JTR_TOPIC_STR "jtr_topic"

#define JTR_CFG_FILE "lbm.cfg"

/* Options and their defaults. See get_options(). */
int opt_cpu_num = 5;
char *opt_descr = "Jitter lbm test";
int opt_fifo_priority = -1;
char *opt_gnuplot_file = NULL;
int opt_histo_buckets = 800;
int opt_loops = 3;
int opt_num_samples = 2000000;
int opt_pause = JTR_10G_XMIT_1024_PKT_NS;
int opt_timebase = 1;  /* 1=RDTSC, 2=clock_gettime() */
int opt_warmup_loops = 1500;
int opt_verbose = 0;

/* Options specific to the UM test. */
int opt_Msg_size = 1024;

/* UM objects. */
lbm_context_t *jtr_ctx;     /* Handle for context object. */
lbm_src_t *jtr_src;         /* Handle for source (sender) object. */
char *message_buf;

/* Other globals. */
pthread_t jtr_ctx_thread_id;
int jtr_ctx_running = 0;
int jtr_no_send_spin = 0;


void usage()
{
  fprintf(stderr,
"Usage: jtr_sock [-M msg_size]"
" [-c cpu_num] [-d descr] [-f fifo_priority] [-g gnuplot_file]"
" [-h histo_buckets] [-l loops] [-n num_samples] [-p pause] [-t timebase]"
" [-w warmup_loops] [-v verbose]\n");
  fprintf(stderr,
"Where:\n");
  fprintf(stderr,
" -M msg_size : number of bytes in message to send. (default=1024)\n"
" -c cpu_num : integer CPU number to pin thread. Use -1 to not pin.\n"
"              (default=5)\n"
" -d descr : description string. (default='Jitter lbm test')\n"
" -f fifo_priority : integer priority level to set real-time FIFO scheduling.\n"
"                  : -1 does not set real-time. (default=-1)\n"
" -g gnuplot_file : name of file to create containing gnuplot commands to\n"
"                   graph the histogram data. (default=no file)\n"
" -h histo_buckets : number of buckets for histogram. (default=800)\n"
" -l loops : number of test runs to perform. Each test run does two tests,\n"
"            first with the UM call, and then with an equivelent null loop.\n"
"            (default=3)\n"
" -n num_samples : number of samples in a test. (default=1,000,000)\n"
" -p pause : number of nanoseconds to pause between each sample, to allow\n"
"            packet to exit NIC. (default=898)\n"
" -t timebase : integer indicating which method of time measement to use.\n"
"               1=RDTSC, 2=clock_gettime(). (default=1)\n"
" -w warmup_loops : number of samples to initially take without accumulating\n"
"                   the results. (default=500)\n"
" -v verbose : integer indicating how much information to print.\n"
"              -1=quiet, 0=normal, 2=details. (default=0)\n");
  exit(1);
}  /* usage */


/* Parse command-line options.
 */
void get_options(int argc, char **argv)
{
  int opt;

  while ((opt = getopt(argc, argv, "M:c:d:f:g:h:l:n:p:t:w:v:")) != EOF) {
    switch (opt) {
      case 'M': opt_Msg_size = atoi(optarg); break;
      case 'c': opt_cpu_num = atoi(optarg); break;
      case 'd': opt_descr = strdup(optarg); break;
      case 'f': opt_fifo_priority = atoi(optarg); break;
      case 'g': opt_gnuplot_file = strdup(optarg); break;
      case 'h': opt_histo_buckets = atoi(optarg); break;
      case 'l': opt_loops = atoi(optarg); break;
      case 'n': opt_num_samples = atoi(optarg); break;
      case 'p': opt_pause = atoi(optarg); break;
      case 't': opt_timebase = atoi(optarg); break;
      case 'w': opt_warmup_loops = atoi(optarg); break;
      case 'v': opt_verbose = atoi(optarg); break;
      default: usage();
    }  /* switch opt */
  }  /* while getopt */
}  /* get_options */


/* Run context thread on a different core. */
void *my_ctx_thread(void *arg)
{
  if (opt_cpu_num >= 0) {
    jtr_pin_cpu(opt_cpu_num+2);  /* +2 keeps it on the same CPU chip. */
  }

  while (jtr_ctx_running) {
    LBME(lbm_context_process_events(jtr_ctx, 1000));
  }

  return NULL;
}  /* my_ctx_thread */


void lbm_send_cb(void *clientd)
{
  LBME(lbm_src_buffs_complete_and_acquire(jtr_src, (void **)&message_buf,
       opt_Msg_size, 0));
}  /* lbm_send_cb */


void null_send_cb(void *clientd)
{
  int i;

  for (i = 0; i < jtr_no_send_spin; i++) { } 
}  /* null_send_cb */


int main(int argc, char **argv)
{
  lbm_context_attr_t *ctx_attr;
  lbm_src_topic_attr_t *src_attr;
  lbm_topic_t *topic_obj; /* Handle for topic object. */
  int i;
  char title[1024];

  /* Parse command-line options. */
  get_options(argc, argv);

  jtr_y_high = opt_num_samples;

  /* Empty out the results buffer (and page it in). */
  for (i = 0; i < sizeof(jtr_results_buf); i++) {
    jtr_results_buf[i] = 0;
  }
  for (i = 0; i < sizeof(jtr_gnuplot_buf); i++) {
    jtr_gnuplot_buf[i] = '\0';
  }

  if (opt_cpu_num >= 0) {
    jtr_pin_cpu(opt_cpu_num);
  }

  if (opt_fifo_priority >= 0) { 
    jtr_set_fifo_priority(opt_fifo_priority);
  }

  LBME(lbm_config(JTR_CFG_FILE));

  LBME(lbm_context_attr_create(&ctx_attr));
  LBME(lbm_context_attr_str_setopt(ctx_attr, "operational_mode", "sequential"));
  LBME(lbm_context_create(&jtr_ctx, ctx_attr, NULL, NULL));
  LBME(lbm_context_attr_delete(ctx_attr));
  jtr_ctx_running = 1;
  SYSE(pthread_create(&jtr_ctx_thread_id, NULL, my_ctx_thread, NULL));

  LBME(lbm_src_topic_attr_create(&src_attr));
  LBME(lbm_src_topic_alloc(&topic_obj, jtr_ctx, JTR_TOPIC_STR, src_attr));
  LBME(lbm_src_create(&jtr_src, jtr_ctx, topic_obj, NULL, NULL, NULL));
  LBME(lbm_src_topic_attr_delete(src_attr));

  /* Let topic resolution complete. */
  jtr_spin_sleep_ns(NANOS_PER_SEC/10, 2);  /* Use clock_gettime() timebase. */

  /* Init message buffer into cache. */
  LBME(lbm_src_buff_acquire(jtr_src, (void **)&message_buf, opt_Msg_size, 0));
  for (i = 0; i < opt_Msg_size; i++) {
    message_buf[i] = (char)i;
  }

  for (i = 0; i < 20; i++) {
    jtr_calibrate();
  }

  /* Conduct the timing tests! */

  for (i = 0; i < opt_loops; i++) {
    jtr_no_send_spin = 0;
    jtr_histo_init(opt_histo_buckets);
    jtr_measure_calls(opt_warmup_loops, opt_num_samples, opt_pause,
                      opt_timebase, lbm_send_cb, NULL);

    snprintf(title, sizeof(title),
            "%s: Msg_size=%d,"
            " cpu_num=%d, fifo_priority=%d, histo_buckets=%d, num_samples=%d,\n"
            " pause=%d timebase=%d, warmup_loops=%d, gettime_cost=%lld,\n"
            " rdtsc_cost=%lld, ticks_per_sec=%lld, jtr_1000_loops_cost=%lld",
            opt_descr, opt_Msg_size,
            opt_cpu_num, opt_fifo_priority, opt_histo_buckets, opt_num_samples,
            opt_pause, opt_timebase, opt_warmup_loops, jtr_gettime_cost,
            jtr_rdtsc_cost, jtr_ticks_per_sec, jtr_1000_loops_cost);
    SYSE(title[sizeof(title)-2] != '\0'); /* Don't fill. */
    snprintf(&jtr_results_buf[strlen(jtr_results_buf)],
             sizeof(jtr_results_buf) - strlen(jtr_results_buf),
             "%s\n", title);
    SYSE(jtr_results_buf[sizeof(jtr_results_buf)-2] != '\0'); /* Don't fill. */
    jtr_histo_print_all(opt_verbose, title);

    jtr_no_send_spin = jtr_busy_loop_wait_count(jtr_histo_average);
    jtr_histo_init(opt_histo_buckets);
    jtr_measure_calls(opt_warmup_loops, opt_num_samples, opt_pause,
                      opt_timebase, null_send_cb, NULL);

    snprintf(title, sizeof(title),
            "%s (null loop): no_send_spin=%d,"
            " cpu_num=%d, fifo_priority=%d, histo_buckets=%d, num_samples=%d,\n"
            " pause=%d timebase=%d, warmup_loops=%d, gettime_cost=%lld,\n"
            " rdtsc_cost=%lld, ticks_per_sec=%lld, jtr_1000_loops_cost=%lld",
            opt_descr, jtr_no_send_spin,
            opt_cpu_num, opt_fifo_priority, opt_histo_buckets, opt_num_samples,
            opt_pause, opt_timebase, opt_warmup_loops, jtr_gettime_cost,
            jtr_rdtsc_cost, jtr_ticks_per_sec, jtr_1000_loops_cost);
    SYSE(title[sizeof(title)-2] != '\0'); /* Don't fill. */
    snprintf(&jtr_results_buf[strlen(jtr_results_buf)],
             sizeof(jtr_results_buf) - strlen(jtr_results_buf),
             "%s\n", title);
    SYSE(jtr_results_buf[sizeof(jtr_results_buf)-2] != '\0'); /* Don't fill. */
    jtr_histo_print_all(opt_verbose, title);
  }

  if (opt_verbose >= 0) {
    printf("%s", jtr_results_buf);

    if (opt_gnuplot_file != NULL) {
      FILE *gnuplot_fp = fopen(opt_gnuplot_file, "w");
      SYSE(gnuplot_fp == NULL);

      fprintf(gnuplot_fp, "%s", jtr_gnuplot_buf);
      fclose(gnuplot_fp);
    }
  }

  LBME(lbm_src_delete(jtr_src));

  /* Shut down context. */
  jtr_ctx_running = 0;
  LBME(lbm_context_unblock(jtr_ctx));
  SYSE(pthread_join(jtr_ctx_thread_id, NULL));
  LBME(lbm_context_delete(jtr_ctx));

  return 0;
}  /* main */
