# COoL-TEE: Resilient Decentralized Search against Front-Running Attacks

This repository contains the code and scripts used to simulate and analyse the search and market systems considered in the COoL-TEE paper.

Hereafter are the requirements and step-by-step guide to replicate results presented in the paper.

## Dependencies & System Requirements
- *PlanetLab dataset*: the used network latency dataset can be cloned from the [NetLatency-Data repository](https://github.com/uofa-rzhu3/NetLatency-Data);
- *Omnet++*: version 6.0.1 for Windows, available for download [here](https://omnetpp.org/download/old), was used to run experiments. It is also possible to use the version for other distributions, but provided automation scripts assume Windows as the operating system. See also the [Omnet++ installation guide](https://doc.omnetpp.org/omnetpp6/InstallGuide.pdf);
- *WSL2* (Windows Subsystems for Linux): Omnet++ requires WSL to be set up. You can follow the [Omnet++ installation guide](https://doc.omnetpp.org/omnetpp6/InstallGuide.pdf) to set up WSL;
- *Python3*: required modules are provided in *[requirements.txt](./requirements.txt)* (installable using: `pip install -r requirements.txt`);
- *Jupyter* (optional): Jupyter notebooks are available for figure generation, but they are also available in Python script form.
- *Disk space*: Around 400GB of storage space are sufficient to hold all experiment data, across all presented system configurations. Around 2.2GB of space are taken for single configurations with 500 runs, 1.3GB for those with 100 runs, should system contraints require running experiments in smaller batches;

## Experiment replication

Experiment data generation is divided in two steps:

- the search mechanism simulation (left side of fig.4 in the paper), simulated with Omnet++;
- whose results are then joined with market events simulation (right side of fig. 4), simulated in Python.

### I. Search mechanism simulation

Omnet++ experiments can be deterministically replicated given the provided [omnetpp.ini](sim/search/simulations/omnetpp.ini) configuration file.

For single experiments, the Omnet++ IDE can be used to visualize the system's behaviour.

For batch experiments, e.g., for replication purposes, the [cmd.sh](sim/search/simulations/cmd.sh) script can be used (in a PowerShell terminal).

#### A. Configuration

The output folder for experiment data should be set to a valid location in [omnetpp.ini](sim/search/simulations/omnetpp.ini), using the `outputPathPrefix` variable.
The batch automation script [cmd.sh](sim/search/simulations/cmd.sh)'s variable *LOG_DIR* should be modified in line 7 to point to a valid folder where Omnet++ logs will be stored.

Then, [cmd.sh](sim/search/simulations/cmd.sh) can be run using the following format:
```sh
./cmd.sh <run-start-num> <run-end-num> <batch-size> <exp-name> {TwoSpeedService|TwoSpeedServiceNoLat}
```
- to run experiments with run-numbers in `[<run-start-num>, <run-end-num>[`, 
- in batches of `<batch-size>` runs executed in parallel, 
- and whose logfile names will be prefixed by `<exp-name>`. For linking purposes with the next step, `<exp-name>` should be either *wait500ByzRho100* or *wait100noLatByzRho100*.
- The last parameter should be "TwoSpeedService" if `<exp-name>=wait500ByzRho100`, for PlanetLab-based experiments, or "TwoSpeedServiceNoLat" if `<exp-name>=wait100noLatByzRho100`, for same-datacenter experiments.

Running [cmd.sh](sim/search/simulations/cmd.sh) will output a file named `<exp-name>-runs.log` at `LOG_DIR`, containing available run numbers for experiments. This file can be generated without running experiments, for either "TwoSpeedService" or "TwoSpeedServiceNoLat" configurations, by setting `<run-start-num>` and `<run-end-num>` to 0.

*NB: The two configuration types "TwoSpeedService" and "TwoSpeedServiceNoLat" should not be run in parallel, as result filenames will conflict.*

#### B. Obtained results

Results will be available at `<outputPathPrefix>/all`. Each run will have its file in the format `all-<run-number>-*.csv`.

For a simpler next simulation step, i.e., following the automated name resolution, these files should be placed in a folder named `<exp-name>`.

### II. Joint search+market simulation

For full-replication purposes, the datasets of market events are provided in the [data](data) folder.

The Python script [race_winners.py](sim/market/race_winners.py) takes the search mechanism's results and merges them with the market event logic. 

#### A. High-Level Configuration

`INPUT_DIR` and `OUTPUT_DIR` variables should be modified in [race_winners.py](sim/market/race_winners.py) (lines 541-542), to reflect the location of the `all` folder from the previous step, i.e., `<outputPathPrefix>`, and where the new joint results should be stored.

To use the provided market events for full-replication of the paper's results, `OUTPUT_DIR` should point to the `data` folder in this repository.

#### B. Experiment-level Configuration

To generate the PlanetLab-based results, the [race_winners.py](sim/market/race_winners.py) can be run as-is for TEE-based configurations. To run the special case where no TEEs are used, line 559 should be uncommented, i.e., the list item `"noTEE"` in the list `EXP_SPECS` (see code block below).

To generate the same-datacenter results, because the result folders and configuration files are different, `EXP_DIR` and `CONFIG_FILENAME` (lines 554-556) should be set to "wait100noLatByzRho100" and "config_8SP_wait100noLatByzRho100" respectively (available in commented form at the relevant lines). 
Additionally, for name-resolution purposes in the analysis phase, the list item `"sameDC"` in `EXP_SPECS` should be uncommented (line 561).
Similarly to before, the configurations without TEEs can be run by uncommenting the `"noTEE"` list item.

```python  
    EXP_SPECS=[
        #"noTEE",
        "WtA",#Winner takes all (assets)
        #"sameDC",
        f"+{TIME_OFFSET}s"
    ]
```

#### C. Obtained results

`OUTPUT_DIR` will contain results for each sub-configuration, e.g., with different numbers of malicious providers. 

### III. Figure regeneration

The [analysis](analysis) folder contains 3 Python scripts, and their Jupyter notebooks equivalents, to regenerate figures presented in the paper.

#### A. Configuration

Each notebook should be modified at node 2, or equivalently each Python script at lines 7-8, to specify valid paths to:

- where result data is located, in `OUTPUT_DIR`;
- where figures should be stored once generated, in `FIG_DIR`.

#### B. Running experiments

Once configured the Python scripts or notebooks can be run to generate the corresponding figures, which will be available in `FIG_DIR`.