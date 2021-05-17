#
# Dockerfile for Clam binary
# produces package in /clam/build
# Arguments:
#  - UBUNTU:     trusty, xenial, bionic
#  - BUILD_TYPE: debug, release
#  - BRANCH

ARG UBUNTU

# Pull base image.
FROM seahorn/seahorn-build-llvm5:$UBUNTU

# Needed to run clang with -m32
RUN apt-get update && \
    apt-get install -yqq libc6-dev-i386 && \
    apt-get install -yqq libboost-all-dev

RUN cd / && git clone https://github.com/antoinemine/apron.git && mkdir -p /apron/install
WORKDIR /apron
RUN ./configure -prefix /apron/install -no-ppl && \ 
    make && \ 
    make install

ARG BRANCH
RUN cd / && rm -rf /clam && \
    git clone https://github.com/yugeshk/crab-llvm -b $BRANCH clam --depth=10 ; \
    mkdir -p /clam/build
WORKDIR /clam/build

ARG BUILD_TYPE
# Build configuration.
RUN cmake -GNinja \
          -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
          -DBOOST_ROOT=/deps/boost \
          -DLLVM_DIR=/deps/LLVM-5.0.2-Linux/lib/cmake/llvm \
          -DCMAKE_INSTALL_PREFIX=run \
          -DCMAKE_CXX_COMPILER=g++-5 \
          -DCMAKE_EXPORT_COMPILE_COMMANDS=1 \
          -DCRAB_USE_LDD=ON \
          -DCRAB_USE_ELINA=ON \
          ../ && \
    cmake --build . --target extra  && cmake .. && \
    cmake --build . --target crab  && cmake .. && \
    cmake --build . --target ldd  && cmake .. && \
    cmake --build . --target elina  && cmake .. && \
    cmake --build . --target install

# symlink clang (from base image)
RUN ln -s /clang-5.0/bin/clang run/bin/clang
RUN ln -s /clang-5.0/bin/clang++ run/bin/clang++

ENV PATH "/deps/LLVM-5.0.2-Linux/bin:$PATH"
ENV PATH "/clam/build/run/bin:$PATH"

#run dynamic linked for shared objects that were possibly not found
RUN ldconfig -v /clam/build/run/lib /clam/build/run/elina/lib /apron/install/lib 

# run tests
RUN cmake --build . --target test-simple
RUN cmake --build . --target test-readme
RUN cmake --build . --target test-ssh-simplified
RUN cmake --build . --target test-ntdrivers-simplified



WORKDIR /clam