
# sdplane-oss (Soft Data Plane)

A high-performance open-source software router built on
DPDK (Data Plane Development Kit), designed for
software-defined networking applications.

**Language:** **English** | [Japanese](README.ja.md)

## Features

- **High-Performance Packet Processing**:
  Leverages DPDK for zero-copy, user-space packet processing
- **Layer 2/3 Forwarding**:
  Integrated L2 and L3 forwarding with ACL, LPM, and FIB support
- **Packet Generation**:
  Built-in packet generator for testing and benchmarking
- **Network Virtualization**:
  TAP interface support and VLAN switching capabilities
- **CLI Management**:
  Interactive command-line interface for configuration and monitoring
- **Multi-threading**:
  Cooperative threading model with per-core workers

### Architecture
- **Main Application**: Core router logic and initialization
- **DPDK Modules**: L2/L3 forwarding and packet generation
- **CLI System**: Command-line interface with completion and help
- **Threading**: lthread-based cooperative multitasking
- **Virtualization**: TAP interfaces and virtual switching

## Quick Start by Debian Package


## Build from Source

### System Requirements
- **OS**:
  Ubuntu 24.04 LTS (currently supported)
- **NICs**:
  [DPDK supported NICs](https://doc.dpdk.org/guides/nics/)
- **Memory**:
  Hugepage support required
- **CPU**:
  Multi-core processor recommended

## Hardware Platforms

The project has been tested on:
- **Topton (N305/N100)**: Mini-PC with 10G NICs
- **Partaker (N100)**: Mini-PC with 1G NICs


### Prerequisite Ubuntu Packages

#### For Build from Source
```bash
# Core build tools
sudo apt install build-essential cmake autotools-dev autoconf automake libtool pkg-config

# DPDK prerequisites
sudo apt install python3 python3-pip meson ninja-build python3-pyelftools libnuma-dev pkgconf

# sdplane dependencies
sudo apt install liburcu-dev libpcap-dev
```

#### For Debian Package Build
```bash
sudo apt install build-essential cmake devscripts debhelper
```

#### Optional Packages
```bash
sudo apt install etckeeper tig bridge-utils \
                 iptables-persistent fail2ban dmidecode screen ripgrep
```

### Dependencies
- **DPDK**: Data Plane Development Kit
- **lthread**: [yasuhironet/lthread](https://github.com/yasuhironet/lthread) (DPDK-based cooperative threading)
- **liburcu-qsbr**: Userspace RCU library
- **libpcap**: Packet capture library

### 1. Install Dependencies

First, install the required lthread library:
```bash
# Install lthread
git clone https://github.com/yasuhironet/lthread
cd lthread
cmake .
make
sudo make install
```

### 2. Build sdplane-oss

```bash
# Clone the repository
git clone https://github.com/kait-cronos/sdplane-oss
cd sdplane-oss

# Generate build files
./autogen.sh

# Configure and build
mkdir build
cd build
CFLAGS="-g -O0" sh ../configure
make
```

### 3. Run the Software Router

```bash
# Run in foreground
sudo ./sdplane/sdplane
  or
# Run in background, when you installed the dpkg.
sudo systemctl start sdplane

# connect to CLI
telnet localhost 9882
```

## Configuration

### System Configuration
- **Hugepages**: Configure system hugepages for DPDK
- **Network**: Use netplan for network interface configuration
- **Firewall**: Configure iptables rules as needed

### Configuration Files

#### OS Setup Configuration (`etc/`)
- `etc/sdplane.conf.sample`: Main configuration template
- `etc/sdplane.service`: systemd service file
- `etc/modules-load.d/`: Kernel module loading configuration

#### Application Configuration (`example-config/`)
- `example-config/sdplane-pktgen.conf`: Packet generator configuration
- `example-config/sdplane-topton.conf`: Topton hardware configuration
- `example-config/sdplane_l2_repeater.conf`: L2 repeater configuration
- `example-config/sdplane_l2fwd.conf`: L2 forwarding configuration
- `example-config/sdplane_l3fwd-lpm.conf`: L3 forwarding with LPM configuration

## User's Guide (Manual)

Comprehensive user guides and command references are available:

- [User Guide](doc/manual/README.md) - Complete overview and command classification
- [Port Management & Statistics](doc/manual/port-management.md) - DPDK port management and statistics
- [Worker & lcore Management](doc/manual/worker-management.md) - Worker threads and lcore management
- [Debug & Logging](doc/manual/debug-logging.md) - Debug and logging functions
- [VTY & Shell Management](doc/manual/vty-shell.md) - VTY and shell management
- [System Information & Monitoring](doc/manual/system-monitoring.md) - System information and monitoring
- [RIB & Routing](doc/manual/routing.md) - RIB and routing functions
- [Queue Configuration](doc/manual/queue-configuration.md) - Queue configuration and management
- [Packet Generation](doc/manual/packet-generation.md) - Packet generation using PKTGEN
- [Thread Information](doc/manual/thread-information.md) - Thread information and monitoring
- [TAP Interface](doc/manual/tap-interface.md) - TAP interface management
- [lthread Management](doc/manual/lthread-management.md) - lthread management
- [Device Management](doc/manual/device-management.md) - Device and driver management

## Developer's Guide

### Documentation

- [Topton Installation Guide](doc/install-memo-topton.txt) - For 10G NIC systems
- [General Installation Guide](doc/install-memo.txt) - For 1G NIC systems
- [Technical Presentation](https://enog.jp/wordpress/wp-content/uploads/2024/11/2024-11-22-sdn-onsen-yasu.pdf) (Japanese)

### Code Style
The project follows GNU coding standards. Use the provided scripts to check and format code:

```bash
# Check formatting
./style/check_gnu_style.sh check

# Show formatting differences
./style/check_gnu_style.sh diff

# Auto-format code
./style/check_gnu_style.sh update
```

## License

This project is open source. See the LICENSE file for license details.

