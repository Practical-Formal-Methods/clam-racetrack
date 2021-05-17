# Neuro-Aware Program Analysis

This repository contains the code for Proof-of-Concept implementaion of our approach to verifying heterogenous software systems using a tightly bound verification process leveraging Abstract Interpretation. We call this approach Neuro-Aware Program Analysis and use it to verify reachability and crash avoidance for variants of the [Racetrack Benchmark](https://github.com/Practical-Formal-Methods/Racetrack-Benchmark). This was part of our submission to CAV'21 on Automated Safety Verification of Programs Invoking Neural Networks.

This code is provided _as-is_ without detailed build instructions. For obtaining a working build and reproducing results presented in the paper we have provided docker images [here](https://hub.docker.com/r/practicalformalmethods/neuro-aware-verification). Once you obtain an image, you can read through the README at `/root/README.md` for detailed instructions on reproducing experiment results.

#### Steps to get image

The images are tagged by `<map>` and `<nnAnalyzer>`. A tag is therefore `<map>_<nnAnalyzer>` where:

```
<map> = barto-small, barto-big, ring
<nnAnalyzer> = deepsymbol, eran
```

To pull an image, run:

```
docker pull practicalformalmethods/neuro-aware-verification:<map>_<nnAnalyzer>
```

To run the image:

```
docker run -it practicalformalmethods/neuro-aware-verification:<tagname> /bin/bash
```

The primary backbone for this project is the [CLAM](https://github.com/seahorn/clam) static analyzer that computes inductive invariants for LLVM-based languages using the [CRAB](https://github.com/seahorn/crab) Abstract Interpreter. Folders `barto_small` , `barto_big` and `ring` contain code specific to each map variant along with all neural-network models (or agents), code for the Neural Network analyzers, interface layer and scripts to run.

## Steps to build from source using clam

### Clam: Crab for LLvm Abstraction Manager

<img src="https://upload.wikimedia.org/wikipedia/en/4/4c/LLVM_Logo.svg" alt="llvm logo" width=280 height=200 /><img src="http://i.imgur.com/IDKhq5h.png" alt="crab logo" width=280 height=200 /> 

Clam is a static analyzer that computes inductive invariants for LLVM-based languages based on
the [Crab](https://github.com/seahorn/crab) library. The version provided under this project uses LLVM `5.0`.

### Requirements

Clam is written in C++ and uses heavily the Boost library. The
main requirements are:

- Modern C++ compiler supporting c++11
- Boost >= 1.62
- GMP 
- MPFR (if `-DCRAB_USE_APRON=ON` or `-DCRAB_USE_ELINA=ON`)

In linux, you can install requirements typing the commands:

     sudo apt-get install libboost-all-dev libboost-program-options-dev
     sudo apt-get install libgmp-dev
     sudo apt-get install libmpfr-dev	

To run tests you need to install `lit` and `OutputCheck`. In Linux:

     apt-get install python-pip
     pip install lit
     pip install OutputCheck

### Installation from source 

#### Note: We provide only a rough outline of the steps needed to build this project from source. It is highly recommeded that you use the [images](https://hub.docker.com/r/practicalformalmethods/neuro-aware-verification) we provide.

Clam provides several components that are installed via the `extra`
target. These components are already provided here. 

For using the Deepsymbol verifier, we use the `Apron` numerical analysis library, while for ERAN we use `elina`. We make use of the `Boxes` domain for this project. 

- If you want to use the Apron library domains then add
  `-DCRAB_USE_APRON=ON` option.

- If you want to use the Elina library domains then add
  `-DCRAB_USE_ELINA=ON` option.

**Important:** Apron and Elina are currently not compatible so you
cannot enable `-DCRAB_USE_APRON=ON` and `-DCRAB_USE_ELINA=ON` at the same time. 

For instance, to install Clam with Boxes and Apron:

     mkdir build && cd build
     cmake -DCMAKE_INSTALL_PREFIX=_DIR_ -DCRAB_USE_LDD=ON -DCRAB_USE_APRON=ON ../
     cmake --build . --target ldd && cmake ..
     cmake --build . --target apron && cmake ..
     cmake --build . --target llvm && cmake ..                
     cmake --build . --target install 

### Checking installation

To run some regression tests for clam:

     cmake --build . --target test-simple

**Important:** the first thing that clam does is to compile
  the C program into LLVM bitcode by using Clang. Since Clam is
  based on LLVM 5.0, the version of clang must be 5.0 as well. 

## LICENSE

This code is released under the [CRAPL](http://matt.might.net/articles/crapl/) license. Code from included dependencies were released under the [Apache License](https://www.apache.org/licenses/LICENSE-2.0), a copy of which is also included.

Running [ERAN](https://github.com/eth-sri/eran) with Gurobi requires to obtain an academic license for gurobi from https://www.gurobi.com/academia/academic-program-and-licenses/
