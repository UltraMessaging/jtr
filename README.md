# jtr v0.1 README - Jitter measurement tools for Ultra Messaging

Tools to measure latency jitter (outliers).
Although these tools were designed for use with Ultra Messaging,
they can be used to measure system-level jitter.
This is useful during system tuning to evaluate different
configurations.

There is a PowerPoint presentation that gives an overview of the package
and the measurements, plus several lessons learned during their development.
See "jtr.pptx".

## IMPORTANT WARNING

These tools transmit multicast data onto your network at extreme rates.
They can SEVERELY DISRUPT YOUR ENTIRE NETWORK if you are not careful.
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

* context resolver\_multicast\_address
* context transport\_lbtrm\_multicast\_address\_low
* context transport\_lbtrm\_multicast\_address\_high
* source transport\_lbtrm\_destination\_port
* context resolver\_multicast\_interface

Note that the non-UM socket test also uses the UM configuration file to
determine the multicast group and port.
I.e. even if you aren't using UM and only intend to use the socket tests,
you should update the UM configuration file with multicast addresses and
ports.

## Preparation

To use the UM-based tool, you need UM version 6.10 or newer.
These tools make use of the Smart Source feature, which was first
introduced in UM version 6.10.
We recommend the latest version of UM.

Also, if you want pretty graphs, install gnuplot.
This package has scripts which graph the data from each test.

The "jtr\_null" and "jtr\_sock" tools do not use UM,
so they can be built and run without UM.

Set up tests:

* Create "lbm.lic" file containing a standard UM license key.
* Modify "lbm.sh", setting RELPATH and PLATFORM to point to your
installation of UM.
* Modify "streaming.cfg", setting the network-oriented options to values
that will not interfere with your network.
In particular, modify the first two groups of options.
See [IMPORTANT WARNING](#important-warning).
* Build the tools by running "bld.sh".
* To test setting FIFO priority to high values (like 99),
you'll need root access.
See [Root Usage](#root-usage).

## Initial Tests

The basic flow of a test is to take an action that we want to time,
and execute it 1 million times,
using "RDTSC" to get timestamps before and after each execution.
The difference between end and start times is a sample,
which is added to a histogram.

The program's test thread pins itself to a core using "sched\_setaffinity()".
(This is the same thing that happens when you run with "taskset".)
By default it chooses core number 5, and is controlled with the "-c" option.
(You can also inhibit pinning by specifying core number "-1" using
"-c -1".
But this often leads to increased jitter, so users are advised to pin.)

Note that the build script does not compile with optimization.
The code contains empty "for" loops, which it uses for several purposes.
The optimizer will optimize away the empty "for" loop, and the test
programs won't work correctly.
We aren't trying to get the test program to execute as fast as possible,
we are trying to measure latency variation (jitter).

You can run all of the tests with:

    ./tstall.sh

You can then produce all of the graphs with:

    ./plotall.sh

The compressed tar file "crush.tz" contains the output files associated
with running these tools on our host named "crush" which has been
tuned for low jitter.
Unpack the tar file using:

    tar xzf crush.tz

and look at the files "*.txt" (summaries), "*.gp" (histogram
counts), and "*.png" (graphs).

Note that the system was in use for general testing while these
measurements were taken, so the amount of jitter is higher than it would
be if the system were otherwise unused.

Alternatively, you can run tests individually.

### Test 1: 100-Cycle For Loop

Take a million samples of the execution time for a 100-cycle empty "for" loop.

    ./tst_null.sh

This essentially measures your processor speed, and the base jitter in
your system.
The results are saved in "tst\_null.txt" (summary) and "tst\_null.gp"
(histogram counts).

By default, the program runs the test 3 times,
so you will see 3 sets of results.
Each test consists of doing 1 million samples, where each sample consists
of measuring the time to execute:

    for (i = 0; i < 100; i++) {}

On a system tuned for low jitter, that 100 cycle loop should execute for the
same amount of time each of the million times it is called.
We include some results from our system named "Crush",
which is partially tuned for low jitter, but there is more we could do.

Here's an example result from our host "Crush":

    $ ./tst_null.sh
    $ cat tst_null.txt
    tst_null.sh: Busy_spins=100, cpu_num=5, fifo_priority=-1, histo_buckets=200, num_samples=1000000,
     pause=898 timebase=1, warmup_loops=500, gettime_cost=15,
     rdtsc_cost=6, ticks_per_sec=3399967548, jtr_1000_loops_cost=1675
    Minimum=171, Maximum=202, Average=187, Overflows=0
    90.000% are below 210 ns
    99.000% are below 210 ns
    99.900% are below 210 ns
    99.990% are below 210 ns
    99.999% are below 210 ns
    tst_null.sh: Busy_spins=100, cpu_num=5, fifo_priority=-1, histo_buckets=200, num_samples=1000000,
     pause=898 timebase=1, warmup_loops=500, gettime_cost=15,
     rdtsc_cost=6, ticks_per_sec=3399967548, jtr_1000_loops_cost=1675
    Minimum=170, Maximum=1477, Average=174, Overflows=0
    90.000% are below 180 ns
    99.000% are below 180 ns
    99.900% are below 180 ns
    99.990% are below 180 ns
    99.999% are below 990 ns
    tst_null.sh: Busy_spins=100, cpu_num=5, fifo_priority=-1, histo_buckets=200, num_samples=1000000,
     pause=898 timebase=1, warmup_loops=500, gettime_cost=15,
     rdtsc_cost=6, ticks_per_sec=3399967548, jtr_1000_loops_cost=1675
    Minimum=172, Maximum=1371, Average=185, Overflows=0
    90.000% are below 210 ns
    99.000% are below 210 ns
    99.900% are below 210 ns
    99.990% are below 210 ns
    99.999% are below 210 ns

Interesting measurements are:
* ticks\_per\_sec - the RDTSC instruction returns these ticks.
Should be the same as the CPU clock frequency.
* rdtsc\_cost - overhead in measuring system time using RDTSC.
Our system "Crush" measures 6 nanoseconds.
* Minimum - Out of 1 million samples, the least time measured to execute the
for loop.
On Crush, we measure 171 nanoseconds.
* Maximum - Out of 1 million samples, the most time measured.
This value can vary widely on systems that have not been tuned for low jitter.
On Crush, I've seen runs of 1 million messages with a maximum as low as 201
and as high as 9664.
(When "-c 0" is used to force the test to run on CPU 0,
the maximum can be MUCH higher.
We've seen values as high as 25325.)
* Average - Out of 1 million samples, the average measurement.
Crush measures averages in the 170-190 nanosecond range.
It's the outliers that pushes the value up.
* 99.999% - Out of 1 million samples, 999,990 were below this time;
10 were at or above.
This value can range widely on systems that have not been tuned for low jitter.
On Crush, I've seen runs as low as 550 nanoseconds and as high as 980
nanoseconds.

It can also be interesting to see the full histogram of latencies:

    $ cat tst_null.gp
    # title_1 = "tst\_null.sh: Busy\_spins=100, cpu\_num=5, fifo\_priority=-1, histo\_buckets=200, num\_samples=1000000,\n pause=898 timebase=1, warmup\_loops=500, gettime\_cost=15,\n rdtsc\_cost=6, ticks\_per\_sec=3399967548, jtr\_1000\_loops\_cost=1675\nMinimum=171, Maximum=202, Average=187, Overflows=0"
    # xrange_1 = 2000
    170 500000
    190 83334
    200 416666
    
    
    # title_2 = "tst\_null.sh: Busy\_spins=100, cpu\_num=5, fifo\_priority=-1, histo\_buckets=200, num\_samples=1000000,\n pause=898 timebase=1, warmup\_loops=500, gettime\_cost=15,\n rdtsc\_cost=6, ticks\_per\_sec=3399967548, jtr\_1000\_loops\_cost=1675\nMinimum=170, Maximum=1477, Average=174, Overflows=0"
    # xrange_2 = 2000
    170 999970
    180 1
    190 3
    200 15
    980 1
    990 5
    1000 1
    1010 1
    1030 2
    1470 1
    
    
    # title_3 = "tst\_null.sh: Busy\_spins=100, cpu\_num=5, fifo\_priority=-1, histo\_buckets=200, num\_samples=1000000,\n pause=898 timebase=1, warmup\_loops=500, gettime\_cost=15,\n rdtsc\_cost=6, ticks\_per\_sec=3399967548, jtr\_1000\_loops\_cost=1675\nMinimum=172, Maximum=1371, Average=185, Overflows=0"
    # xrange_3 = 2000
    170 580708
    190 3
    200 419281
    1090 3
    1100 3
    1130 1
    1370 1

(The odd formatting is due to gnuplot requirements.)

To see the graphs for the three test runs:

    $ ./plot_null.sh

Then look at plot\_null-1.png, plot\_null-2.png, and plot\_null-3.png.

### Test 2: UM Smart Source Without Socket

Take a million samples of the execution time for a call to UM Smart
Source send function.

    ./tst_lbmnosock.sh

This measures the execution time of the UM library Smart Source send code,
*NOT* including the socket call.
I.e. the socket send is skipped,
and no packet is sent.

In this (and most other) tests, there are six tests run.
Three of them test a type of send function, and the other three
test an empty "for" loop set to the same average duration.
These results are presented in pairs, one for the send function
followed by the null loop.

For example, here is the startup messages and the first pair of results
from a run on our host "Crush":

    $ cat tst_lbmnosock.txt
    tst_lbmnosock.sh: Msg_size=1024, cpu_num=0, fifo_priority=-1, histo_buckets=200, num_samples=1000000,
     pause=898 timebase=1, warmup_loops=500, gettime_cost=16,
     rdtsc_cost=6, ticks_per_sec=3399967858, jtr_1000_loops_cost=1673
    Minimum=71, Maximum=1796, Average=82, Overflows=0
    90.000% are below 100 ns
    99.000% are below 100 ns
    99.900% are below 110 ns
    99.990% are below 220 ns
    99.999% are below 1290 ns
    tst_lbmnosock.sh (null loop): no_send_spin=49, cpu_num=0, fifo_priority=-1, histo_buckets=200, num_samples=1000000,
     pause=898 timebase=1, warmup_loops=500, gettime_cost=16,
     rdtsc_cost=6, ticks_per_sec=3399967858, jtr_1000_loops_cost=1673
    Minimum=59, Maximum=2986, Average=77, Overflows=2
    90.000% are below 100 ns
    99.000% are below 100 ns
    99.900% are below 100 ns
    99.990% are below 110 ns
    99.999% are below 1460 ns
    tst_lbmnosock.sh: Msg_size=1024, cpu_num=0, fifo_priority=-1, histo_buckets=200, num_samples=1000000,
     pause=898 timebase=1, warmup_loops=500, gettime_cost=16,
     rdtsc_cost=6, ticks_per_sec=3399967858, jtr_1000_loops_cost=1673
    Minimum=71, Maximum=1468, Average=82, Overflows=0
    90.000% are below 100 ns
    99.000% are below 100 ns
    99.900% are below 110 ns
    99.990% are below 210 ns
    99.999% are below 1310 ns
    tst_lbmnosock.sh (null loop): no_send_spin=49, cpu_num=0, fifo_priority=-1, histo_buckets=200, num_samples=1000000,
     pause=898 timebase=1, warmup_loops=500, gettime_cost=16,
     rdtsc_cost=6, ticks_per_sec=3399967858, jtr_1000_loops_cost=1673
    Minimum=59, Maximum=1784, Average=63, Overflows=0
    90.000% are below 70 ns
    99.000% are below 70 ns
    99.900% are below 70 ns
    99.990% are below 100 ns
    99.999% are below 1450 ns
    tst_lbmnosock.sh: Msg_size=1024, cpu_num=0, fifo_priority=-1, histo_buckets=200, num_samples=1000000,
     pause=898 timebase=1, warmup_loops=500, gettime_cost=16,
     rdtsc_cost=6, ticks_per_sec=3399967858, jtr_1000_loops_cost=1673
    Minimum=70, Maximum=1506, Average=82, Overflows=0
    90.000% are below 100 ns
    99.000% are below 100 ns
    99.900% are below 110 ns
    99.990% are below 210 ns
    99.999% are below 1320 ns
    tst_lbmnosock.sh (null loop): no_send_spin=49, cpu_num=0, fifo_priority=-1, histo_buckets=200, num_samples=1000000,
     pause=898 timebase=1, warmup_loops=500, gettime_cost=16,
     rdtsc_cost=6, ticks_per_sec=3399967858, jtr_1000_loops_cost=1673
    Minimum=59, Maximum=4880, Average=63, Overflows=1
    90.000% are below 70 ns
    99.000% are below 70 ns
    99.900% are below 70 ns
    99.990% are below 100 ns
    99.999% are below 1450 ns

The first set of results gives timing for the UM Smart Source send function.
The second set of results gives timing for a 49-cycle empty "for" loop.
The file continues with 4 more sets of results: send, null, send, and null.

To see the graphs:

    ./plot_lbmnosock_v_null.sh

Then look at plot\_lbmnosock\_v\_null-1.png, plot\_lbmnosock\_v\_null-2.png, and
plot\_lbmnosock\_v\_null-3.png.
Each graph corresponds to one pair of results.

As you can see, there is some variation in the time for Smart Source send,
depending on what the code needs to do.
However, there is also variation in the "for" loop, which should be
invariant.
The outliers in the 99.99% and 99.999% are somewhat due to system-level
jitter.

### All Tests

* tst\_clock.sh - Demonstrates the impact of using clock\_gettime() to
measure latencies. It can introduce multi-microsecond latencies of its own.
* tst\_lbmnosock.sh - UM Smart Source send call with NO socket call.
* tst\_lbmnosock500.sh - Short (500-sample) lbmnosocksend.
* tst\_lbmnosockclock.sh - UM Smart Source send call using clock\_gettime() to
measure latencies. Demonstrates the impact of clock\_gettime().
* tst\_lbmnosockdeftwb.sh - UM Smart Source send call with NO socket call.
But uses the default 32MB transmission window buffer. Demonstrates that large
buffers increase latencies due to increased cache misses.
* tst\_lbmonload.sh - UM Smart Source send call using Onload socket.
* tst\_lbmsock.sh - UM Smart Source send call using kernel socket.
* tst\_null.sh - 100-cycle empty "for" loop. Demonstrates base jitter of system.
* tst\_onload.sh - Onload socket send (no UM).
* tst\_sock.sh - Kernel socket send (no UM).

Note that there are only 3 C programs, "jtr\_null", "jtr\_sock", and "jtr\_lbm".
The 10 test scripts each runs one of the 3 C programs in a certain way to
produce the desired results.

For example, the "tst\_lbmnosock.sh" script runs the "jtr\_lbm" program
setting 100% source-side loss, which suppresses calls to the socket.
I.e. it is measuring the UM code only; no packets are actually sent.

In contrast, the "lbm\_sock.sh" script runs the same "jtr\_lbm" program
with no loss, meaning that the socket calls are made.
The times include both the UM code and the socket code.
"lbm\_sock.sh" is written to use the kernel socket driver.

Finally, the "lbm\_onload.sh" script runs the same "jtr\_lbm" program
with no loss, using Onload.
The times include both the UM code and the Onload code.

To run all of the "tst\_*.sh" test scripts, use "tstall.sh".

### Plot Scripts

Most of the plots are intended to compare two differnt tests.
Hence the naming convention using "_v_" (meaning "versus").
For example, "plot_onload_v_lbmonload.sh" takes the results from
"tst_onload.sh" and "tst_lbmonload.sh" and combines on one graph.
This demonstrates the impact of calling UM compared to calling Onload
sockets directly.

Just as each test script runs its test three times,
each plot script produces three ".png" files.
For example, "plot_onload_v_lbmonload.sh" produces:
"plot_onload_v_lbmonload-1.png", "plot_onload_v_lbmonload-2.png", and
"plot_onload_v_lbmonload-3.png".

* plot_clock.sh
* plot_lbmnosock500_v_null.sh
* plot_lbmnosock_v_clock.sh
* plot_lbmnosock_v_null.sh
* plot_lbmnosockclock_v_null.sh
* plot_lbmnosockdeftwb_v_lbmnosock.sh
* plot_null.sh
* plot_onload_v_lbmonload.sh
* plot_onload_v_null.sh
* plot_sock_v_lbmsock.sh
* plot_sock_v_null.sh
* plot_sock_v_onload.sh

To run all of the "plot_*.sh" plot scripts, use "plotall.sh".

## Implementation Notes

### Onload

The test scripts named "tst_*onload.sh" use the "onload" command to
test with the Solarflare kernel-bypass library.
Note that those scripts assume a very simple Onload configuration.
It is highly probable that a more-sophisticated configuration would
produce better Onload numbers.

#### Test Scripts

The gnuplot graphs are produced using a set of "plot_*.sh" scripts.
Most of the "plot_*.sh" scripts access histogram results from two
different test scripts.
For example, "plot\_onload\_v\_lbmonload.sh" reads the results from the
"tst\_onload.sh" and "tst\_lbmonload.sh" and produces the graphs comparing
Onload socket calls to UM send calls using Onload.

#### The Easy Way

The easy way to run all "tst_*.sh" test scripts:

    ./tstall.sh


The easy way to run all "plot_*.sh" gnuplot scripts:

    ./plotall.sh


To see all the individual test results with percentiles, see tst_*.txt

To see all the graphs, see plot_*.png


### Error Handling

The test programs use a crude form of error handling in the form of
two macros: LBME() and SYSE().
Both of them are designed to print some useful error information and then
trigger a core dump by calling abort().
The core dump is intended to make it easier to debug the issue in the
event of a bug in the test code.

To see it in action, you can try to pin the thread to an illegal CPU:

    ./tst_null.sh -c 999


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

## Root Usage

It is possible to run the test tools with high real-time (FIFO) priority,
for example with priority 99 (the highest).
This prevents your thread from being time-shared with other user threads,
and even some kernel threads.
(Note: I do not use real-time scheduling in my example outputs.)

A common way to do this is with the "chrt" command:

    sudo chrt -f 99 ./tst_null.sh


These test tools were also written to do the same thing programmatically:

    sudo ./tst_null.sh -f 99


Finally, it is possible to do it without "sudo",
by changing ownership of the executable files to root and
setting the "set uid" bit:

    sudo chown 0:0 jtr_null jtr_sock jtr_lbm
    sudo chmod u+s jtr_null jtr_sock jtr_lbm

Now when you run one of those programs,
Unix sets the UID to root automatically, without the use of sudo:

    ./tst_null.sh -f 99


But to run the jtr\_lbm as root will require installing UM itself
as a system package.
For example, assuming your 6.12.1 package is in /tmp:

    sudo cd /usr/local; /tmp/UMP_6.12.1_Linux-glibc-2.17-x86_64.sh
    sudo echo "/usr/local/UMP_6.12.1/Linux-glibc-2.17-x86_64/lib" >/etc/ld.so.conf.d/lbm.conf
    sudo ldconfig


Finally, the tools create a gnuplot data file.
If the tool is run as root, that file will end up being owned by root.
