# Packet Generation

**Language / 言語:** **English** | [Japanese](ja/packet-generation.md)

Commands for packet generation functions using PKTGEN (Packet Generator).

## Command List

### show_pktgen - Display PKTGEN Information
```
show pktgen
```

Display current status and configuration of PKTGEN (Packet Generator).

**Examples:**
```bash
show pktgen
```

This command displays the following information:
- PKTGEN initialization status
- Current configuration parameters
- Running tasks
- Statistics information

### pktgen_init - PKTGEN Initialization
```
pktgen init argv-list <0-7>
```

Initialize PKTGEN using specified argv-list.

**Parameters:**
- `<0-7>` - argv-list index to use

**Examples:**
```bash
# Initialize PKTGEN using argv-list 0
pktgen init argv-list 0

# Initialize PKTGEN using argv-list 2
pktgen init argv-list 2
```

### pktgen_do_start - Start PKTGEN
```
pktgen do start (<0-7>|all)
```

Start packet generation on specified ports.

**Target:**
- `<0-7>` - Specific port number
- `all` - All ports

**Examples:**
```bash
# Start packet generation on port 0
pktgen do start 0

# Start packet generation on all ports
pktgen do start all
```

### pktgen_do_stop - Stop PKTGEN
```
pktgen do stop (<0-7>|all)
```

Stop packet generation on specified ports.

**Target:**
- `<0-7>` - Specific port number
- `all` - All ports

**Examples:**
```bash
# Stop packet generation on port 1
pktgen do stop 1

# Stop packet generation on all ports
pktgen do stop all
```

## PKTGEN Overview

### What is PKTGEN
PKTGEN (Packet Generator) is a packet generation tool for network testing. It provides the following functions:

- **High-Speed Packet Generation** - High-performance packet generation
- **Various Packet Formats** - Support for various protocols
- **Flexible Configuration** - Detailed packet configuration possible
- **Statistics Functions** - Provision of detailed statistics

### Main Uses
- **Network Performance Testing** - Throughput and latency measurement
- **Load Testing** - System load resistance testing
- **Function Testing** - Network function verification
- **Benchmarking** - Performance comparison testing

## PKTGEN Configuration

### Basic Configuration Steps
1. **Configure argv-list**
```bash
# Set PKTGEN parameters
set argv-list 0 "-c 0x3 -n 4"
set argv-list 1 "--socket-mem 1024"
set argv-list 2 "--huge-dir /mnt/huge"
```

2. **Initialize PKTGEN**
```bash
pktgen init argv-list 0
```

3. **Configure Worker**
```bash
set worker lcore 1 pktgen
start worker lcore 1
```

4. **Start Packet Generation**
```bash
pktgen do start 0
```

### Configuration Parameters
Example parameters configurable with argv-list:

- **-c** - CPU mask
- **-n** - Number of memory channels
- **--socket-mem** - Socket memory size
- **--huge-dir** - Hugepage directory
- **--file-prefix** - File prefix

## Usage Examples

### Basic Packet Generation
```bash
# Configuration
set argv-list 0 "-c 0x3 -n 4 --socket-mem 1024"

# Initialization
pktgen init argv-list 0

# Worker configuration
set worker lcore 1 pktgen
start worker lcore 1

# Start packet generation
pktgen do start 0

# Check status
show pktgen

# Stop packet generation
pktgen do stop 0
```

### Multi-port Generation
```bash
# Start on multiple ports
pktgen do start all

# Check status
show pktgen

# Stop on multiple ports
pktgen do stop all
```

## Monitoring and Statistics

### Statistics Information Check
```bash
# Display PKTGEN statistics
show pktgen

# Display port statistics
show port statistics

# Display worker statistics
show worker
```

### Performance Monitoring
```bash
# Check PPS (Packets Per Second)
show port statistics pps

# Check total packet count
show port statistics total

# Check bytes per second
show port statistics Bps
```

## Troubleshooting

### When PKTGEN Does Not Start
1. Check initialization status
```bash
show pktgen
```

2. Check worker status
```bash
show worker
```

3. Check port status
```bash
show port
```

### When Packet Generation Does Not Stop
1. Explicitly stop
```bash
pktgen do stop all
```

2. Restart worker
```bash
restart worker lcore 1
```

### When Performance Is Low
1. Check CPU usage
2. Check memory configuration
3. Check port configuration

## Advanced Features

### Packet Format Configuration
PKTGEN can generate various packet formats:
- **Ethernet** - Basic Ethernet frames
- **IP** - IPv4/IPv6 packets
- **UDP/TCP** - UDP/TCP packets
- **VLAN** - VLAN-tagged packets

### Load Control
- **Rate Control** - Control packet generation rate
- **Burst Control** - Generate burst packets
- **Size Control** - Control packet size

## Definition Location

These commands are defined in the following file:
- `sdplane/pktgen_cmd.c`

## Related Topics

- [Worker & lcore Management](worker-management.md)
- [Port Management & Statistics](port-management.md)
- [System Information & Monitoring](system-monitoring.md)