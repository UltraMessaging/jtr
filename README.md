# jtr v0.1 README - Jitter measurement tools for Ultra Messaging

Tools to measure latency jitter (outliers).
Although these tools were designed for use with Ultra Messaging,
they can be used to measure system-level jitter.
This is useful during system tuning to evaluate different system
configurations.

## IMPORTANT WARNING

These tools transmit data onto your network at extreme rates.
It can SEVERELY DISRUPT YOUR ENTIRE NETWORK if you are not careful.
You MUST configure the test to use a multicast group that your network
administrators have allocated for very high rate tests.
In particular, the group should be blocked by the switch from being
propagated to other networks over bandwidth-limited links.

Some people suggest using the Multicast TTL to prevent the propagation of
multicast traffic.
This is fine for low-rate publishers of a few packets per second,
but TTL-based discarding of packets is often done by the switch's supervisory
processor, and high-rate discards can overwhelm a switch's supervisory
processor.
It is generally considered better to use rules to allow or prevent the
propagation of specific multicast groups.
Modern switches can do this in the switch hardware without involving the
supervisory processor.

These tests make use of a standard Ultra Messaging configuration file.
So you should edit "streaming.cfg" and update the first two groups of options,
paying special attention to:
* context resolver_multicast_address
* context transport_lbtrm_multicast_address_low
* context transport_lbtrm_multicast_address_high
* source transport_lbtrm_destination_port
* context resolver_multicast_interface

## Preparation

* Create "lbm.lic" file containing a standard UM license key.
* Modify "lbm.sh", setting RELPATH and PLATFORM to point to your
installation of UM.
* Modify "streaming.cfg", setting the network-oriented options to values
that will not interfere with your network.
See [IMPORTANT WARNING](important-warning).
* Build the tools by running "bld.sh".
* To test setting FIFO priority to high values (like 99), you can
change ownership of the resulting executable files to root and
set the "suid" bit:
```
sudo chown 0:0 jtr_null jtr_sock jtr_lbm
sudo chmod u+s jtr_null jtr_sock jtr_lbm
```
This changes the process to root as it executes the program.
But to run the jtr_lbm as root will require installing UM itself
as a system package.
For example, assuming your 6.12.1 package is in /tmp:
```
sudo cd /usr/local; /tmp/UMP_6.12.1_Linux-glibc-2.17-x86_64.sh
echo "/usr/local/UMP_6.12.1/Linux-glibc-2.17-x86_64/lib" >/etc/ld.so.conf.d/lbm.conf
ldconfig
```

## Initial Tests

The basic flow of a test is to take an action that we want to time,
and execute it 1 million times,
using clock_gettime() to get timestamps before and after executing it.
The difference between end and start times is a sample,
which is added to a histogram.

The program's test thread pins itself to a core using sched_setaffinity().
By default it chooses core number 3, and is controlled with the "-c" option.
(You can also inhibit the pinning by supplying core number -1 using
"-c -1".
But this often leads to increased jitter, so users are advised to pin.)

Note that the build script does *not* set a high level of optimization.
We aren't trying to get the test program to execute as fast as possible,
we are trying to measure latency variation (jitter).

### Test 1: 100-loop busy spin:
```
./tst_null.sh
```
This essentially measures your processor speed, and the base jitter in
your system.

By default, the program runs the test 3 times,
so you will see 3 sets of results.
Each test consists of doing 1 million samples, where each sample consists
of measuring the time to execute:
```
for (i = 0; i < 100; i++) {}
```
On a system tuned for low jitter, that 100 cycle loop should execute for the
same amount of time each of the million times it is called.
We include some results from our system named "Crush",
which is partially tuned for low jitter, but there is more we could do.

Here's an example result (first of three) from our machine "Crush":
```
$ ./tst_null.sh
tst_null.sh 
tst_null.sh 
Busy_spins=100, fifo_priority=-1, cpu_num=3, msg_size=1024, num_msgs=1000000, pkt_delay=898 warmup_loops=500, gettime_cost=15, jtr_1000_loops_cost=1680
Minimum=170, Maximum=1219, Average=173, Overflows=0
90.000% are below 180 ns
99.000% are below 180 ns
99.900% are below 180 ns
99.990% are below 280 ns
99.999% are below 600 ns
```

Interesting measurements are:
* gettime_cost - overhead in measuring system time using clock_gettime().
Our system "Crush" measures 15 nanoseconds.
* Minimum - Out of 1 million samples, the least time measured to execute the
for loop.
On Crush, we measure 171 nanoseconds.
* Maximum - Out of 1 million samples, the most time measured.
This value can vary widely on systems that have not been tuned for low jitter.
On Crush, I've seen runs of 1 million messages with a maximum as low as 932
and as high as 9664.
(When "-c 0" is used to force the test to run on CPU 0,
the maximum can be MUCH higher.
We've seen values as high as 25325.)
* Average - Out of 1 million samples, the average measurement.
Crush measures 197 nanoseconds.
It's the outliers that pushes the value up.
* 99.999% - Out of 1 million samples, 999,990 were below this time;
10 were at or above.
This value can range widely on systems that have not been tuned for low jitter.
On Crush, I've seen runs as low as 550 nanoseconds and as high as 980
nanoseconds.

It can also be interesting to see the full histogram of latencies with
the "-v" verbose option:
```
./tst_null.sh -v
```
On Crush, there are two timing peaks at buckets 170 and 200.
We don't know why that is, although it might be related timer resolution.

### Test 2: UM Smart Source without socket
```
./tst_lbmnosock.sh
```
This measures the execution time of the UM library Smart Source send code,
*NOT* including the socket call.
I.e. the socket send is skipped,
and no packet is sent.

Note that there are 3 pairs of results.
Each pair consists of running a test of the UM code,
and then running a test of a "for" loop of approximately the same timing.

For example, here is the startup messages and the first pair of results
from a run on our machine "Crush":
```
$ ./tst_lbmnosock.sh
tst_lbmnosock.sh 
Core-7911-1: Onload extensions API has been dynamically loaded
cpu_num=3, msg_size=1024, num_msgs=1000000, pkt_delay=898 warmup_loops=500, gettime_cost=15, jtr_1000_loops_cost=1700

jtr_no_send_spin=0
Minimum=76, Maximum=1303, Average=160, Overflows=0
90.000% are below 170 ns
99.000% are below 420 ns
99.900% are below 560 ns
99.990% are below 670 ns
99.999% are below 800 ns

jtr_no_send_spin=94
Minimum=175, Maximum=4223, Average=183, Overflows=0
90.000% are below 210 ns
99.000% are below 210 ns
99.900% are below 210 ns
99.990% are below 360 ns
99.999% are below 860 ns
```

The first set of results is for jtr_no_send_spin=0,
which means that the UM Smart Source send function is called.
For the second set, a "for" loop is substituted for the UM send function,
with a number of loops chosen to be approximately the average time measured
for the UM send.

As you can see, there is some variation in the time for Smart Source send,
depending on what the code needs to do.
However, there is also variation in the "for" loop, which should be
invariant.
The outliers in the 99.99% and 99.999% are somewhat due to system-level
jitter.

### Test 3: Kernel Socket
```
./tst_sock.sh
```
This measures the execution time of the operating system's socket
"sendto()" API.
It does *not* include any Ultra Messaging code.

As with test 2, the results are reported in pairs.
For example, here's the first pair:
```
$ ./tst_sock.sh
tst_sock.sh -D 12000 -G 239.101.3.10 -I 10.29.4.53 -T 15 
cpu_num=3, msg_size=1024, num_msgs=1000000, pkt_delay=898 warmup_loops=500, gettime_cost=15, jtr_1000_loops_cost=1679

jtr_no_send_spin=0
Minimum=1991, Maximum=12214, Average=4824, Overflows=686
90.000% are below 5070 ns
99.000% are below 6870 ns
99.900% are below 7830 ns
Warning, historgram overflow for 99.990% (too many samples >= 9000)
Warning, historgram overflow for 99.999% (too many samples >= 9000)

jtr_no_send_spin=2873
Minimum=4792, Maximum=9674, Average=4862, Overflows=4
90.000% are below 4800 ns
99.000% are below 5600 ns
99.900% are below 5600 ns
99.990% are below 5700 ns
99.999% are below 5720 ns
```

As with test 2, the first set of results is for actual socket calls.
The second is for a "for" loop of the same average duration.
As you can see, the outliers generated from normal socket calls
overflow the histogram, with a maximum time of over 12 microseconds.

### Test 4: UM with Kernel Socket
```
./tst_lbmsock.sh
```
This measures the execution time of UM and the operating system's
socket.

As before, the results are in pairs.
```
$ ./tst_lbmsock.sh
tst_lbmsock.sh 
Core-7911-1: Onload extensions API has been dynamically loaded
cpu_num=3, msg_size=1024, num_msgs=1000000, pkt_delay=898 warmup_loops=500, gettime_cost=15, jtr_1000_loops_cost=1736

jtr_no_send_spin=0
Minimum=2397, Maximum=16324, Average=5239, Overflows=1009
90.000% are below 5550 ns
99.000% are below 7190 ns
Warning, historgram overflow for 99.900% (too many samples >= 9000)
Warning, historgram overflow for 99.990% (too many samples >= 9000)
Warning, historgram overflow for 99.999% (too many samples >= 9000)

jtr_no_send_spin=3017
Minimum=5403, Maximum=10512, Average=5568, Overflows=4
90.000% are below 6240 ns
99.000% are below 6250 ns
99.900% are below 6340 ns
99.990% are below 6340 ns
99.999% are below 7140 ns
```

The average is a little more than the average UM with no socket call
plus the socket call with no UM.
We aren't sure exactly why that is.

### Test 5: Onload Socket
```
./tst_onload.sh
```
This measures the execution time of Solarflare's Onload
"sendto()" API.
It does *not* include any Ultra Messaging code.

As with test 2, the results are reported in pairs.
For example, here's the first pair:
```
$ ./tst_onload.sh
tst_sock.sh -D 12000 -G 239.101.3.10 -I 10.29.4.53 -T 15 
onload: env: EF_SPIN_USEC=-1
onload: env: EF_MCAST_SEND=2
onload: env: LD_PRELOAD=libonload.so
onload: env: LD_LIBRARY_PATH=/home/sford/UMP_6.12/Linux-glibc-2.17-x86_64/lib
onload: env: EF_TIMESTAMPING_REPORTING=1
onload: env: EF_UDP_SEND_SPIN=1
onload: env: EF_TX_TIMESTAMPING=1
onload: env: EF_RX_TIMESTAMPING=1
onload: invoking: ./jtr_sock -D 12000 -G 239.101.3.10 -I 10.29.4.53 -T 15
oo:jtr_sock[66306]: Using OpenOnload 201811 Copyright 2006-2018 Solarflare Communications, 2002-2005 Level 5 Networks [4]
cpu_num=3, msg_size=1024, num_msgs=1000000, pkt_delay=898 warmup_loops=500, gettime_cost=15, jtr_1000_loops_cost=1680

jtr_no_send_spin=0
Minimum=268, Maximum=8127, Average=330, Overflows=0
90.000% are below 310 ns
99.000% are below 2500 ns
99.900% are below 2750 ns
99.990% are below 2820 ns
99.999% are below 2980 ns

jtr_no_send_spin=196
Minimum=331, Maximum=1659, Average=353, Overflows=0
90.000% are below 400 ns
99.000% are below 400 ns
99.900% are below 400 ns
99.990% are below 500 ns
99.999% are below 1050 ns
```

As you can see, the results are *MUCH* better than using kernel sockets.
with an average of 330 nanoseconds.
But there are significant numbers of outliers starting in the 99%.
In contrast, the "for" loop stays low into the 99.99%.
(Full disclosure, Crush's Onload driver has not been updated in a few
years and the settings have not been optimized for low jitter.)

### Test 6: UM with Onload Socket
```
./tst_lbmonload.sh
```
This measures the execution time of UM and Solarflare's Onload
socket.

```
$ ./tst_lbmonload.sh
tst_lbmonload.sh 
onload: env: EF_SPIN_USEC=-1
onload: env: EF_MCAST_SEND=2
onload: env: LD_PRELOAD=libonload.so
onload: env: LD_LIBRARY_PATH=/home/sford/UMP_6.12/Linux-glibc-2.17-x86_64/lib
onload: env: EF_TIMESTAMPING_REPORTING=1
onload: env: EF_UDP_SEND_SPIN=1
onload: env: EF_TX_TIMESTAMPING=1
onload: env: EF_RX_TIMESTAMPING=1
onload: invoking: ./jtr_lbm
oo:jtr_lbm[109194]: Using OpenOnload 201811 Copyright 2006-2018 Solarflare Communications, 2002-2005 Level 5 Networks [6]
Core-7911-1: Onload extensions API has been dynamically loaded
cpu_num=3, msg_size=1024, num_msgs=1000000, pkt_delay=898 warmup_loops=500, gettime_cost=15, jtr_1000_loops_cost=1727

jtr_no_send_spin=0
Minimum=334, Maximum=23388, Average=492, Overflows=27
90.000% are below 480 ns
99.000% are below 2920 ns
99.900% are below 3330 ns
99.990% are below 3610 ns
Warning, historgram overflow for 99.999% (too many samples >= 9000)

jtr_no_send_spin=284
Minimum=514, Maximum=3210, Average=524, Overflows=0
90.000% are below 530 ns
99.000% are below 600 ns
99.900% are below 600 ns
99.990% are below 730 ns
99.999% are below 1420 ns
```

## Error handling

The test programs use a crude form of error handling in the form of
two macros: LBME() and SYSE().
Both of them are designed to print some useful error information and then
trigger a core dump by calling abort().
The core dump is intended to make it easier to debug the issue in the
event of a bug in the test code.

To see it in action, you can try to pin the thread to an illegal CPU:
```
./tst_null.sh -c 999
```

## NIC Speed

The tests default to assuming that the NIC is 10G.
This becomes important when sending packets at high rates.
Modern servers can attempt to send packets faster than the NIC
can transmit them.
This normally leads to blocking the sender when the send-side socket
buffer fills, which interferes with the latency measurements.
So these tools introduce a short delay between sends to allow the
packet to exit the NIC.
This delay defaults to 898 nanoseconds (about 1 microsecond),
which is how long a 1K user message takes over a 10G network.

When using a 1G NIC, you should supply the option "-p 10000".
This increases the delay to 10 microseconds.

## Onload

The test scripts named "*onload.sh" use the "onload" command to
test with the Solarflare kernel-bypass library.
Note that those scripts assume a very simple Onload configuration.
Feel free to modify them to match your deployment patterns.
