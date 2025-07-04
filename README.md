
# sdplane-dev (Soft Data Plane)

<a href="https://gist.github.com/cheerfulstoic/d107229326a01ff0f333a1d3476e068d"><img src="https://img.shields.io/badge/Maintenance%20Level-Actively%20Developed-brightgreen.svg" /></a>

A high-performance open-source software router built on DPDK (Data Plane Development Kit), designed for software-defined networking applications.

## Features

- **High-Performance Packet Processing**: Leverages DPDK for zero-copy, user-space packet processing
- **Layer 2/3 Forwarding**: Integrated L2 and L3 forwarding with ACL, LPM, and FIB support
- **Packet Generation**: Built-in packet generator for testing and benchmarking
- **Network Virtualization**: TAP interface support and VLAN switching capabilities
- **CLI Management**: Interactive command-line interface for configuration and monitoring
- **Multi-threading**: Cooperative threading model with per-core workers

## Requirements

### Dependencies
- **DPDK**: Data Plane Development Kit
- **libsdplane-dev**: [kait-cronos/libsdplane-dev](https://github.com/kait-cronos/libsdplane-dev) (build and install separately)
- **lthread**: [yasuhironet/lthread](https://github.com/yasuhironet/lthread) (DPDK-based cooperative threading)
- **liburcu-qsbr**: Userspace RCU library
- **libpcap**: Packet capture library

### System Requirements
- **OS**: Ubuntu Linux (currently supported)
- **NICs**: 4 network interfaces (virtio-net supported for virtual environments)
- **Memory**: Hugepage support required
- **CPU**: Multi-core processor recommended

### Ubuntu Packages
```bash
sudo apt install etckeeper tig build-essential bridge-utils cmake \
                 iptables-persistent fail2ban dmidecode screen ripgrep \
                 autotools-dev autoconf automake libtool pkg-config
```

## Quick Start

### 1. Install Dependencies

First, install and build the required libraries:
```bash
# Install libsdplane-dev (build from source)
git clone https://github.com/kait-cronos/libsdplane-dev
cd libsdplane-dev
# Follow build instructions in that repository

# Install lthread
git clone https://github.com/yasuhironet/lthread
cd lthread
# Follow build instructions in that repository
```

### 2. Build sdplane-dev

```bash
# Clone the repository
git clone https://github.com/kait-cronos/sdplane-dev
cd sdplane-dev

# Generate build files
./autogen.sh

# Configure and build
mkdir build
cd build
../configure
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
- `etc/60-netplan-sdplane.yaml`: Network interface configuration
- `etc/iptables-rules.v4`: IPv4 firewall rules
- `etc/iptables-rules.v6`: IPv6 firewall rules
- `etc/sshd_config`: SSH daemon configuration
- `etc/modules-load.d/`: Kernel module loading configuration

#### Application Configuration (`example-config/`)
- `example-config/sdplane-nettlp.conf`: NetTLP configuration
- `example-config/sdplane-pktgen.conf`: Packet generator configuration
- `example-config/sdplane-topton.conf`: Topton hardware configuration
- `example-config/sdplane_l2_repeater.conf`: L2 repeater configuration
- `example-config/sdplane_l2fwd.conf`: L2 forwarding configuration
- `example-config/sdplane_l3fwd-lpm.conf`: L3 forwarding with LPM configuration

## Hardware Platforms

The project has been tested on:
- **Topton**: Mini-PC with 10G NICs
- **Wiretap**: Mini-PC with 1G NICs

## Documentation

- [Topton Installation Guide](doc/install-memo-topton.txt) - For 10G NIC systems
- [General Installation Guide](doc/install-memo.txt) - For 1G NIC systems
- [NetTLP Configuration Guide](doc/nettlp-memo.txt) - NetTLP setup instructions
- [Technical Presentation](https://enog.jp/wordpress/wp-content/uploads/2024/11/2024-11-22-sdn-onsen-yasu.pdf) (Japanese)

## Development

### Code Style
The project follows GNU coding standards. Use the provided scripts to check and format code:

```bash
# Check formatting
./style/check_gnu_style.sh check

# Auto-format code
./style/check_gnu_style.sh update
```

### Architecture
- **Main Application**: Core router logic and initialization
- **DPDK Modules**: L2/L3 forwarding and packet generation
- **CLI System**: Command-line interface with completion and help
- **Threading**: lthread-based cooperative multitasking
- **Virtualization**: TAP interfaces and virtual switching

## License

This project is open source. See the COPYING file for license details.

## Author

Yasuhiro Ohara (yasu1976@gmail.com)

## Contributing

Contributions are welcome! Please ensure code follows the project's coding standards and includes appropriate documentation.

