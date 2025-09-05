# Enhanced Repeater Application

**Language / 言語:** **English** | [Japanese](ja/enhanced-repeater-application.md)

The Enhanced Repeater application provides advanced VLAN-aware Layer 2 switching with TAP interface integration for L3 routing and packet capture capabilities.

## Overview

Enhanced Repeater is a sophisticated Layer 2 forwarding application that extends basic L2 forwarding with:
- **VLAN-aware switching**: Full IEEE 802.1Q VLAN support with tagging/untagging
- **Virtual switch abstraction**: Multiple virtual switches with independent forwarding tables
- **TAP interface integration**: Router and capture interfaces for kernel integration
- **Advanced VLAN processing**: VLAN translation, insertion, and stripping capabilities

## Architecture

### Virtual Switch Framework
The Enhanced Repeater implements a virtual switch architecture:
- **Multiple VLANs**: Support for multiple VLAN domains (1-4094)
- **Port aggregation**: Multiple physical ports per virtual switch
- **Isolated forwarding**: Independent forwarding domains per VLAN
- **Flexible tagging**: Native, tagged, and translation modes per port

### TAP Interface Integration
- **Router interfaces**: Kernel networking stack integration for L3 processing
- **Capture interfaces**: Packet monitoring and analysis capabilities  
- **Ring buffers**: Efficient packet transfer between data plane and kernel
- **Bidirectional**: Both ingress and egress packet handling

## Key Features

### VLAN Processing
- **VLAN tagging**: Add 802.1Q headers to untagged frames
- **VLAN untagging**: Remove 802.1Q headers from tagged frames
- **VLAN translation**: Modify VLAN IDs between ingress and egress
- **Native VLAN**: Handle untagged traffic on trunk ports

### Virtual Switching
- **Learning**: Automatic MAC address learning per VLAN
- **Flooding**: Proper handling of unknown unicast and broadcast
- **Split horizon**: Loop prevention within virtual switches
- **Multi-domain**: Independent forwarding tables per VLAN

### Packet Processing
- **Zero-copy**: Efficient DPDK packet handling with minimal overhead
- **Burst processing**: Optimized for high packet rates
- **Header manipulation**: Efficient VLAN header insertion/removal
- **Copy optimization**: Selective packet copying for TAP interfaces

## Configuration

### Virtual Switch Setup
Create virtual switches for different VLAN domains:

```bash
# Create virtual switches with VLAN IDs
set vswitch 2031
set vswitch 2032
```

### Port to VSwitch Linking
Link physical DPDK ports to virtual switches:

```bash
# Link port 0 to vswitch 0 with VLAN tag 2031
set vswitch-link vswitch 0 port 0 tag 2031

# Link port 0 to vswitch 1 with native/untagged
set vswitch-link vswitch 1 port 0 tag 0

# VLAN translation example
set vswitch-link vswitch 0 port 1 tag 2040
```

### Router Interfaces
Create router interfaces for L3 connectivity:

```bash
# Create router interfaces for L3 processing
set router-if 0 rif0
set router-if 1 rif1
```

### Capture Interfaces  
Setup capture interfaces for monitoring:

```bash
# Create capture interfaces for packet monitoring
set capture-if 0 cif0
set capture-if 1 cif1
```

### Worker Configuration
Assign the enhanced repeater to an lcore:

```bash
# Set enhanced repeater worker
set worker lcore 1 enhanced-repeater

# Configure queue assignments
set thread 1 port 0 queue 0
```

## Example Configuration

### Complete Setup
See [`example-config/sdplane_enhanced_repeater.conf`](../../example-config/sdplane_enhanced_repeater.conf):

```bash
# Device setup
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
set worker lthread stat-collector
set worker lthread rib-manager
set worker lcore 1 enhanced-repeater
set worker lcore 2 l3-tap-handler

# Enable promiscuous mode
set port all promiscuous enable
start port all

# Queue configuration
set thread 1 port 0 queue 0

# Virtual switch setup
set vswitch 2031
set vswitch 2032

# Link configuration
set vswitch-link vswitch 0 port 0 tag 2031
set vswitch-link vswitch 1 port 0 tag 2032

# Interface creation
set router-if 0 rif0
set router-if 1 rif1
set capture-if 0 cif0
set capture-if 1 cif1

# Start workers
start worker lcore all
```

## VLAN Processing Details

### Tagging Modes

#### Tagged Mode
- **Ingress**: Accepts packets with specific VLAN tag
- **Egress**: Maintains or translates VLAN tag
- **Use case**: Trunk ports, VLAN translation

#### Native Mode (tag 0)  
- **Ingress**: Accepts untagged packets
- **Egress**: Removes VLAN headers
- **Use case**: Access ports, VLAN-unaware devices

#### Translation Mode
- **Function**: Changes VLAN IDs between ports
- **Configuration**: Different tags on ingress/egress links
- **Use case**: VLAN mapping, service provider networks

### Packet Flow

1. **Ingress Processing**:
   - Receive packet on DPDK port
   - Determine VLAN based on tag or native configuration
   - Lookup destination virtual switch

2. **Virtual Switch Lookup**:
   - MAC address learning and lookup
   - Determine egress ports within VLAN domain
   - Handle unknown unicast/broadcast flooding

3. **Egress Processing**:
   - VLAN header manipulation per port configuration
   - Packet copying for multiple destinations
   - TAP interface integration

4. **TAP Interface Handling**:
   - Router interface: Kernel L3 processing
   - Capture interface: Monitoring and analysis

## Monitoring and Management

### Status Commands
```bash
# Show virtual switch configuration
show vswitch_rib

# Display virtual switch links
show vswitch-link  

# View router interfaces
show router-if

# Check capture interfaces
show capture-if
```

### Statistics and Performance
```bash
# Port-level statistics
show port statistics all

# Worker performance
show worker statistics

# Thread information
show thread information
```

### Debug Commands
```bash
# Enhanced repeater debug
debug sdplane enhanced-repeater

# RIB and forwarding debug
debug sdplane rib
debug sdplane fdb-change
```

## Use Cases

### VLAN Aggregation
- Consolidate multiple VLANs onto single physical links
- Provider edge functionality
- Service multiplexing

### L3 Integration
- Router-on-a-stick configurations  
- Inter-VLAN routing via TAP interfaces
- Hybrid L2/L3 forwarding

### Network Monitoring
- Per-VLAN packet capture
- Traffic analysis and debugging
- Service monitoring

### Service Provider Networks
- VLAN translation for customer isolation
- Multi-tenant networking
- Traffic engineering

## Performance Considerations

### Scaling
- **Worker assignment**: Dedicate lcores for optimal performance
- **Queue configuration**: Balance queue assignments across cores
- **Memory pools**: Size appropriately for packet rates and buffer needs

### VLAN Processing Overhead
- **Header manipulation**: Minimal overhead for VLAN operations
- **Packet copying**: Selective copying only when necessary
- **Burst optimization**: Process packets in bursts for efficiency

### TAP Interface Performance
- **Ring buffer sizing**: Configure appropriate ring sizes
- **Selective forwarding**: Send only relevant packets to TAP interfaces
- **Kernel integration**: Consider kernel processing overhead

## Troubleshooting

### Common Issues
- **VLAN mismatch**: Verify tag configurations match network setup
- **TAP interface creation**: Ensure proper permissions and kernel support
- **Performance issues**: Check queue assignments and worker distribution

### Debug Strategies
- **Enable debug logging**: Use debug commands for detailed packet flow
- **Statistics monitoring**: Watch port and worker statistics
- **Packet capture**: Use capture interfaces for traffic analysis

## Related Documentation

- [Enhanced Repeater Commands](enhanced-repeater.md) - Complete command reference
- [Worker Management](worker-management.md) - Worker configuration details
- [Port Management](port-management.md) - DPDK port configuration
- [TAP Interface Management](tap-interface.md) - TAP interface details