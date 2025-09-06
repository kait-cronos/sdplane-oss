# Packet Generator (PKTGEN) Application

**Language:** **English** | [Japanese](ja/packet-generator-application.md) | [French](fr/packet-generator-application.md) | [Chinese](zh/packet-generator-application.md)

The Packet Generator (PKTGEN) application provides high-performance packet generation and traffic testing capabilities using DPDK's optimized packet processing framework.

## Overview

PKTGEN is a sophisticated traffic generation tool integrated into sdplane-oss that enables:
- **High-speed packet generation**: Multi-gigabit packet transmission rates
- **Flexible traffic patterns**: Customizable packet sizes, rates, and patterns
- **Multi-port support**: Independent traffic generation on multiple ports
- **Advanced features**: Range testing, rate limiting, and traffic shaping
- **Performance testing**: Network throughput and latency measurements

## Architecture

### Core Components
- **TX Engine**: High-performance packet transmission using DPDK
- **RX Engine**: Packet reception and statistics collection
- **L2P Framework**: Lcore-to-port mapping for optimal performance
- **Configuration Management**: Dynamic traffic parameter configuration
- **Statistics Engine**: Comprehensive performance metrics and reporting

### Worker Model
PKTGEN operates using dedicated worker threads (lcores):
- **TX Workers**: Dedicated cores for packet transmission
- **RX Workers**: Dedicated cores for packet reception
- **Mixed Workers**: Combined TX/RX on single cores
- **Control Thread**: Management and statistics collection

## Key Features

### Traffic Generation
- **Packet rates**: Line rate traffic generation up to interface limits
- **Packet sizes**: Configurable from 64 bytes to jumbo frames
- **Traffic patterns**: Uniform, burst, and custom patterns
- **Multi-stream**: Multiple traffic streams per port

### Advanced Capabilities  
- **Rate limiting**: Precise traffic rate control
- **Range testing**: Packet size sweeps and rate sweeps
- **Load patterns**: Constant, ramp-up, and burst traffic
- **Protocol support**: Ethernet, IPv4, IPv6, TCP, UDP

### Performance Monitoring
- **Real-time statistics**: TX/RX rates, packet counts, error counts
- **Latency measurement**: End-to-end packet latency testing
- **Throughput analysis**: Bandwidth utilization and efficiency
- **Error detection**: Packet loss and corruption detection

## Configuration

### Basic Setup
PKTGEN requires specific initialization and worker configuration:

```bash
# Device binding for PKTGEN
set device 01:00.0 driver vfio-pci bind
set device 01:00.1 driver vfio-pci bind

# PKTGEN initialization with argv-list
set argv-list 2 ./usr/local/bin/pktgen -l 0-7 -n 4 --proc-type auto --log-level 7 --file-prefix pg -- -v -T -P -l pktgen.log -m [4:5].0 -m [6:7].1 -f themes/black-yellow.theme

# Initialize PKTGEN
pktgen init argv-list 2
```

### Worker Assignment
Assign dedicated cores for optimal performance:

```bash
# Assign PKTGEN workers to specific lcores
set worker lcore 4 pktgen  # Port 0 TX/RX
set worker lcore 5 pktgen  # Port 0 TX/RX  
set worker lcore 6 pktgen  # Port 1 TX/RX
set worker lcore 7 pktgen  # Port 1 TX/RX
```

### Port Configuration
Configure DPDK ports for PKTGEN use:

```bash
# Port setup
stop port all
set port all dev-configure 1 4
set port all nrxdesc 1024
set port all ntxdesc 1024
set port all promiscuous enable
start port all
```

## Example Configuration

### Complete PKTGEN Setup
See [`example-config/sdplane-pktgen.conf`](../../example-config/sdplane-pktgen.conf):

```bash
# Logging configuration
log file /var/log/sdplane.log
log stdout

# Device binding
set device 01:00.0 driver vfio-pci bind
set device 01:00.1 driver vfio-pci bind
set device 03:00.0 driver uio_pci_generic bind

# PKTGEN initialization
set argv-list 2 ./usr/local/bin/pktgen -l 0-7 -n 4 --proc-type auto --log-level 7 --file-prefix pg -- -v -T -P -l pktgen.log -m [4:5].0 -m [6:7].1 -f themes/black-yellow.theme
pktgen init argv-list 2

# Memory pool setup
set mempool

# Background workers
set worker lthread stat-collector
set worker lthread rib-manager
set worker lthread netlink-thread

# Port configuration  
stop port all
set port all dev-configure 1 4
set port all nrxdesc 1024
set port all ntxdesc 1024

# Worker assignments
set worker lcore 2 vlan-switch
set worker lcore 3 tap-handler
set worker lcore 4 pktgen
set worker lcore 5 pktgen
set worker lcore 6 pktgen  
set worker lcore 7 pktgen

# Enable promiscuous mode and start
set port all promiscuous enable
start port all

# Start all workers
start worker lcore all
```

## PKTGEN Command-Line Arguments

### Core Parameters
- `-l 0-7`: CPU core list (cores 0-7)
- `-n 4`: Number of memory channels
- `--proc-type auto`: Process type (primary/secondary)
- `--log-level 7`: Debug logging level
- `--file-prefix pg`: Shared memory prefix

### PKTGEN-Specific Options
- `-v`: Verbose output
- `-T`: Enable color terminal output
- `-P`: Enable promiscuous mode
- `-l pktgen.log`: Log file location
- `-m [4:5].0`: Map lcores 4,5 to port 0
- `-m [6:7].1`: Map lcores 6,7 to port 1
- `-f themes/black-yellow.theme`: Color theme

## Lcore-to-Port Mapping

### Mapping Syntax
The mapping format `[lcore_list].port` assigns cores to ports:
- `[4:5].0`: Lcores 4 and 5 handle port 0
- `[6:7].1`: Lcores 6 and 7 handle port 1
- `[4].0`: Single lcore 4 handles port 0
- `[4-7].0`: Lcores 4 through 7 handle port 0

### Performance Optimization
- **Dedicated cores**: Assign separate cores for TX and RX
- **NUMA awareness**: Use cores local to network interfaces
- **Avoid conflicts**: Don't overlap with system workers
- **Load balancing**: Distribute traffic across multiple cores

## Operations and Monitoring

### Status Commands
```bash
# Show PKTGEN status and configuration
show pktgen

# Display worker assignments and performance
show worker statistics

# Port-level statistics
show port statistics all
```

### Runtime Management
PKTGEN provides extensive runtime configuration through its interactive CLI:
- **Traffic parameters**: Packet size, rate, pattern
- **Start/stop control**: Per-port traffic control  
- **Statistics**: Real-time performance monitoring
- **Range testing**: Automated parameter sweeps

### Performance Monitoring
```bash
# Monitor real-time statistics
# (Available through PKTGEN interactive interface)

# Key metrics:
# - TX/RX packets per second
# - Bandwidth utilization
# - Packet loss rates
# - Latency measurements
```

## CLI Commands

### PKTGEN Control Commands

#### Initialization
```bash
# Initialize PKTGEN with argv-list configuration
pktgen init argv-list <0-7>
```

#### Start/Stop Traffic Generation
```bash
# Start traffic generation on specific port
pktgen do start port <0-7>
pktgen do start port all

# Stop traffic generation
pktgen do stop port <0-7>
pktgen do stop port all
```

#### Traffic Configuration Commands

##### Set Packet Count
```bash
# Configure number of packets to transmit
pktgen do set port <0-7> count <0-4000000000>
pktgen do set port all count <0-4000000000>
```

##### Set Packet Size
```bash
# Configure packet size in bytes
pktgen do set port <0-7> size <0-9999>
pktgen do set port all size <0-9999>
```

##### Set Transmission Rate
```bash
# Configure transmission rate as percentage
pktgen do set port <0-7> rate <0-100>
pktgen do set port all rate <0-100>
```

##### Set TCP/UDP Port Numbers
```bash
# Configure TCP source and destination ports
pktgen do set port <0-7> tcp src <0-65535> dst <0-65535>
pktgen do set port all tcp src <0-65535> dst <0-65535>

# Configure UDP source and destination ports
pktgen do set port <0-7> udp src <0-65535> dst <0-65535>
pktgen do set port all udp src <0-65535> dst <0-65535>
```

##### Set TTL Value
```bash
# Configure IP Time-to-Live value
pktgen do set port <0-7> ttl <0-255>
pktgen do set port all ttl <0-255>
```

##### Set MAC Addresses
```bash
# Configure source MAC address
pktgen do set port <0-7> mac source <MAC>
pktgen do set port all mac source <MAC>

# Configure destination MAC address
pktgen do set port <0-7> mac destination <MAC>
pktgen do set port all mac destination <MAC>
```

##### Set IPv4 Addresses
```bash
# Configure source IPv4 address
pktgen do set port <0-7> ipv4 source <IPv4>
pktgen do set port all ipv4 source <IPv4>

# Configure destination IPv4 address
pktgen do set port <0-7> ipv4 destination <IPv4>
pktgen do set port all ipv4 destination <IPv4>
```

#### Status and Monitoring Commands
```bash
# Show PKTGEN status and configuration
show pktgen

# Show port statistics
show port statistics all
show port statistics <0-7>
```

## Use Cases

### Network Performance Testing
- **Throughput testing**: Maximum bandwidth measurements
- **Latency testing**: End-to-end delay analysis
- **Load testing**: Sustained traffic generation
- **Stress testing**: Maximum packet rate validation

### Equipment Validation
- **Switch testing**: Forwarding performance validation
- **Router testing**: L3 forwarding performance
- **Interface testing**: Port and cable validation
- **Protocol testing**: Specific protocol behavior

### Network Development
- **Protocol development**: Test new network protocols
- **Application testing**: Generate realistic traffic patterns
- **Performance tuning**: Optimize network configurations
- **Benchmarking**: Standardized performance comparisons

## Performance Tuning

### Core Assignment
```bash
# Optimize core usage for best performance
# Separate TX and RX cores when possible
# Use local NUMA cores for network interfaces
set worker lcore 4 pktgen  # TX core for port 0
set worker lcore 5 pktgen  # RX core for port 0
```

### Memory Configuration
```bash
# Optimize descriptor rings for traffic patterns
set port all nrxdesc 2048  # Increase for high rates
set port all ntxdesc 2048  # Increase for bursting
```

### System Tuning
- **CPU isolation**: Isolate PKTGEN cores from OS scheduler
- **Interrupt affinity**: Bind interrupts to non-PKTGEN cores
- **Memory allocation**: Use hugepages for optimal performance
- **NIC tuning**: Optimize network interface settings

## Troubleshooting

### Common Issues
- **Low performance**: Check core assignments and NUMA topology
- **Packet loss**: Verify buffer sizes and system resources
- **Initialization failure**: Check device binding and permissions
- **Rate limitations**: Verify interface capabilities and configuration

### Debug Strategies
- **Enable verbose logging**: Use higher log levels for detailed output
- **Check statistics**: Monitor TX/RX counters for anomalies
- **Verify mappings**: Ensure correct lcore-to-port assignments
- **System monitoring**: Check CPU, memory, and interrupt usage

#### Debug Commands
```bash
# Enable PKTGEN debug logging
debug sdplane pktgen

# General sdplane debugging  
debug sdplane rib
debug sdplane fdb-change
```

### Performance Validation
```bash
# Verify PKTGEN is achieving expected rates
show pktgen
show port statistics all

# Check for errors or drops
# Monitor system resources during testing
```

## Advanced Features

### Range Testing
PKTGEN supports automated testing across parameter ranges:
- **Packet size sweeps**: Test from 64 to 1518 bytes
- **Rate sweeps**: Test from 1% to 100% line rate
- **Automated reporting**: Generate comprehensive test reports

### Traffic Patterns
- **Constant rate**: Steady traffic generation
- **Burst patterns**: Traffic bursts with idle periods
- **Ramp patterns**: Gradually increasing/decreasing rates
- **Custom patterns**: User-defined traffic profiles

## Related Documentation

- [Packet Generation Commands](packet-generation.md) - Command reference
- [Worker Management](worker-management.md) - Worker configuration
- [Port Management](port-management.md) - DPDK port setup
- [Performance Tuning Guide](#) - System optimization tips

## External Resources

- [DPDK Pktgen Documentation](http://pktgen-dpdk.readthedocs.io/) - Official PKTGEN documentation
- [DPDK Performance Guide](https://doc.dpdk.org/guides/prog_guide/) - DPDK optimization guide