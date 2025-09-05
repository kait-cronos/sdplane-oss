# L2 Repeater Application

**Language / 言語:** **English** | [Japanese](ja/l2-repeater-application.md)

The L2 Repeater application provides simple Layer 2 packet forwarding between DPDK ports with basic port-to-port repeating functionality.

## Overview

L2 Repeater is a straightforward Layer 2 forwarding application that:
- Forwards packets between paired DPDK ports (simple port-to-port repeating)
- Provides basic packet repeating without MAC address learning
- Supports optional MAC address updating (modifying source MAC addresses)
- Operates with high performance using DPDK's zero-copy packet processing

## Key Features

### Layer 2 Forwarding
- **Port-to-port repeating**: Simple packet forwarding between pre-configured port pairs
- **No MAC learning**: Direct packet repeating without building forwarding tables
- **Transparent forwarding**: All packets are forwarded regardless of destination MAC
- **Port pairing**: Fixed port-to-port forwarding configurations

### Performance Characteristics
- **Zero-copy processing**: Uses DPDK's efficient packet handling
- **Burst processing**: Processes packets in bursts for optimal throughput  
- **Low latency**: Minimal processing overhead for fast forwarding
- **Multi-core support**: Can run on dedicated lcores for scaling

## Configuration

### Basic Setup
The L2 repeater is configured through the main sdplane configuration system:

```bash
# Set worker type to L2 repeater
set worker lcore 1 l2-repeater

# Configure ports and queues
set thread 1 port 0 queue 0  
set thread 1 port 1 queue 0

# Enable promiscuous mode for learning
set port all promiscuous enable
```

### Example Configuration File
See [`example-config/sdplane_l2_repeater.conf`](../../example-config/sdplane_l2_repeater.conf) for a complete configuration example:

```bash
# Device binding
set device 02:00.0 driver vfio-pci bind
set device 03:00.0 driver vfio-pci bind

# DPDK initialization
set rte_eal argv -c 0x7
rte_eal_init
set mempool

# Port configuration
stop port all
set port all dev-configure 1 4
set port all nrxdesc 1024
set port all ntxdesc 1024

# Worker setup
set worker lcore 1 l2-repeater
set worker lcore 2 tap-handler
set port all promiscuous enable
start port all

# Start workers
start worker lcore all
```

## Operation

### Port Pairing
The L2 repeater uses pre-configured port pairs for packet forwarding:
- **Fixed mapping**: Uses `l2fwd_dst_ports[]` array to define port pairs
- **Direct forwarding**: All packets received on a port are forwarded to its paired port
- **No filtering**: Forwards all packet types (unicast, broadcast, multicast)

### Forwarding Behavior
- **All traffic forwarded**: Repeats all packets regardless of destination MAC
- **Port-based**: Forwarding decision based solely on ingress port
- **Transparent**: No modification of packet contents (unless MAC updating is enabled)
- **Bidirectional**: Supports bidirectional traffic between port pairs

### MAC Address Updating
When enabled, the L2 repeater can modify packet MAC addresses:
- **Source MAC update**: Changes source MAC to the egress port's MAC
- **Transparent bridging**: Maintains original MAC addresses (default)

## Performance Tuning

### Buffer Configuration
```bash
# Optimize descriptor counts for your workload
set port all nrxdesc 2048  # Increase for high packet rates
set port all ntxdesc 2048  # Increase for buffering
```

### Worker Assignment
```bash
# Dedicate specific lcores for L2 forwarding
set worker lcore 1 l2-repeater  # Assign to dedicated core
set worker lcore 2 tap-handler  # Separate TAP handling
```

### Memory Pool Sizing
The memory pool should be sized appropriately for the expected traffic:
- Consider packet rate and buffer requirements
- Account for burst sizes and temporary packet storage

## Monitoring and Debugging

### Port Statistics
```bash
# View forwarding statistics
show port statistics all

# Monitor specific ports  
show port statistics 0
show port statistics 1
```

### Debug Commands
```bash
# Enable L2 forwarding debug
debug l2fwd enable

# View port pairing configuration
show l2fwd configuration
```

## Use Cases

### Simple Bridge
- Connect two network segments
- Transparent Layer 2 repeating
- Basic bridge functionality without learning

### Port Mirroring/Repeating
- Mirror traffic between ports
- Network monitoring and analysis
- Simple packet replication

### Performance Testing
- Measure forwarding performance
- Baseline L2 forwarding benchmarks
- Validate DPDK port configurations

## Limitations

- **No VLAN processing**: Simple L2 repeating without VLAN awareness
- **No MAC learning**: Fixed port-to-port forwarding without address learning
- **No STP support**: No Spanning Tree Protocol implementation
- **No filtering**: All packets are forwarded regardless of destination

## Related Applications

- **Enhanced Repeater**: Advanced version with VLAN support and TAP interfaces
- **L3 Forwarding**: Layer 3 routing functionality
- **VLAN Switch**: VLAN-aware switching capabilities

For more advanced Layer 2 functionality with VLAN support, see the [Enhanced Repeater](enhanced-repeater.md) documentation.