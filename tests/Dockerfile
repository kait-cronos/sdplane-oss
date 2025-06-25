FROM ubuntu:24.04

ENV DPDK_VERSION=23.11.1
USER root

WORKDIR /tmp

RUN apt-get update && apt-get install -y \
    git \
    wget \
    build-essential \
    clang-format \
    autoconf \
    cmake \
    python3 \
    python3-pip \
    meson \
    ninja-build \
    python3-pyelftools \
    libnuma-dev \
    pkgconf \
    liburcu-dev \
    libpcap-dev

RUN git clone https://github.com/yasuhironet/lthread.git && \
    cd lthread && \
    cmake . && \
    make && \
    make install

RUN wget https://fast.dpdk.org/rel/dpdk-${DPDK_VERSION}.tar.xz && \
    tar -xvJf dpdk-${DPDK_VERSION}.tar.xz && \
    cd dpdk-stable-${DPDK_VERSION} && \
    meson setup build && \
    cd build && \
    ninja && \
    meson install && \
    ldconfig

RUN wget https://www.yasuhironet.net/download/partaker/libsdplane_0.3.36-52_amd64.deb && \
    apt-get install -y ./libsdplane_0.3.36-52_amd64.deb

RUN apt-get install -y \
    sudo

COPY . /sdplane-dev

WORKDIR /sdplane-dev/tests

CMD ["make", "test"]
