# Worker & lcore Management

**Language / 言語:** **English** | [Japanese](ja/worker-management.md)

Commands for managing DPDK worker threads and lcores.

## Command List

### set_worker - Set Worker Type
```
set worker lcore <0-16> (|none|l2fwd|l3fwd|l3fwd-lpm|tap-handler|l2-repeater|enhanced-repeater|vlan-switch|pktgen|linkflap-generator)
```

Set worker type on specified lcores.

**Worker Types:**
- `none` - No worker
- `l2fwd` - Layer 2 forwarding
- `l3fwd` - Layer 3 forwarding
- `l3fwd-lpm` - Layer 3 forwarding (LPM)
- `tap-handler` - TAP interface handler
- `l2-repeater` - Layer 2 repeater
- `enhanced-repeater` - Enhanced repeater with VLAN switching and TAP interfaces
- `vlan-switch` - VLAN switch
- `pktgen` - Packet generator
- `linkflap-generator` - Link flap generator

**Examples:**
```bash
# Set L2 forwarding worker on lcore 1
set worker lcore 1 l2fwd

# Set enhanced repeater worker on lcore 1
set worker lcore 1 enhanced-repeater

# Set no worker on lcore 2
set worker lcore 2 none

# Set L3 forwarding (LPM) worker on lcore 3
set worker lcore 3 l3fwd-lpm
```

### reset_worker - Reset Worker
```
reset worker lcore <0-16>
```

Reset worker on specified lcore.

**Examples:**
```bash
# Reset worker on lcore 2
reset worker lcore 2
```

### start_worker - Start Worker
```
start worker lcore <0-16>
```

Start worker on specified lcore.

**Examples:**
```bash
# Start worker on lcore 1
start worker lcore 1
```

### restart_worker - Restart Worker
```
restart worker lcore <0-16>
```

Restart worker on specified lcore.

**Examples:**
```bash
# Restart worker on lcore 4
restart worker lcore 4
```

### start_worker_all - Start Worker (with all option)
```
start worker lcore (<0-16>|all)
```

Start worker on specified lcore or all lcores.

**Examples:**
```bash
# Start worker on lcore 1
start worker lcore 1

# Start all workers
start worker lcore all
```

### stop_worker - Stop Worker
```
stop worker lcore (<0-16>|all)
```

Stop worker on specified lcore or all lcores.

**Examples:**
```bash
# Stop worker on lcore 1
stop worker lcore 1

# Stop all workers
stop worker lcore all
```

### reset_worker_all - Reset Worker (with all option)
```
reset worker lcore (<0-16>|all)
```

Reset worker on specified lcore or all lcores.

**Examples:**
```bash
# Reset worker on lcore 2
reset worker lcore 2

# Reset all workers
reset worker lcore all
```

### restart_worker_all - Restart Worker (with all option)
```
restart worker lcore (<0-16>|all)
```

Restart worker on specified lcore or all lcores.

**Examples:**
```bash
# Restart worker on lcore 3
restart worker lcore 3

# Restart all workers
restart worker lcore all
```

### show_worker - Display Worker Information
```
show worker
```

Display current worker status and configuration.

**Examples:**
```bash
show worker
```

### set_mempool - Set Memory Pool
```
set mempool
```

Configure DPDK memory pools.

**Examples:**
```bash
set mempool
```

### set_rte_eal_argv - Set RTE EAL Command Line Arguments
```
set rte_eal argv <WORD> <WORD> <WORD> <WORD> <WORD> <WORD>
```

Set command line arguments for RTE EAL (Environment Abstraction Layer) initialization.

**Examples:**
```bash
# Set EAL arguments
set rte_eal argv -c 0x1 -n 4 --socket-mem 1024,1024 --huge-dir /mnt/huge
```

### rte_eal_init - RTE EAL Initialization
```
rte_eal_init
```

Initialize RTE EAL (Environment Abstraction Layer).

**Examples:**
```bash
rte_eal_init
```

## Worker Type Descriptions

### L2 Forwarding (l2fwd)
Worker for packet forwarding at Layer 2 level. Forwards packets based on MAC addresses.

### L3 Forwarding (l3fwd)
Worker for packet forwarding at Layer 3 level. Performs routing based on IP addresses.

### L3 Forwarding LPM (l3fwd-lpm)
Layer 3 forwarding worker using Longest Prefix Matching (LPM).

### TAP Handler (tap-handler)
Worker for packet forwarding between TAP interfaces and DPDK ports.

### L2 Repeater (l2-repeater)
Worker for packet duplication and repeating at Layer 2 level.


### VLAN Switch (vlan-switch)
Switching worker that provides VLAN (Virtual LAN) functionality.

### Packet Generator (pktgen)
Worker for generating test packets.

### Link Flap Generator (linkflap-generator)
Worker for testing network link state changes.

## Definition Location

These commands are defined in the following file:
- `sdplane/dpdk_lcore_cmd.c`

## Related Topics

- [Port Management & Statistics](port-management.md)
- [Thread Information](thread-information.md)
- [lthread Management](lthread-management.md)