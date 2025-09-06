# DPDK Application Integration Guide

**Language:** **English** | [Japanese](ja/dpdk-integration-guide.md)

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
- Adapt to sdplane's cooperative threading model using lthread

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
int
my_worker_function(__rte_unused void *dummy)
{
    unsigned lcore_id = rte_lcore_id();
    
    while (!force_quit && !force_stop[lcore_id]) {
        // Your packet processing logic here
        
        // Cooperative yield for lthread compatibility
        rte_delay_us(1);
    }
    
    return 0;
}
```

### 3. Add CLI Commands
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
- Understand sdplane's cooperative threading
- Avoid blocking operations in worker functions
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
    
    // 4. Cooperative yield
    rte_delay_us(1);
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