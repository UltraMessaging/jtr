/* jtr_lbm.c - tool to measure Ultra Messaging jitter.
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

/* Options and their defaults. See jtr_getopts(). */
int opt_cpu_num = 3;
int opt_fifo_priority = -1;
int opt_loops = 3;
int opt_msg_size = 1024;
int opt_num_msgs = 1000000;
int opt_pkt_delay = JTR_10G_XMIT_1024_PKT_NS;
int opt_warmup_loops = 500;
int opt_verbose = 0;

/* UM objects. */
lbm_context_t *jtr_ctx;     /* Handle for context object. */
lbm_ssrc_t *jtr_ssrc;       /* Handle for source (sender) object. */
char *jtr_ss_msg_buf;
lbm_ssrc_send_ex_info_t jtr_send_ex_info;

/* Other globals. */
pthread_t jtr_ctx_thread_id;
int jtr_ctx_running = 0;
int jtr_no_send_spin = 0;


void usage()
{
  fprintf(stderr, "Usage: jtr_lbm [-c cpu_num] [-f fifo_priority] [-l loops] [-m msg_size] [-n num_msgs] [-p pkt_delay] [-w warmup_loops] [-v]\n");
  exit(1);
}  /* usage */


/* Parse command-line options.
 */
void jtr_getopts(int argc, char **argv)
{
  int opt;

  while ((opt = getopt(argc, argv, "c:f:l:m:n:p:w:v:")) != EOF) {
    switch (opt) {
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
  LBME(lbm_ssrc_send_ex(jtr_ssrc, jtr_ss_msg_buf,
                        opt_msg_size, 0, &jtr_send_ex_info));
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

  /* Parse command-line options. */
  jtr_getopts(argc, argv);

  /* Empty out the results buffer (and page it in). */
  for (i = 0; i < sizeof(jtr_results_buf); i++) {
    jtr_results_buf[i] = 0;
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
  LBME(lbm_ssrc_create(&jtr_ssrc, jtr_ctx, topic_obj, NULL, NULL, NULL));
  LBME(lbm_src_topic_attr_delete(src_attr));

  LBME(lbm_ssrc_buff_get(jtr_ssrc, &jtr_ss_msg_buf, 0));
  memset(&jtr_send_ex_info, 0, sizeof(lbm_ssrc_send_ex_info_t));

  /* Get message buffer into cache. */
  for (i = 0; i < opt_msg_size; i++) {
    jtr_ss_msg_buf[i] = (char)i;
  }

  /* Let topic resolution complete. */
  jtr_spin_sleep_ns(NANOS_PER_SEC);

  for (i = 0; i < 20; i++) {
    jtr_calibrate();
  }

  snprintf(&jtr_results_buf[strlen(jtr_results_buf)],
           sizeof(jtr_results_buf),
           "cpu_num=%d, fifo_priority=%d, msg_size=%d, num_msgs=%d, pkt_delay=%d warmup_loops=%d, gettime_cost=%lld, jtr_1000_loops_cost=%lld\n",
           opt_cpu_num, opt_fifo_priority, opt_msg_size, opt_num_msgs, opt_pkt_delay, opt_warmup_loops, jtr_gettime_cost, jtr_1000_loops_cost);
  SYSE(jtr_results_buf[sizeof(jtr_results_buf)-2] != '\0'); /* Mustn't be full. */

  /* Conduct the timing tests! */

  for (i = 0; i < opt_loops; i++) {
    jtr_no_send_spin = 0;
    snprintf(&jtr_results_buf[strlen(jtr_results_buf)],
             sizeof(jtr_results_buf),
             "\njtr_no_send_spin=%d\n",
             jtr_no_send_spin);
    SYSE(jtr_results_buf[sizeof(jtr_results_buf)-2] != '\0'); /* Mustn't be full. */
    jtr_histo_init();
    jtr_measure_calls(opt_warmup_loops, opt_num_msgs, opt_pkt_delay, lbm_send_cb, NULL);
    jtr_histo_print_all(opt_verbose);

    jtr_no_send_spin = jtr_busy_loop_wait_count(jtr_histo_average);
    snprintf(&jtr_results_buf[strlen(jtr_results_buf)],
             sizeof(jtr_results_buf),
             "\njtr_no_send_spin=%d\n",
             jtr_no_send_spin);
    SYSE(jtr_results_buf[sizeof(jtr_results_buf)-2] != '\0'); /* Mustn't be full. */
    jtr_histo_init();
    jtr_measure_calls(opt_warmup_loops, opt_num_msgs, opt_pkt_delay, null_send_cb, NULL);
    jtr_histo_print_all(opt_verbose);
  }

  if (opt_verbose >= 0) {
    printf("%s", jtr_results_buf);
  }

  LBME(lbm_ssrc_delete(jtr_ssrc));

  /* Shut down context. */
  jtr_ctx_running = 0;
  LBME(lbm_context_unblock(jtr_ctx));
  SYSE(pthread_join(jtr_ctx_thread_id, NULL));
  LBME(lbm_context_delete(jtr_ctx));

  return 0;
}  /* main */
