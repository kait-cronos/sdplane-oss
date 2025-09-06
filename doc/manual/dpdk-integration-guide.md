# DPDK Application Integration Guide

**Language:** **English** | [Japanese](ja/dpdk-integration-guide.md) | [French](fr/dpdk-integration-guide.md) | [Chinese](zh/dpdk-integration-guide.md) | [German](de/dpdk-integration-guide.md) | [Italian](it/dpdk-integration-guide.md) | [Korean](ko/dpdk-integration-guide.md) | [Thai](th/dpdk-integration-guide.md) | [Spanish](es/dpdk-integration-guide.md)

This guide explains how to integrate existing DPDK applications and their pthread-based worker threads into the sdplane framework using the DPDK-dock approach.

## Overview

sdplane provides a unified framework for running multiple DPDK applications within a single process space using the **DPDK-dock approach**. Instead of each DPDK application initializing its own EAL environment, applications can be integrated as worker modules within sdplane's cooperative threading model.

The DPDK-dock method allows multiple DPDK applications to coexist and share resources efficiently by:
- Centralizing DPDK EAL initialization
- Providing shared memory pool management
- Unifying port and queue configuration
- Enabling cooperative multitasking between different DPDK workloads

## Key Integration Points

### Worker Thread Conversion
- Convert traditional DPDK pthread workers to sdplane lcore workers
- Replace `pthread_create()` with sdplane's `set worker lcore <id> <worker-type>`
- Integrate with sdplane's lcore-based threading model

### Initialization Integration 
- Remove application-specific `rte_eal_init()` calls
- Use sdplane's centralized EAL initialization
- Register application-specific CLI commands via sdplane's command framework

### Memory and Port Management
- Leverage sdplane's shared memory pool management
- Use sdplane's port configuration and queue assignment system
- Integrate with sdplane's RIB (Routing Information Base) for port status

## Integration Steps

### 1. Identify Worker Functions
Locate main packet processing loops in your DPDK application. These are typically functions that:
- Run in infinite loops processing packets
- Use `rte_eth_rx_burst()` and `rte_eth_tx_burst()`
- Handle packet forwarding or processing logic

### 2. Create Worker Module
Implement worker function following sdplane's worker interface:

```c
static __thread uint64_t loop_counter = 0;

int
my_worker_function(__rte_unused void *dummy)
{
    unsigned lcore_id = rte_lcore_id();
    int thread_id;
    
    // Register loop counter for monitoring
    thread_id = thread_lookup_by_lcore(my_worker_function, lcore_id);
    thread_register_loop_counter(thread_id, &loop_counter);
    
    while (!force_quit && !force_stop[lcore_id]) {
        // Your packet processing logic here
        
        // Increment loop counter for monitoring
        loop_counter++;
    }
    
    return 0;
}
```

### Worker Loop Counter Monitoring

The `loop_counter` variable enables monitoring of worker performance from the sdplane shell:

- **Thread-local variable**: Each worker maintains its own loop counter
- **Registration**: Counter is registered with sdplane's monitoring system using worker name and lcore ID
- **Increment**: Counter increases with each main loop iteration
- **Monitoring**: View counter values from sdplane CLI to verify worker activity

**CLI Monitoring Commands:**
```bash
# View thread counter information including loop counters
show thread counter

# View general thread information
show thread

# View worker configuration and status
show worker
```

This allows administrators to verify that workers are actively processing and detect potential performance issues or worker stalls by observing loop counter increments.

### 3. Accessing RIB Information with RCU

To access port information and configuration within DPDK packet processing workers, sdplane provides RIB (Routing Information Base) access through RCU (Read-Copy-Update) for thread-safe operations.

#### RIB Access Pattern

```c
#if HAVE_LIBURCU_QSBR
#include <urcu/urcu-qsbr.h>
#endif /*HAVE_LIBURCU_QSBR*/

static __thread struct rib *rib = NULL;

int
my_worker_function(__rte_unused void *dummy)
{
    unsigned lcore_id = rte_lcore_id();
    int thread_id;
    
    // Register loop counter for monitoring
    thread_id = thread_lookup_by_lcore(my_worker_function, lcore_id);
    thread_register_loop_counter(thread_id, &loop_counter);
    
#if HAVE_LIBURCU_QSBR
    urcu_qsbr_register_thread();
#endif /*HAVE_LIBURCU_QSBR*/

    while (!force_quit && !force_stop[lcore_id]) {
#if HAVE_LIBURCU_QSBR
        urcu_qsbr_read_lock();
        rib = (struct rib *) rcu_dereference(rcu_global_ptr_rib);
#endif /*HAVE_LIBURCU_QSBR*/

        // Your packet processing logic here
        // Access port information via rib->rib_info->port[portid]
        
#if HAVE_LIBURCU_QSBR
        urcu_qsbr_read_unlock();
        urcu_qsbr_quiescent_state();
#endif /*HAVE_LIBURCU_QSBR*/

        loop_counter++;
    }

#if HAVE_LIBURCU_QSBR
    urcu_qsbr_unregister_thread();
#endif /*HAVE_LIBURCU_QSBR*/
    
    return 0;
}
```

#### Accessing Port Information

Once RIB is obtained, access port-specific information:

```c
// Check port link status
if (!rib->rib_info->port[portid].link.link_status) {
    // Port is down, skip processing
    continue;
}

// Check if port is stopped
if (unlikely(rib->rib_info->port[portid].is_stopped)) {
    // Port is administratively stopped
    continue;
}

// Access port configuration
struct port_config *port_config = &rib->rib_info->port[portid];

// Get lcore queue configuration
struct lcore_qconf *lcore_qconf = &rib->rib_info->lcore_qconf[lcore_id];
for (i = 0; i < lcore_qconf->nrxq; i++) {
    portid = lcore_qconf->rx_queue_list[i].port_id;
    queueid = lcore_qconf->rx_queue_list[i].queue_id;
    // Process packets from this port/queue
}
```

#### RCU Safety Guidelines

- **Thread Registration**: Always register thread with `urcu_qsbr_register_thread()`
- **Read Lock**: Acquire read lock before accessing RIB data
- **Dereference**: Use `rcu_dereference()` to safely access RCU-protected pointers
- **Quiescent State**: Call `urcu_qsbr_quiescent_state()` to indicate safe points
- **Thread Cleanup**: Unregister thread with `urcu_qsbr_unregister_thread()`

#### RIB Data Structures

Key information available through RIB:
- **Port information**: Link status, configuration, statistics
- **Queue configuration**: Lcore to port/queue assignments
- **VLAN configuration**: Virtual switch and VLAN settings (for advanced features)
- **Interface configuration**: TAP interfaces and routing information

### 4. Add CLI Commands
Register application-specific commands in sdplane's CLI system:

```c
CLI_COMMAND2(my_app_command,
             "my-app command <WORD>",
             "my application\n",
             "command help\n")
{
    // Command implementation
    return 0;
}

// In initialization function
INSTALL_COMMAND2(cmdset, my_app_command);
```

### 4. Configure Build System
Update build configuration to include your module:

```makefile
# Add to sdplane/Makefile.am
sdplane_SOURCES += my_worker.c my_worker.h
```

### 5. Test Integration
Verify functionality using sdplane's configuration:

```bash
# In sdplane configuration
set worker lcore 1 my-worker-type
set thread 1 port 0 queue 0

# Start workers
start worker lcore all
```

## Successfully Integrated Applications

The following DPDK applications have been successfully integrated into sdplane using the DPDK-dock approach:

### L2FWD Integration (`module/l2fwd/`)
**Status**: ✅ Successfully integrated
- Original DPDK l2fwd application adapted for sdplane
- Demonstrates basic packet forwarding between ports
- **Reference**: Patch files available for original DPDK l2fwd source code
- Shows pthread-to-lcore worker conversion
- Key files: `module/l2fwd/l2fwd.c`, integration patches

### PKTGEN Integration (`module/pktgen/`)
**Status**: ✅ Successfully integrated  
- Full DPDK PKTGEN application integration
- Complex multi-threaded application example
- External library integration with sdplane CLI
- Demonstrates advanced integration techniques
- Key files: `module/pktgen/app/`, `sdplane/pktgen_cmd.c`

### L3FWD Integration
**Status**: ⚠️ Partially integrated (not fully functional)
- Integration attempted but requires additional work
- Not recommended as reference for new integrations
- Use L2FWD and PKTGEN as primary examples

## Recommended Integration References

For developers integrating new DPDK applications, we recommend using these as primary references:

### 1. L2FWD Integration (Recommended)
- **Location**: `module/l2fwd/`
- **Patch Files**: Available for original DPDK l2fwd source
- **Best for**: Understanding basic pthread-to-lcore conversion
- **Integration pattern**: Simple packet forwarding worker

### 2. PKTGEN Integration (Advanced Reference)  
- **Location**: `module/pktgen/` and `sdplane/pktgen_cmd.c`
- **Best for**: Complex application integration with CLI commands
- **Integration pattern**: Multi-component application with external dependencies

### Integration Patch Files

For L2FWD integration, patch files are available that show the exact modifications needed to adapt the original DPDK l2fwd application for sdplane integration. These patches demonstrate:
- EAL initialization removal
- Threading model adaptation
- CLI command integration  
- Resource management changes

Developers should refer to these patch files to understand the systematic approach to DPDK application integration.

## Custom Worker Examples

### L2 Repeater (`sdplane/l2_repeater.c`)
Custom sdplane worker (not based on existing DPDK app):
- Simple packet forwarding worker
- Broadcast forwarding to all active ports
- Integration with sdplane's port management

### Enhanced Repeater (`sdplane/enhanced_repeater.c`)
Custom sdplane worker with advanced features:
- VLAN-aware switching
- TAP interface integration
- RIB integration for port status

### VLAN Switch (`sdplane/vlan_switch.c`)
Custom Layer 2 switching implementation:
- MAC learning and forwarding
- VLAN processing
- Multi-port packet handling

## Best Practices

### Performance Considerations
- Use burst processing for optimal performance
- Minimize packet copying when possible
- Leverage DPDK's zero-copy mechanisms
- Consider NUMA topology in worker assignments

### Error Handling
- Check return values from DPDK functions
- Implement graceful shutdown handling
- Use appropriate logging levels
- Handle resource cleanup properly

### Threading Model
- Understand sdplane's lcore-based threading
- Design efficient packet processing loops
- Use appropriate synchronization mechanisms
- Consider thread affinity and CPU isolation

## Debugging and Monitoring

### Debug Logging
Enable debug logging for your worker:

```bash
debug sdplane my-worker-type
```

### Statistics Collection
Integrate with sdplane's statistics framework:

```c
// Update port statistics
port_statistics[portid].rx += nb_rx;
port_statistics[portid].tx += nb_tx;
```

### CLI Monitoring
Provide status commands for monitoring:

```bash
show my-worker status
show my-worker statistics
```

## Common Integration Patterns

### Packet Processing Pipeline
```c
// Typical packet processing in sdplane worker
while (!force_quit && !force_stop[lcore_id]) {
    // 1. Receive packets
    nb_rx = rte_eth_rx_burst(portid, queueid, pkts_burst, MAX_PKT_BURST);
    
    // 2. Process packets
    for (i = 0; i < nb_rx; i++) {
        // Your processing logic
        process_packet(pkts_burst[i]);
    }
    
    // 3. Transmit packets
    rte_eth_tx_burst(dst_port, queueid, pkts_burst, nb_rx);
}
```

### Configuration Integration
```c
// Register with sdplane's configuration system
struct worker_config my_worker_config = {
    .name = "my-worker",
    .worker_func = my_worker_function,
    .init_func = my_worker_init,
    .cleanup_func = my_worker_cleanup
};
```

## Troubleshooting

### Common Issues
- **Worker not starting**: Check lcore assignment and configuration
- **Packet drops**: Verify queue configuration and buffer sizes
- **Performance issues**: Review CPU affinity and NUMA settings
- **CLI commands not working**: Ensure proper command registration

### Debug Techniques
- Use sdplane's debug logging system
- Monitor worker statistics and counters  
- Check port link status and configuration
- Verify memory pool allocation

For detailed implementation examples, refer to the existing worker modules in the codebase and their corresponding CLI command definitions.