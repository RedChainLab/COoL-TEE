# COoL-Triad

This repository contains the integration of COoL-TEE with Fernandez et al.'s Triad.

## Requirements

Triad relies on AEX-Notify, available with Linux kernel 6.2+, and is not available in SGX simulation mode.
Availability of AEX-Notify on SGX-enabled hardware can be verified using the [SGX-Hardware](https://github.com/ayeks/SGX-hardware) test executable.
One core per provider should be dedicated to read the TSC, while minimizing interruptions. 
This can be done using boot option `isolcpus`.

For analyses, required Python modules are listed in the [requirements](requirements) file.

## Configuration

There are 3 main available build configuration:
- without time source (`NO_TIME`): the TEE does not measure queuing delays at all;
- with OS time source (`UNTRUSTED_TIME`): the TEE performs an ocall to obtain an untrusted timestamp from the OS and measures queuing delays;
- with a Trusted Time source (`TRUSTED_TIME`): the TEE uses the Triad protocol to maintain a local trusted clock and measures queuing delays.

To switch between configurations, one must give the corresponding enum value to the Provider's `use_time` member in the constructor ([EProv.cpp, line 59](node/Enclave/EProv.cpp#L59)), e.g.:
``` C++
    use_time(TRUSTED_TIME)
```
Additionally, to fully deactivate the Triad stack, [lines 240-243 in Node.cpp](node/App/Node.cpp#L240-243) should be commented.

A compile-time definition of the time source configuration will be added in the future.

## Building

After choosing the time source, compilation is done using `make` at repository root.
In the context of the accompanying paper submission, the client is here compiled by default using COoL-TEE's provider selection.
To explicitly compile it with either COoL or DeSearch-like selection, respectively use `make cool` or `make rdm` in the [client](client) directory. 

## Execution

### Triad Time Authority
If `TRUSTED_TIME` is used, then Triad is used and therefore one must run the built `server` executable (no arguments needed) in the [time_authority](time_authority) directory. This server is the Triad time authority, who listens on port 12340 for incoming timestamp requests.

### COoL-TEE + Triad integration 
The proposed execution scenario in [main.cpp](user/main.cpp) is to run 3 providers. Providers i (i.e., 0, 1, and 2) will communicate with each other using ports `<triad-start-port>+i` as part of the Triad protocol, and will each listen on `<cool-start-port>+i` for incoming consumer requests.
Moreover, a core is dedicated to read the TSC, specified in `<rdTSC-core>`.
The integrated COoL-TEE + Triad stack can be run using:
``` sh
    ./triad <triad-start-port> <rdTSC-core> <cool-start-port>
```
e.g.:
``` sh
    ./triad 12345 2 12445
```

[sim_interrupts.sh](analysis/sim_interrupts.sh) can be used to simulate an interruption distribution on a core, in a low-interruption environment:
``` sh
    analysis/sim_interrupts.sh <core-id> {<probability*10000>-<sleep-time-sec>}...
```
e.g., to reproduce the interruption distribution in the original Triad paper (fig.7):
``` sh
    analysis/sim_interrupts.sh 2 3334-0.01 3333-0.53 3333-1.5895 
```

### COoL-TEE consumer

Consumers use the [network.json](out/network.json) file to know which providers should serve them. The IP and port must be specified per-provider there, as well as an initial selection probability, e.g., 1 over the number of providers. 

The easiest way to run clients for benchmarks is to use available scripts in [out](out), e.g., to reproduce latency-throughput figures.

- [run_clients_scale.sh](out/run_clients_scale.sh) launches `<nb-clients>` client processes every `<period>` seconds. These clients send each of their requests to `<k>` providers, use PD-controller parameters `<Kp>` and `<Kd>`.
  ``` sh
    ./run_clients_scale.sh <nb-clients> <period> <k> <Kp> <Kd>
  ```
- [run_clients.sh](out/run_clients_scale.sh) is used by the previous script and launches clients with IDs `<ID-start>` to `<ID-end>` with the previous client parameters, but also how often selection ratios will be updated (`<update-ratios>`) and the window size for averaging latencies `<window-size>`. This script also assigns a specific subset of cores to run clients, so as to separate physical resources between providers and consumers, using `taskset -p <core-set> <command>` (line 9).
  ``` sh
    ./run_clients.sh <nb-clients> <period> <k> <Kp> <Kd> <update-ratios> <window-size>
  ```

To run a single client, one can use the previous arguments used with [run_clients.sh](out/run_clients_scale.sh) with the [client](out/client) executable obtained after compilation.

Clients create latency log files in [out/results/latency/*.log](out/results/latency).
If multiple runs are done, the [latency](out/results/latency) directory from a previous run must renamed beforehand, or data will be overwritten.

## Analyses

Directory [analysis](analysis) constains shell and Python scripts to pre-process and generate figures:

- [scaling.py](analysis/scaling.py) is a monitoring script to use mainly during runs of the COoL-TEE system. It shows the 30s-window throughput and latency perceived by the clients (e.g., to know when to stop the run when the peak throughput is reached).
- [scaling_provider.py](analysis/scaling_provider.py) provides a fine-grained version of [scaling.py](analysis/scaling.py), showing per-provider throughput and latencies.
- [fig5-scaling_lat_thr.py](analysis/fig5-scaling_lat_thr.py) generates the paper's Figure 5, i.e., the latency-throughput of the system. It is fed with latency log-files from clients, stored in [out/results/latency-*/*.log](out/results), as specified in `RES_DIR_LIST`.
- [scaling_lat_thr_single.py](analysis/scaling_lat_thr_single.py) generates the paper's Figure 5, i.e., the latency-throughput of the system, but for the current run in [out/results/latency/*.log](out/results).
- [extract_lats.sh](analysis/extract_lats.sh) retrieves pipeline sub-step delays from the output of Providers (stored in a file `<log-file>`) and stores them in csv format in a `<csv-file>`: `analysis/extract_lats.sh <log-file> <csv-file>` 
- CSV files from [extract_lats.sh](analysis/extract_lats.sh) can fed to [lat_bars.py](analysis/lat_bars.py) to plot bar charts of those pipeline sub-step delays. Input files must be specified inside `RES_DIR_LIST` at line 26.