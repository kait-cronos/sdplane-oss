# Install Dependencies

**Language:** **English** | [日本語](../ja/install-dependencies.md)

## Dependencies
- **liburcu-qsbr**: Userspace RCU library
- **libpcap**: Packet capture library
- **lthread**: [yasuhironet/lthread](https://github.com/yasuhironet/lthread) (lightweight cooperative threading)
- **DPDK**: Data Plane Development Kit

## Install sdplane dependencies debian packages
```bash
sudo apt install liburcu-dev libpcap-dev
```

## Install Build Tools and DPDK Prerequisites

```bash
# Core build tools
sudo apt install build-essential cmake autotools-dev autoconf automake libtool pkg-config

# DPDK prerequisites
sudo apt install python3 python3-pip meson ninja-build python3-pyelftools libnuma-dev pkgconf
```

## Install lthread
```bash
# Install lthread
git clone https://github.com/yasuhironet/lthread
cd lthread
cmake .
make
sudo make install
```
## Install DPDK 23.11.1
```bash
# Download and extract DPDK
wget https://fast.dpdk.org/rel/dpdk-23.11.1.tar.xz
tar vxJf dpdk-23.11.1.tar.xz
cd dpdk-stable-23.11.1

# Build and install DPDK
meson setup build
cd build
ninja
sudo meson install
sudo ldconfig

# Verify installation
pkg-config --modversion libdpdk
# Should output: 23.11.1
```