# COoL-TEE-deployed

This subrepository contains the code and scripts used to run and analyse the provider selection and search systems considered in the COoL-TEE paper.

Hereafter are the requirements and step-by-step guide to replicate results presented in the paper.

## Dependencies & System Requirements
- [Ubuntu 20.04](https://releases.ubuntu.com/20.04.6/ubuntu-20.04.6-live-server-amd64.iso): is required through the dependency to DeSearch below. Using Ubuntu 22.04 or above will not work.
- [Intel Linux SGX SDK](https://github.com/intel/linux-sgx): to run the search mechanism is any SGX mode (Simulation or Hardware), the Intel SGX SDK for Linux must be installed. The SDK and the procedure for Ubuntu 20.04 are available on [Github](https://github.com/intel/linux-sgx). Gold Release 2.24 was used for the paper (`git checkout sgx_2.24` once cloned).
- Intel Linux SGX PSW: to run the search mechanism is Hardware mode (using SGX hardware), the Intel SGX PSW for Linux must be installed. The PSW installation procedure for Ubuntu 20.04 is provided by [Intel's Software Installation Guide](https://download.01.org/intel-sgx/latest/linux-latest/docs/Intel_SGX_SW_Installation_Guide_for_Linux.pdf#%5B%7B%22num%22%3A16%2C%22gen%22%3A0%7D%2C%7B%22name%22%3A%22XYZ%22%7D%2C69%2C179%2C0%5D).
- An SGX-enabled machine is required to run the search mechanism in Hardware mode. Following DeSearch, an SGX1 machine was used in the paper. SGX2 ("Scalable SGX") machines should also work.
- [DeSearch](https://github.com/sjtu-ipads/desearch) ([Li et al. 2022](https://www.usenix.org/conference/osdi21/presentation/li)): the underlying search mechanism used in the implemented protocol is DeSearch. If the [repository](https://github.com/sjtu-ipads/desearch) is cloned at the same level as this repository (i.e., the root is the deployed folder), then it is possible to use the provided [Makefile](Makefile) to compile both DeSearch and the COoL-TEE client.
- *Python3*: required modules are provided in *[requirements.txt](./requirements.txt)* (installable using: `pip install -r requirements.txt`);

## Experiment replication

Deployed experiments proceed in two steps:
- the clients (consumers) and servers (providers) are run, and latency traces are logged;
- these logs are aggregated into figures.

### I. Search mechanism deployment

The [client](client) folder and the [DeSearch repository](https://github.com/sjtu-ipads/desearch) enable together to run the deployed experiment. A [test-server](test-server) (with simple ping-pong logic) is also provided as a placeholder to DeSearch's server ("querier"), to ascertain that there is no bottleneck between the client and the server.

If an SGX-enabled machine is available, servers should run on that machine. 
The client does not require an SGX machine.

#### A. Provider/Server-side

DeSearch "queriers" (our "providers") can be compiled using `make SGX_MODE={SIM|HW}` in the [sgx-executor](https://github.com/SJTU-IPADS/DeSearch/tree/main/sgx-executor) folder, respectively to compile the executable in SGX Simulation or Hardware modes. Furthermore, prior to compilation, one can toggle the `USE_ORAM` definition in `sgx-executor/Enclave2/oram_enclave_querier.cpp` to (de)activate the usage of Circuit-ORAM. If one deactivates ORAM, lines 152 to 154 in that file should either be commented or better yet surrounded by:
```cpp
#ifdef USE_ORAM
    g_offset.clear();
    g_length.clear();
    g_VLB_buffer.clear();
#endif
```

Before running queriers, the "kanban" Redis server must first be launched using `./src/redis-server ./redis.conf` in DeSearch's `kanban` folder.
To make kanban use a non-empty index, i.e., so queriers serve a non-empty index, one can place the provided [dump.rdb](dump.rdb) file in the `kanban` folder.
The alternative is to deploy the complete DeSearch pipeline to populate the index.
Please follow the DeSearch [Readme.md](https://github.com/SJTU-IPADS/DeSearch/blob/main/README.md) in that case.

Then, the queriers can be launched with the following command format in the `sgx-executor` folder: `./executor querier <IP> <port>`, e.g., `(./executor querier 0.0.0.0 12345 &) && (./executor querier 0.0.0.0 12346 &) && (./executor querier 0.0.0.0 12347 &) && (./executor querier 0.0.0.0 12348 &)` to launch 4 providers, as is done for Figure 5.

#### A'. Test server

The test server can be compiled using the provided [Makefile](test-server/Makefile).
The executable will be generated in the [out](out) folder.

To run test-servers, their IP/ports to which they will listen should be written in the [network.json](out/network.json) file.
The batch of configured servers in the [network.json](out/network.json) can be run using the [run_servers.sh](out/run_servers.sh) script.

#### B. Consumer/Client-side

The client can be compiled using the provided [Makefile](test-server/Makefile).
Two `make` targets (`make [{cool|rdm}]`) are available to either use COoL-selection or DeSearch-like/random selection. The default target is COoL-selection.
The executable will be generated in the [out](out) folder.

To run clients as in the latency-throughput experiment, one can use the [run_clients_scale.sh](out/run_clients_scale.sh) script using `./run_clients_scale.sh <nb-clients-per-batch> <period-b/w-batches> <k-reqs> <kErr> <kD>`, with:
- `nb-clients-per-batch` the number of client processes to spawn each `period-b/w-batches` seconds;
- `period-b/w-batches` the time between the launch of two batches of client processes;
- `k-reqs` the number of requests sent by each client/consumer process for each unique request content (i.e., the "k" parameter in multi-provider selection in the paper);
- `kErr` the proportional parameter of COoL-selection's PD-controller (e.g., 0.01);
- `kD` the derivatie parameter of COoL-selection's PD-controller (e.g., 0.1).

For example: `./run_clients_scale.sh 10 30 1 0.01 0.1` launches 10 new client processes each 30s. Each client sends only one request replica per unique request content, and COoL-selection PD-controller parameters are (0.01,0.1).

For DeSearch-like selection, `kErr` and `kD` are not used by the code (but a value should still be provided).

Clients send requests to the server end-points configured in [network.json](out/network.json).
The initial proportion of requests sent to each server is defined by the `weight` value (all weights should sum to 1).
They write their individual request latencies in `out/results/latency/<client-ID>.log`.
Error outputs (e.g., failed connections) are directed to the parent terminal.

### Latency-throughput analysis

Figure 5 in the paper can be generated using the [fig5-scaling_lat_thr.py](analysis/fig5-scaling_lat_thr.py) Python script.

To reproduce the figure, one should run all combinations of the client (in both `make {cool|rdm}`) and DeSearch queriers security configurations (`make SGX_MODE={SIM|HW}` with `USE_ORAM` undefined, and `make SGX_MODE=HW` with `USE_ORAM` defined). Each combination should be placed in a separate folder (e.g., `out/results_<CONFIG-NAME>`). All 6 combinations' folder paths can then be written into [fig5-scaling_lat_thr.py](analysis/fig5-scaling_lat_thr.py)'s `RES_DIR_LIST` (line 26).

Running `python fig5-scaling_lat_thr.py` then outputs a `mean_latency_vs_total_throughput_<timestamp>.pdf` in the [out](out) folder.

### COoL latency minimization convergence

One can set skewed initial weight values in a same-datacenter experiment setup (or use a latency-heterogeneous network setup) to assess the convergence of COoL-selection towards a minimization of the average latency. The deployment procedure is the same as before, and the [scaling_provider.py](analysis/scaling_provider.py) script (`python scaling_provider.py`) can used to generate a figure showing the evolution of latency and throughput per provider over time. The figure is generated in the [out](out) folder.