# Build and Install from Source

**Language:** **English** | [Japanese](../ja/build-install-source.md) | [Français](../fr/build-install-source.md) | [中文](../zh/build-install-source.md) | [Deutsch](../de/build-install-source.md) | [Italiano](../it/build-install-source.md) | [한국어](../ko/build-install-source.md) | [ไทย](../th/build-install-source.md) | [Español](../es/build-install-source.md)

**Generally, please follow this procedure.**

## Install Prerequisite Ubuntu Packages

### For Build from Source
```bash
# Core build tools
sudo apt install build-essential cmake autotools-dev autoconf automake libtool pkg-config

# DPDK prerequisites
sudo apt install python3 python3-pip meson ninja-build python3-pyelftools libnuma-dev pkgconf
```

### Optional Packages
```bash
sudo apt install etckeeper tig bridge-utils \
                 iptables-persistent fail2ban dmidecode screen ripgrep
```

## Build sdplane-oss from Source

```bash
# Clone the repository
git clone https://github.com/kait-cronos/sdplane-oss
cd sdplane-oss

# Generate build files
sh autogen.sh

# Configure and build
mkdir build
cd build
CFLAGS="-g -O0" sh ../configure
make

# Install to the system
# This will install to $prefix (default: /usr/local/sbin)
sudo make install
```