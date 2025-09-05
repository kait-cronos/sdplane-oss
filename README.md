
# sdplane-oss (Soft Data Plane) <img src="sdplane-logo.png" alt="sdplane-oss Logo" width="100" align="right">

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

## Supported System

### Software Requirements
- **OS**:
  Ubuntu 24.04 LTS (currently supported)
- **NICs**:
  [Drivers](https://doc.dpdk.org/guides/nics/) | [Supported NICs](https://core.dpdk.org/supported/)
- **Memory**:
  Hugepage support required
- **CPU**:
  Multi-core processor recommended

### Target Hardware Platforms

The project has been tested on:
- **Topton (N305/N100)**: Mini-PC with 10G NICs
- **Partaker (J3160)**: Mini-PC with 1G NICs

There is no reason to believe the sdplane-oss doesn't work on other CPUs such as Intel (Core i7/9, Xeon), AMD, ARM processors, etc.

## 1. Install Dependencies

### Dependencies
- **liburcu-qsbr**: Userspace RCU library
- **libpcap**: Packet capture library
- **lthread**: [yasuhironet/lthread](https://github.com/yasuhironet/lthread) (lightweight cooperative threading)
- **DPDK**: Data Plane Development Kit

### Install sdplane dependencies debian packages
```
sudo apt install liburcu-dev libpcap-dev
```

### Install lthread
```bash
# Install lthread
git clone https://github.com/yasuhironet/lthread
cd lthread
cmake .
make
sudo make install
```
### Install DPDK 23.11.1
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

## 2. Quick Start by Debian Package

For quick installation, download and install the pre-built Debian package:

```bash
# Download the latest package for n305
wget https://www.yasuhironet.net/download/n305/sdplane_0.1.4-36_amd64.deb
wget https://www.yasuhironet.net/download/n305/sdplane-dbgsym_0.1.4-36_amd64.ddeb

# or for j3160
wget https://www.yasuhironet.net/download/j3160/sdplane_0.1.4-35_amd64.deb
wget https://www.yasuhironet.net/download/j3160/sdplane-dbgsym_0.1.4-35_amd64.ddeb

# Install the package
sudo apt install ./sdplane_0.1.4-*_amd64.deb
sudo apt install ./sdplane-dbgsym_0.1.4-*_amd64.ddeb
```

**Note**: Check [yasuhironet.net downloads](https://www.yasuhironet.net/download/) for the latest package version.

Jump to 5. System Configuration.

## 3. Build from Source

### Install Prerequisite Ubuntu Packages

#### For Build from Source
```bash
# Core build tools
sudo apt install build-essential cmake autotools-dev autoconf automake libtool pkg-config

# DPDK prerequisites
sudo apt install python3 python3-pip meson ninja-build python3-pyelftools libnuma-dev pkgconf
```

#### Optional Packages
```bash
sudo apt install etckeeper tig bridge-utils \
                 iptables-persistent fail2ban dmidecode screen ripgrep
```

### Build sdplane-oss from Source

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
```

## 4. Build sdplane-oss Debian Package (Optional)

### Install prerequisite package
```bash
sudo apt install build-essential cmake devscripts debhelper
```

### Build sdplane-oss Debian Package
```bash
# First make sure to start in a clean space.
(cd build && make distclean)
make distclean

# Build Debian package from source
bash build-debian.sh

# Install the generated package (will be produced in parent dir)
sudo apt install ../sdplane_*.deb
```

## 5. System Configuration

- **Hugepages**: Configure system hugepages for DPDK
- **Network**: Use netplan for network interface configuration
- **Firewall**: Configure iptables rules as needed

### Configure Hugepages
```bash
# Edit GRUB configuration
sudo vi /etc/default/grub

# Add one of the following lines:
# For 2MB hugepages (1536 pages = ~3GB):
GRUB_CMDLINE_LINUX="hugepages=1536"

# Or for 1GB hugepages (8 pages = 8GB):
GRUB_CMDLINE_LINUX="default_hugepagesz=1G hugepagesz=1G hugepages=8"

# Update GRUB and reboot
sudo update-grub
sudo reboot
```

### Install DPDK IGB Kernel Module (Optional)
```bash
# Option 1: Install from package
sudo apt-get install -y dpdk-igb-uio-dkms

# Option 2: Build from source
git clone http://dpdk.org/git/dpdk-kmods
cd dpdk-kmods/linux/igb_uio
make
sudo mkdir -p /lib/modules/`uname -r`/extra/dpdk/
sudo cp igb_uio.ko /lib/modules/`uname -r`/extra/dpdk/
echo igb_uio | sudo tee /etc/modules-load.d/igb_uio.conf
```

## 6. sdplane Configuration

### Configuration Files

Place one of the following configuration files as
/etc/sdplane/sdplane.conf

#### OS Setup Configuration (`etc/`)
- [`etc/sdplane.conf.sample`](etc/sdplane.conf.sample): Main configuration template
- [`etc/sdplane.service`](etc/sdplane.service): systemd service file
- [`etc/modules-load.d/`](etc/modules-load.d/): Kernel module loading configuration

#### Application Configuration (`example-config/`)
- [`example-config/sdplane-pktgen.conf`](example-config/sdplane-pktgen.conf): Packet generator configuration
- [`example-config/sdplane-topton.conf`](example-config/sdplane-topton.conf): Topton hardware configuration
- [`example-config/sdplane_l2_repeater.conf`](example-config/sdplane_l2_repeater.conf): L2 repeater configuration
- [`example-config/sdplane_enhanced_repeater.conf`](example-config/sdplane_enhanced_repeater.conf): Enhanced repeater configuration with VLAN switching, router interfaces, and capture interfaces

## 7. Run the Software Router

```bash
# Run in foreground
sudo ./sdplane/sdplane

# Run with config file
sudo ./sdplane/sdplane -f /etc/sdplane/sdplane_enhanced_repeater.conf

# Run via systemd, when you installed the dpkg.
sudo systemctl enable sdplane
sudo systemctl start sdplane

# Connect to CLI
telnet localhost 9882
```

### Enhanced Repeater Configuration

The enhanced repeater provides advanced VLAN switching capabilities with TAP interfaces for L3 routing and packet capture. Key configuration commands:

**Virtual Switch Setup:**
```bash
# Create virtual switches with VLAN IDs
set vswitch 2031 vlan 2031
set vswitch 2031 vlan 2032
```

**DPDK Port to VSwitch Linking:**
```bash
# Link port 0 to vswitch with VLAN tag 2031
set vswitch 2031 port 0 (tagged|untag|tag swap 2032)
# Link port 0 to vswitch with VLAN tag 2032  
set vswitch 2032 port 0 (tagged|untag|tag swap 2031)
```

**Router Interfaces (L3 connectivity):**
```bash
# Create router interfaces for L3 processing
set vswitch 2031 router-if rif2031
set vswitch 2032 router-if cif2032
```

**Capture Interfaces (packet monitoring):**
```bash
# Create capture interfaces for packet monitoring
set vswitch 2031 capture-if cif2031
set vswitch 2032 capture-if cif2032
```

The enhanced repeater performs VLAN translation, stripping, and insertion based on the vswitch-link configuration, while providing TAP interfaces for kernel networking stack integration.

## User's Guide (Manual)

Comprehensive user guides and command references are available:

- [User Guide](doc/manual/README.md) - Complete overview and command classification
- [Enhanced Repeater](doc/manual/enhanced-repeater.md) - Virtual switching, VLAN processing, and TAP interfaces
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

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for license details.

## Contact

For questions, issues, or contributions, please contact: **sdplane [at] nwlab.org**

