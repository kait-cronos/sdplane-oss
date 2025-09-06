# Thread Information

**Language:** **English** | [Japanese](ja/thread-information.md) | [French](fr/thread-information.md) | [Chinese](zh/thread-information.md)

Commands for thread information and monitoring.

## Command List

### show_thread_cmd - Display Thread Information
```
show thread
```

Display current thread status and information.

**Examples:**
```bash
show thread
```

This command displays the following information:
- Thread ID
- Thread status
- Running tasks
- CPU usage
- Memory usage

### show_thread_counter - Display Thread Counter
```
show thread counter
```

Display thread counter information.

**Examples:**
```bash
show thread counter
```

This command displays the following information:
- Processed packet count
- Execution count
- Error count
- Processing time statistics

## Thread System Overview

### sdplane Thread Architecture
sdplane uses the following thread types:

#### 1. Worker Threads
- **L2 Forwarding** - Layer 2 packet forwarding
- **L3 Forwarding** - Layer 3 packet forwarding
- **TAP Handler** - TAP interface processing
- **PKTGEN** - Packet generation

#### 2. Management Threads
- **RIB Manager** - Routing information management
- **Statistics Collector** - Statistics collection
- **Netlink Thread** - Netlink communication processing

#### 3. System Threads
- **VTY Server** - VTY connection processing
- **Console** - Console input/output

### Relationship with lthread
sdplane uses a cooperative multithreading model:

- **lthread** - Lightweight thread implementation
- **Cooperative Scheduling** - Control through explicit yield
- **High Efficiency** - Reduced context switch overhead

## Reading Thread Information

### Basic Display Items
- **Thread ID** - Thread identifier
- **Name** - Thread name
- **State** - Thread status
- **lcore** - Running CPU core
- **Type** - Thread type

### Thread Status
- **Running** - Currently running
- **Ready** - Ready to run
- **Blocked** - Blocked
- **Terminated** - Terminated

### Counter Information
- **Packets** - Processed packet count
- **Loops** - Loop execution count
- **Errors** - Error count
- **CPU Time** - CPU usage time

## Usage Examples

### Basic Monitoring
```bash
# Display thread information
show thread

# Display thread counters
show thread counter
```

### Output Example Interpretation
```bash
# show thread output example
Thread ID: 1
Name: l2fwd-worker
State: Running
lcore: 1
Type: L2FWD

Thread ID: 2
Name: rib-manager
State: Running
lcore: 2
Type: RIB_MANAGER
```

```bash
# show thread counter output example
Thread ID: 1
Packets: 1000000
Loops: 5000000
Errors: 0
CPU Time: 123.45s
```

## Monitoring and Troubleshooting

### Regular Monitoring
```bash
# Regular monitoring commands
show thread
show thread counter
```

### Performance Analysis
```bash
# Performance-related information
show thread counter
show loop-count l2fwd pps
show worker
```

### Troubleshooting

#### When Thread Is Not Responding
1. Check thread status
```bash
show thread
```

2. Check worker status
```bash
show worker
```

3. Restart if necessary
```bash
restart worker lcore 1
```

#### When Performance Is Degrading
1. Check counter information
```bash
show thread counter
```

2. Check loop counters
```bash
show loop-count l2fwd pps
```

3. Check error count
```bash
show thread counter
```

#### When Memory Usage Is High
1. Check memory pool information
```bash
show mempool
```

2. Check thread information
```bash
show thread
```

## Thread Optimization

### CPU Affinity Configuration
- Place workers on appropriate lcores
- Consider NUMA node placement
- Equalize CPU usage

### Memory Efficiency Improvement
- Appropriate memory pool sizing
- Prevent memory leaks
- Improve cache efficiency

## Advanced Features

### lthread Management
```bash
# lthread worker configuration
set worker lthread stat-collector
set worker lthread rib-manager
set worker lthread netlink-thread
```

### Statistics Utilization
- Performance monitoring
- Capacity planning
- Anomaly detection

## Definition Location

These commands are defined in the following file:
- `sdplane/thread_info.c`

## Related Topics

- [Worker & lcore Management](worker-management.md)
- [lthread Management](lthread-management.md)
- [System Information & Monitoring](system-monitoring.md)