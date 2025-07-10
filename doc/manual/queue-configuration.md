# Queue Configuration

**Language / 言語:** **English** | [Japanese](ja/queue-configuration.md)

Commands for configuring and managing DPDK queues.

## Command List

### update_port_status - Update Port Status
```
update port status
```

Update the status of all ports.

**Examples:**
```bash
update port status
```

This command performs the following:
- Check link status of each port
- Update queue configuration
- Update port statistics

### set_thread_lcore_port_queue - Set Thread Queue Configuration
```
set thread <0-128> port <0-128> queue <0-128>
```

Assign port and queue to specified thread.

**Parameters:**
- `<0-128>` (thread) - Thread number
- `<0-128>` (port) - Port number
- `<0-128>` (queue) - Queue number

**Examples:**
```bash
# Assign port 0 queue 0 to thread 0
set thread 0 port 0 queue 0

# Assign port 1 queue 1 to thread 1
set thread 1 port 1 queue 1

# Assign port 0 queue 1 to thread 2
set thread 2 port 0 queue 1
```

### show_thread_qconf - Display Thread Queue Configuration
```
show thread qconf
```

Display current thread queue configuration.

**Examples:**
```bash
show thread qconf
```

## Queue System Overview

### DPDK Queue Concept
In DPDK, multiple transmit and receive queues can be configured for each port:

- **Receive Queue (RX Queue)** - Receives incoming packets
- **Transmit Queue (TX Queue)** - Sends outgoing packets
- **Multi-queue** - Parallel processing with multiple queues

### Importance of Queue Configuration
Proper queue configuration achieves the following:
- **Performance Improvement** - High-speed processing through parallelization
- **Load Distribution** - Processing distribution across multiple workers
- **CPU Efficiency** - Effective utilization of CPU cores

## Queue Configuration Methods

### Basic Configuration Steps
1. **Update Port Status**
```bash
update port status
```

2. **Set Thread Queue Configuration**
```bash
set thread 0 port 0 queue 0
set thread 1 port 0 queue 1
```

3. **Verify Configuration**
```bash
show thread qconf
```

### Recommended Configuration Patterns

#### Single Port, Single Queue
```bash
set thread 0 port 0 queue 0
```

#### Single Port, Multiple Queues
```bash
set thread 0 port 0 queue 0
set thread 1 port 0 queue 1
set thread 2 port 0 queue 2
```

#### Multiple Ports, Multiple Queues
```bash
set thread 0 port 0 queue 0
set thread 1 port 1 queue 0
set thread 2 port 2 queue 0
```

## Configuration Examples

### High-Performance Configuration (4 cores, 4 ports)
```bash
# Update port status
update port status

# Assign different ports to each core
set thread 0 port 0 queue 0
set thread 1 port 1 queue 0
set thread 2 port 2 queue 0
set thread 3 port 3 queue 0

# Verify configuration
show thread qconf
```

### Load Balancing Configuration (2 cores, 1 port)
```bash
# Update port status
update port status

# Configure multiple queues for one port
set thread 0 port 0 queue 0
set thread 1 port 0 queue 1

# Verify configuration
show thread qconf
```

## Performance Tuning

### Determining Queue Count
- **CPU Core Count** - Configure according to available CPU cores
- **Port Count** - Consider number of physical ports
- **Traffic Characteristics** - Consider expected traffic patterns

### Optimization Points
1. **CPU Affinity** - Proper placement considering CPU cores and queues
2. **Memory Placement** - Memory placement considering NUMA nodes
3. **Interrupt Processing** - Efficient interrupt handling

## Troubleshooting

### When Queue Configuration Is Not Applied
1. Update port status
```bash
update port status
```

2. Check worker status
```bash
show worker
```

3. Check port status
```bash
show port
```

### When Performance Does Not Improve
1. Check queue configuration
```bash
show thread qconf
```

2. Check thread load
```bash
show thread counter
```

3. Check port statistics
```bash
show port statistics
```

## Definition Location

These commands are defined in the following file:
- `sdplane/queue_config.c`

## Related Topics

- [Port Management & Statistics](port-management.md)
- [Worker & lcore Management](worker-management.md)
- [Thread Information](thread-information.md)