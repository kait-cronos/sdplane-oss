ARG UBUNTU_VERSION=24.04
FROM ubuntu:${UBUNTU_VERSION}

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
      build-essential debhelper dh-autoreconf autoconf automake libtool \
      pkg-config git cmake ca-certificates \
      dpdk-dev libpcap-dev liburcu-dev

# lthread is linked statically, so it is needed only at build time.
ARG LTHREAD_REF=04ce15a
RUN git clone https://github.com/yasuhironet/lthread.git /tmp/lthread \
 && cd /tmp/lthread && git checkout ${LTHREAD_REF} \
 && cmake -DCMAKE_POLICY_VERSION_MINIMUM=3.5 . \
 && make lthread && cmake --install . \
 && rm -rf /tmp/lthread

RUN git config --system --add safe.directory '*'
