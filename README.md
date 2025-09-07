
<div align="center">
<img src="sdplane-logo.png" alt="sdplane-oss Logo" width="160">
</div>

# sdplane-oss (Soft Data Plane)

A "DPDK-dock Development Environment" consisting of an interactive shell that can control DPDK thread operations and a DPDK thread execution environment (sd-plane)

**Language:** **English** | [Japanese](doc/README.ja.md) | [French](doc/README.fr.md) | [Chinese](doc/README.zh.md) | [German](doc/README.de.md) | [Italian](doc/README.it.md) | [Korean](doc/README.ko.md) | [Thai](doc/README.th.md) | [Spanish](doc/README.es.md)

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
- **Topton (N305/N100)**: Mini-PC with 10G NICs (tested)
- **Partaker (J3160)**: Mini-PC with 1G NICs (tested)
- **Intel Generic PC**: With Intel x520 / Mellanox ConnectX5
- **Other CPUs**: Should work with AMD, ARM processors, etc.

## 1. Install Dependencies

See [Install Dependencies Guide](doc/manual/en/install-dependencies.md) for detailed instructions on installing all required dependencies including liburcu-qsbr, libpcap, lthread, and DPDK.

## 2. Install From Pre-compiled Debian Package

For Intel Core i3-n305/Celeron j3160, quick installation is possible with Debian packages.

Download and install the pre-built Debian package:

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

**Note**: Use of this pre-compiled binary on other CPUs may cause SIGILL (Illegal Instruction). In that case you have to compile by yourself.

Jump to 5. System Configuration.

## 3. Build and Install From Source

See [Build and Install from Source Guide](doc/manual/en/build-install-source.md) for detailed instructions on building sdplane-oss from source code.

You can jump to 5. System Configuration.

## 4. Build Debian Package and Install

See [Build Debian Package Guide](doc/manual/en/build-debian-package.md) for instructions on creating and installing a Debian package from source.

## 5. System Configuration

See [System Configuration Guide](doc/manual/en/system-configuration.md) for instructions on configuring hugepages, network interfaces, and optional kernel modules.

## 6. Sdplane Configuration

### Configuration Files

When installed from Debian Package, `/etc/sdplane/sdplane.conf.sample` and systemd service files are automatically generated.

Create `/etc/sdplane/sdplane.conf` referring to the samples.

#### OS Configuration Examples (`etc/`)
- [`etc/sdplane.conf.sample`](etc/sdplane.conf.sample): Main configuration template
- [`etc/sdplane.service`](etc/sdplane.service): systemd service file
- [`etc/modules-load.d/`](etc/modules-load.d/): Kernel module loading configuration

#### Application Configuration Examples (`example-config/`)
- [`example-config/sdplane-pktgen.conf`](example-config/sdplane-pktgen.conf): Packet generator configuration
- [`example-config/sdplane-topton.conf`](example-config/sdplane-topton.conf): Topton hardware configuration
- [`example-config/sdplane_l2_repeater.conf`](example-config/sdplane_l2_repeater.conf): L2 repeater configuration
- [`example-config/sdplane_enhanced_repeater.conf`](example-config/sdplane_enhanced_repeater.conf): Enhanced repeater configuration with VLAN switching, router interfaces, and capture interfaces

## 7. Run the Sdplane Application

```bash
# Run in foreground
sudo sdplane

# Run with config file
sudo sdplane -f /etc/sdplane/sdplane_enhanced_repeater.conf

# When installed via apt, run via systemd
sudo systemctl enable sdplane
sudo systemctl start sdplane

# Connect to CLI
telnet localhost 9882
```

## Tips

### IOMMU is required when using vfio-pci as NIC driver

- Intel: Intel VT-d
- AMD: AMD IOMMU / AMD-V

These need to be enabled in BIOS settings.
GRUB configuration may also need to be changed:

```conf
# /etc/default/grub
GRUB_CMDLINE_LINUX="iommu=pt intel_iommu=on"
```

```bash
sudo update-grub
sudo reboot
```

### Configuration to permanently load vfio-pci Linux kernel module

```conf
#/etc/modules-load.d/vfio-pci.conf
vfio-pci
```

### For Mellanox ConnectX Series

Driver installation is required from the following link:

https://network.nvidia.com/products/ethernet-drivers/linux/mlnx_en/

During installation, run `./install --dpdk`.
**The option `--dpdk` is mandatory.**

Comment out the following settings in sdplane.conf as they are not needed:

```conf
#set device {pcie-id} driver unbound
#set device {pcie-id} driver {driver-name} driver_override
#set device {pcie-id} driver {driver-name} bind
```

For Mellanox NICs, you need to run the `update port status` command to refresh port information.

### How to check PCIe bus numbers

You can use the dpdk-devbind.py command in DPDK to check the PCIe bus numbers of NICs:

```bash
> dpdk-devbind.py -s     

Network devices using kernel driver
===================================
0000:04:00.0 'NetXtreme BCM5720 Gigabit Ethernet PCIe 165f' numa_node=0 if=eno8303 drv=tg3 unused= *Active*
0000:b1:00.0 'MT27800 Family [ConnectX-5] 1017' numa_node=1 if=enp177s0np0 drv=mlx5_core unused= *Active*
```

### Worker thread ordering in configuration file

If configuring `rib-manager`, `neigh-manager`, and `netlink-thread` workers, they must be configured in this order if they are used.

### DPDK initialization

Only one command that calls `rte_eal_init()` should be invoked from the configuration file. The `rte_eal_init()` function is called by commands such as `rte_eal_init`, `pktgen init`, `l2fwd init`, and `l3fwd init`.

## User's Guide (Manual)

Comprehensive user guides and command references are available:

- [User Guide](doc/manual/en/README.md) - Complete overview and command classification

**Application Guides:**
- [L2 Repeater Application](doc/manual/en/l2-repeater-application.md) - Simple Layer 2 packet forwarding with MAC learning
- [Enhanced Repeater Application](doc/manual/en/enhanced-repeater-application.md) - VLAN-aware switching with TAP interfaces
- [Packet Generator Application](doc/manual/en/packet-generator-application.md) - High-performance traffic generation and testing

**Configuration Guides:**
- [Port Management & Statistics](doc/manual/en/port-management.md) - DPDK port management and statistics
- [Worker & lcore Management & Thread Information](doc/manual/en/worker-lcore-thread-management.md) - Worker threads, lcore, and thread information management
- [Debug & Logging](doc/manual/en/debug-logging.md) - Debug and logging functions
- [VTY & Shell Management](doc/manual/en/vty-shell.md) - VTY and shell management
- [System Information & Monitoring](doc/manual/en/system-monitoring.md) - System information and monitoring
- [RIB & Routing](doc/manual/en/routing.md) - RIB and routing functions
- [Queue Configuration](doc/manual/en/queue-configuration.md) - Queue configuration and management
- [Packet Generation](doc/manual/en/packet-generation.md) - Packet generation using PKTGEN
- [TAP Interface](doc/manual/en/tap-interface.md) - TAP interface management
- [lthread Management](doc/manual/en/lthread-management.md) - lthread management
- [Device Management](doc/manual/en/device-management.md) - Device and driver management
- [Enhanced Repeater](doc/manual/en/enhanced-repeater.md) - Virtual switching, VLAN processing, and TAP interfaces

## Developer's Guide

### Integration Guide

- [DPDK Application Integration Guide](doc/manual/en/dpdk-integration-guide.md) - How to integrate DPDK applications into sdplane using DPDK-dock approach

### Documentation

- [Technical Presentation/2024-11-22-sdn-onsen-yasu.pdf](https://enog.jp/wordpress/wp-content/uploads/2024/11/2024-11-22-sdn-onsen-yasu.pdf) (Japanese)
- [Technical Presentation/20250822_ENOG87_ohara.pdf](https://enog.jp/wordpress/wp-content/uploads/2025/08/20250822_ENOG87_ohara.pdf) (Japanese)

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

## Evaluation Equipment Purchase

Evaluation equipment with additional features and software modifications may be available. Please visit our sales page for more information:

**[https://www.rca.co.jp/sdplane/](https://www.rca.co.jp/sdplane/)**

*Note: The sales page is currently available in Japanese only.*

