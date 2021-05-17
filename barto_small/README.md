# Neuro-Aware Program Analysis

*Note:* This is the README present in the [barto-small_deepsymbol](https://hub.docker.com/layers/practicalformalmethods/neuro-aware-verification/barto-small_deepsymbol/images/sha256-fc5adb8bc279bdc846aac6cbd01ea96dfcf4b5e93520d89eab43bec0b1577687?context=repo) image at `/root/README.md`. Information contained in this is relative to that image. This container has BartoSmall:OD=3:GoodAgent:NormalSize:Deepsymbol configuration from the paper.

*Instructions to run at the end of the file*

## Overview

This artifact contains relevant code and libraries to reproduce results part of our CAV Submission on an approach for using Abstract Interpretation for static analysis of Heterogeneous software systems. This code is a Proof-of-Concept to demonstrate how differet analyses can be used in a tight two-way integration. 

This project builds on a lot of publically available code, and some
independently developed. The code consists braodly of the following pieces (some
of them have been modified for our use case):
 
1. [Crab](https://github.com/seahorn/crab) : A language agnostic library for static analysis using Abstract
   Interpretation. Crab does not operate on source-code rather, a CFG based
   lanaguage which is independent of the source lamguage.

2. [Clam](https://github.com/seahorn/crab-llvm) : A frontend for crab for static analysis on LLVM-based languages 

3. [Apron](https://github.com/antoinemine/apron)/[ELINA](https://github.com/eth-sri/elina) - Abstraction Libraries for Numerical Analysis for Abstract
   Domains

4. Deepsymbol/[ERAN](https://github.com/eth-sri/eran) - (Local) Robustness Analyzers for Neural Networks

5. Middleware - A thin layer of interface code to allow Crab to interact with
   Deepsymbol/ERAN for holistic hybrid analysis

6. RACETRACK - An independently developed benchmark from the AI community.

## Code Anatomy

The following is an (incomplete) list of relevant locations in the source tree:

1. /clam/crab/include/crab/analysis/abs\_tranformer.hpp : Contains abstractions
   within the Abstract Interpretation library to allow hybrid analysis
   - line 806-1161: Deepsymbol
   - line 1841-2101: Array abstraction

2. /deepsymbol/middleware.cpp : Contains the interface layer code that allows crab to interact
   with the external NN analysis tool

3. /clam/barto_small: modified code for the benchmark to facilitate Abstraction
   Interpretation along with all run scripts to run and collect stats.

4. /agents<X> : Contains the NN model in the format required by analysis tools.
    - X=N : NORMAL size agents (2 Hidden Layers x 64 Neurons)
    - X=B : BIG size agents (4 Hidden Layers x 64 Neurons)

5. /agents : Contains archives of all agents

## Steps to Run experiments

### Setting up the configuration

This container defaults the configuration BartoSmall:OD=3:GoodAgent:NormalSize:Deepsymbol

Additional tuning on this configuration includes :

1. Changing the Controller Noise (as described in the paper)
    - /clam/crab/include/crab/analysis/abs\_transformer.hpp Line 1071: Turn the
      Controller Noise On/Off.
    - /clam/crab/include/crab/analysis/abs\_transformer.hpp Line 1072: changing
      the `OD` variable changes the level of Controller Noise (we used 1,2,3)

2. Changing the NN Noise (as described in the paper)
    - /deepsymbol/middleware.cpp Line 8 : Turn the NN Noise On/Off.
    - /deepsymbol/middleware.cpp Line 43 : Change the noise level by turning
      the `epsilon` parameter (for our experiments this was fixed at 0.25)

3. Changing the agent (source location mentioned above)
    - /deepsymbol/middleware.cpp Line 9 : Specify path to the agent

### Running the Analysis

To run the analysis you must do the following: 

1. Configure the Noise as described above.

2. Apply the changes:
In case of Controller Noise, after making any changes do the following:
```
cd /clam/build
cmake --build . --target install
```
This rebuilds the crab binary with the updated configuration

In case of NN Noise or changing the agent, after making changes do the following:
```
cd /deepsymbol
make
```
This rebuilds the middleware binary with the updated configuration

3. Run the experiment: Once the configuration is setup (as per the paper), to
   reproduce an experiment (with no look\_ahead), goto /clam/barto_small/scripts.
   This folder contains a script `run_barto_small.sh`. This script runs the
   verification for all start states parallely on all cores. To run the script:
```
cd /clam/barto_small/scripts
bash run_barto_small.sh
```
In case of Controller Noise, the script expects a parameter `noise`. Therefore
if Controller Noise is turned on, run as follows:
```
bash run_barto_small.sh noise
```
The script will complain if this is not provided and CN is turned on. The script produces the following files:
   - /clam/barto_small/results/results.csv : Contains a csv view of the map with final result per start state
   (i.e., 0,1,2 marking).
   - /clam/barto_small/results/stats.log : Contains some run stats for the experiment
   like total time, avg time and time spent in NN analysis

We have provided all the results in the paper in the folder <paperexp>.
Each sub-folder in this folder contains a file `tags` that describes the
configuration of the particular experiment and the `results.csv` for that
experiment in the paper. To verify reproducibility you can compare this file to
the one you get from running the experiment.
