/* jtr_sock.c - tool to measure system jitter.
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
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "jtr.h"

/* Options and their defaults. See jtr_getopts(). */
int opt_cpu_num = 3;
int opt_loops = 3;
int opt_msg_size = 1024;
int opt_num_msgs = 1000000;
int opt_pkt_delay = JTR_10G_XMIT_1024_PKT_NS;
int opt_warmup_loops = 500;
int opt_verbose = 0;

/* Options specific to the sock test. */
unsigned short opt_Destport = 0;  /* Required "option". */
unsigned long opt_Groupaddr = 0;  /* Required "option". */
unsigned long opt_Interface = 0;  /* Required "option". */
char opt_Ttl = -1;                /* Required "option". */

/* socket object. */
int mcast_sock;
struct sockaddr_in dest_in;
struct in_addr interface_in;
char jtr_ss_msg_buf[65536];

/* Other globals. */
int jtr_no_send_spin = 0;


void usage()
{
  fprintf(stderr, "Usage: jtr_sock -D destport -G groupaddr -I interface -T ttl [-c cpu_num] [-l loops] [-m msg_size] [-n num_msgs] [-p pkt_delay] [-w warmup_loops] [-v]\n");
  exit(1);
}  /* usage */


/* Parse command-line options.
 */
void jtr_getopts(int argc, char **argv)
{
  int opt;

  while ((opt = getopt(argc, argv, "D:G:I:T:c:l:m:n:p:w:v")) != EOF) {
    switch (opt) {
      case 'D': opt_Destport = atoi(optarg); break;
      case 'G': opt_Groupaddr = inet_addr(optarg);
        if (opt_Groupaddr == -1) {
          fprintf(stderr, "Bad IP address: -G %s\n", optarg);
          exit(1);
        }
        break;
      case 'I': opt_Interface = inet_addr(optarg); break;
        if (opt_Interface == -1) {
          fprintf(stderr, "Bad IP address: -I %s\n", optarg);
          exit(1);
        }
        break;
      case 'T': opt_Ttl = atoi(optarg); break;
      case 'c': opt_cpu_num = atoi(optarg); break;
      case 'l': opt_loops = atoi(optarg); break;
      case 'm': opt_msg_size = atoi(optarg); break;
      case 'n': opt_num_msgs = atoi(optarg); break;
      case 'p': opt_pkt_delay = atoi(optarg); break;
      case 'w': opt_warmup_loops = atoi(optarg); break;
      case 'v': opt_verbose = 1; break;
      default: usage();
    }  /* switch opt */
  }  /* while getopt */

  if (opt_Destport == 0) { fprintf(stderr, "Missing -D destport\n"); exit(1); }
  if (opt_Groupaddr == 0) { fprintf(stderr, "Missing -G groupaddr\n"); exit(1); }
  if (opt_Interface == 0) { fprintf(stderr, "Missing -I interface\n"); exit(1); }
  if (opt_Ttl == -1) { fprintf(stderr, "Missing -T ttl\n"); exit(1); }
}  /* jtr_getopts */


void sock_send_cb(void *clientd)
{
  int send_rtn = sendto(mcast_sock, jtr_ss_msg_buf, opt_msg_size, 0,
                        (struct sockaddr *)&dest_in, sizeof(dest_in));
  SYSE(send_rtn == -1);
}  /* sock_send_cb */


void null_send_cb(void *clientd)
{
  int i;

  for (i = 0; i < jtr_no_send_spin; i++) { } 
}  /* null_send_cb */


int main(int argc, char **argv)
{
  int i;

  /* Parse command-line options. */
  jtr_getopts(argc, argv);

  mcast_sock = socket(PF_INET, SOCK_DGRAM, 0);
  SYSE(mcast_sock == -1);
  memset((char *)&dest_in, 0, sizeof(dest_in));
  dest_in.sin_family = AF_INET;
  dest_in.sin_port = htons(opt_Destport);
  dest_in.sin_addr.s_addr = opt_Groupaddr;

  SYSE(setsockopt(mcast_sock, IPPROTO_IP, IP_MULTICAST_TTL,
                  (char *)&opt_Ttl, sizeof(opt_Ttl)));

  memset((char *)&interface_in,0,sizeof(interface_in));
  interface_in.s_addr = opt_Interface;
  SYSE(setsockopt(mcast_sock, IPPROTO_IP, IP_MULTICAST_IF,
                  (const char*)&interface_in, sizeof(interface_in)));

  /* Empty out the results buffer (and page it in). */
  for (i = 0; i < sizeof(jtr_results_buf); i++) {
    jtr_results_buf[i] = 0;
  }

  if (opt_cpu_num >= 0) {
    jtr_pin_cpu(opt_cpu_num);
  }

  /* Get message buffer into cache. */
  for (i = 0; i < opt_msg_size; i++) {
    jtr_ss_msg_buf[i] = (char)i;
  }

  for (i = 0; i < 20; i++) {
    jtr_calibrate();
  }

  snprintf(&jtr_results_buf[strlen(jtr_results_buf)],
           sizeof(jtr_results_buf),
           "cpu_num=%d, msg_size=%d, num_msgs=%d, pkt_delay=%d warmup_loops=%d, gettime_cost=%lld, jtr_1000_loops_cost=%lld\n",
           opt_cpu_num, opt_msg_size, opt_num_msgs, opt_pkt_delay, opt_warmup_loops, jtr_gettime_cost, jtr_1000_loops_cost);
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
    jtr_measure_calls(opt_warmup_loops, opt_num_msgs, opt_pkt_delay, sock_send_cb, NULL);
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

  printf("%s", jtr_results_buf);

  close(mcast_sock);

  return 0;
}  /* main */
