# Gestion des Workers & lcores & Informations de Thread

**Language:** [English](../en/worker-lcore-thread-management.md) | [日本語](../ja/worker-lcore-thread-management.md) | [Français](../fr/worker-lcore-thread-management.md) | [中文](../zh/worker-lcore-thread-management.md) | [Deutsch](../de/worker-lcore-thread-management.md) | [Italiano](../it/worker-lcore-thread-management.md) | [한국어](../ko/worker-lcore-thread-management.md) | [ไทย](../th/worker-lcore-thread-management.md) | [Español](../es/worker-lcore-thread-management.md)

Commandes pour la gestion et la surveillance des threads workers DPDK, des lcores et des informations de thread.

## Commandes de Gestion des Workers

### set_worker - Worker Type Configuration
```
set worker lcore <0-16> (|none|l2fwd|l3fwd|l3fwd-lpm|tap-handler|l2-repeater|enhanced-repeater|vlan-switch|pktgen|linkflap-generator)
```

Définit le type de worker pour le lcore spécifié.

**Types de Workers :**
- `none` - No worker
- `l2fwd` - Layer 2 forwarding
- `l3fwd` - Layer 3 forwarding
- `l3fwd-lpm` - Layer 3 forwarding (LPM)
- `tap-handler` - TAP interface handler
- `l2-repeater` - Layer 2 repeater
- `enhanced-repeater` - Enhanced repeater with VLAN switching and TAP interface
- `vlan-switch` - VLAN switch
- `pktgen` - Packet generator
- `linkflap-generator` - Link flap generator

**Exemples :**
```bash
# Set L2 forwarding worker to lcore 1
set worker lcore 1 l2fwd

# Set enhanced repeater worker to lcore 1
set worker lcore 1 enhanced-repeater

# Set no worker to lcore 2
set worker lcore 2 none
```

### reset_worker - Reset Worker
```
reset worker lcore (<0-16>|all)
```

Resets the worker for the specified lcore or all lcores.

### start_worker - Start Worker
```
start worker lcore (<0-16>|all)
```

Starts the worker for the specified lcore or all lcores.

### stop_worker - Stop Worker
```
stop worker lcore (<0-16>|all)
```

Stops the worker for the specified lcore or all lcores.

### restart_worker - Restart Worker
```
restart worker lcore (<0-16>|all)
```

Restarts the worker for the specified lcore or all lcores.

**Exemples :**
```bash
# Start worker for lcore 1
start worker lcore 1

# Stop all workers
stop worker lcore all

# Restart worker for lcore 3
restart worker lcore 3
```

### show_worker - Display Worker Information
```
show worker
```

Displays the current worker status and configuration.

## Commandes d'Information de Thread

### show_thread - Display Thread Information
```
show thread
```

Displays current thread status and information.

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

Displays thread counter information.

This command displays the following information:
- Processed packet count
- Execution count
- Error count
- Processing time statistics

## Commandes d'Initialisation Système

### set_mempool - Configure Memory Pool
```
set mempool
```

Configures DPDK memory pool.

### set_rte_eal_argv - Configure RTE EAL Command Line Arguments
```
set rte_eal argv <WORD> <WORD> <WORD> <WORD> <WORD> <WORD>
```

Configures command line arguments used for RTE EAL (Environment Abstraction Layer) initialization.

**Exemples :**
```bash
# Configure EAL arguments
set rte_eal argv -c 0x1 -n 4 --socket-mem 1024,1024 --huge-dir /mnt/huge
```

### rte_eal_init - RTE EAL Initialization
```
rte_eal_init
```

Initializes RTE EAL (Environment Abstraction Layer).

## Types de Workers et Architecture des Threads

### Détails des Types de Workers

#### L2 Forwarding (l2fwd)
Worker that performs packet forwarding at Layer 2 level. Forwards packets based on MAC addresses.

#### L3 Forwarding (l3fwd/l3fwd-lpm)
Worker that performs packet forwarding at Layer 3 level. Routes packets based on IP addresses. LPM uses Longest Prefix Matching.

#### TAP Handler (tap-handler)
Worker that performs packet transfer between TAP interfaces and DPDK ports.

#### Enhanced Repeater (enhanced-repeater)
High-functionality repeater worker with VLAN switching and TAP interface capabilities.

#### Packet Generator (pktgen)
Worker that generates packets for testing purposes.

### Architecture des Threads sdplane

sdplane operates with the following thread types in a cooperative multithreading model:

#### 1. Worker Threads (running on lcores)
- **L2/L3 Forwarding** - Packet forwarding processing
- **TAP Handler** - TAP interface processing
- **PKTGEN** - Packet generation

#### 2. Management Threads (lthreads)
- **RIB Manager** - Routing information management
- **Statistics Collector** - Statistics information collection
- **Netlink Thread** - Netlink communication processing

#### 3. System Threads
- **VTY Server** - VTY connection processing
- **Console** - Console input/output

### Relationship with lthread
sdplane uses a cooperative multithreading model:
- **lthread** - Lightweight thread implementation
- **Cooperative Scheduling** - Control through explicit yields
- **High Efficiency** - Reduces context switching overhead

## Comprendre les Informations de Thread

### Basic Display Items
- **Thread ID** - Thread identifier
- **Name** - Thread name
- **State** - Thread state (Running/Ready/Blocked/Terminated)
- **lcore** - Running CPU core
- **Type** - Thread type

### Counter Information
- **Packets** - Processed packet count
- **Loops** - Loop execution count
- **Errors** - Error count
- **CPU Time** - CPU usage time

## Exemples d'Usage et Workflows

### Basic Configuration and Worker Startup
```bash
# 1. Configure EAL initialization
set rte_eal argv -c 0xf -n 4

# 2. Execute EAL initialization
rte_eal_init

# 3. Configure memory pool
set mempool

# 4. Configure workers
set worker lcore 1 l2fwd
set worker lcore 2 l3fwd

# 5. Start workers
start worker lcore all

# 6. Check status
show worker
show thread
```

### Monitoring and Troubleshooting

#### Regular Monitoring
```bash
# Check worker and thread status
show worker
show thread
show thread counter
```

#### Performance Analysis
```bash
# Performance-related information
show thread counter
show loop-count l2fwd pps
```

#### Troubleshooting Procedures

**When threads are unresponsive**
```bash
# 1. Check thread status
show thread

# 2. Check worker status
show worker

# 3. Restart if necessary
restart worker lcore 1
```

**When performance degrades**
```bash
# 1. Check counter information
show thread counter

# 2. Check error count
show thread counter

# 3. Restart workers
restart worker lcore all
```

## Points d'Optimisation

### CPU Affinity Configuration
- Place workers on appropriate lcores
- Consider NUMA node placement
- Balance CPU usage

### Memory Efficiency Improvement
- Appropriate memory pool size
- Prevent memory leaks
- Improve cache efficiency

### Utilizing Statistics
- Performance monitoring
- Capacity planning
- Anomaly detection

## Emplacement de Définition

These commands are defined in the following files:
- `sdplane/dpdk_lcore_cmd.c` - Worker management commands
- `sdplane/thread_info.c` - Thread information commands

## Sujets Connexes

- [Port Management & Statistics](port-management.md)
- [lthread Management](lthread-management.md)
- [System Information & Monitoring](system-monitoring.md)
- [Debug & Logging](debug-logging.md)
