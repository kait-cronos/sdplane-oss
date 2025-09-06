# RIB & Routing

**Language:** **English** | [Japanese](ja/routing.md) | [French](fr/routing.md) | [Chinese](zh/routing.md) | [German](de/routing.md) | [Italian](it/routing.md) | [Korean](ko/routing.md) | [Thai](th/routing.md) | [Spanish](es/routing.md)

Commands for managing RIB (Routing Information Base) and system resource information.

## Command List

### show_rib - Display RIB Information
```
show rib
```

Display RIB (Routing Information Base) information.

**Examples:**
```bash
show rib
```

This command displays the following information:
- RIB version and memory pointer
- Virtual switch configurations and VLAN assignments
- DPDK port status and queue configurations  
- lcore-to-port queue assignments
- Neighbor tables for L2/L3 forwarding

## RIB Overview

### What is RIB
RIB (Routing Information Base) is a centralized database that stores system resource and networking information. In sdplane, it manages the following information:

- **Virtual Switch Configuration** - VLAN switching and port assignments
- **DPDK Port Information** - Link status, queue configuration, and capabilities
- **lcore Queue Configuration** - Packet processing assignments per CPU core
- **Neighbor Tables** - L2/L3 forwarding database entries

### RIB Structure
The RIB consists of two main structures:

```c
struct rib {
    struct rib_info *rib_info;  // Pointer to actual data
};

struct rib_info {
    uint32_t ver;                                    // Version number
    uint8_t vswitch_size;                           // Number of virtual switches
    uint8_t port_size;                              // Number of DPDK ports
    uint8_t lcore_size;                             // Number of lcores
    struct vswitch_conf vswitch[MAX_VSWITCH];       // Virtual switch config
    struct vswitch_link vswitch_link[MAX_VSWITCH_LINK]; // VLAN port links
    struct port_conf port[MAX_ETH_PORTS];           // DPDK port config
    struct lcore_qconf lcore_qconf[RTE_MAX_LCORE];  // lcore queue assignments
    struct neigh_table neigh_tables[NEIGH_NR_TABLES]; // Neighbor/forwarding tables
};
```

## Reading RIB Information

### Basic Display Items
- **RIB Version** - Current RIB version number and memory pointer
- **Virtual Switches** - VLAN configurations and port assignments
- **DPDK Ports** - Link status, speed, duplex, queue configurations
- **lcores** - CPU core assignments and packet processing queues
- **Neighbor Tables** - L2/L3 forwarding database entries

### DPDK Port Status
- **speed** - Link speed in Mbps
- **duplex** - full/half duplex mode
- **autoneg** - Auto-negotiation status (on/off)
- **status** - Link status (up/down)
- **nrxq/ntxq** - Number of RX/TX queues

## Usage Examples

### Basic Usage
```bash
# Display RIB information
show rib
```

### Output Example Interpretation
```
rib information version: 21 (0x55555dd42010)
vswitches: 
dpdk ports: 
  dpdk_port[0]: 
    link: speed=1000Mbps duplex=full autoneg=on status=up
    nb_rxd=1024 nb_txd=1024
    queues: nrxq=1 ntxq=4
  dpdk_port[1]: 
    link: speed=0Mbps duplex=half autoneg=on status=down
    nb_rxd=1024 nb_txd=1024
    queues: nrxq=1 ntxq=4
  dpdk_port[2]: 
    link: speed=0Mbps duplex=half autoneg=off status=down
    nb_rxd=1024 nb_txd=1024
    queues: nrxq=1 ntxq=4
lcores: 
  lcore[0]: nrxq=0
  lcore[1]: nrxq=1
    rxq[0]: dpdk_port[0], queue_id=0
  lcore[2]: nrxq=1
    rxq[0]: dpdk_port[1], queue_id=0
  lcore[3]: nrxq=1
    rxq[0]: dpdk_port[2], queue_id=0
  lcore[4]: nrxq=0
  lcore[5]: nrxq=0
  lcore[6]: nrxq=0
  lcore[7]: nrxq=0
```

In this example:
- RIB version 21 shows current system state
- DPDK port 0 is active (up) with 1Gbps link speed
- DPDK ports 1 and 2 are inactive (down) with no link
- lcore 1, 2, 3 are assigned to handle packets from ports 0, 1, 2 respectively
- Each port uses 1 RX queue and 4 TX queues
- RX/TX descriptor rings are configured with 1024 entries each

## RIB Management

### Automatic Updates
RIB is automatically updated at the following times:
- DPDK port link status changes
- Virtual switch configuration changes
- lcore queue assignment modifications
- Port start/stop operations

### Manual Updates
To manually refresh and check RIB information:
```bash
# Display current RIB state
show rib

# Update port status (useful for Mellanox NICs)
update port status
```

## Troubleshooting

### When Packet Processing Is Not Working Properly
1. Check RIB information for port and queue assignments
```bash
show rib
```

2. Check DPDK port status and link state
```bash
show port all
show port statistics all
```

3. Check worker and lcore assignments
```bash
show worker
show thread
```

### When Ports Are Not Shown in RIB
- Verify DPDK port initialization with `rte_eal_init`
- Check port configuration with `set port all dev-configure`
- Ensure ports are started with `start port all`
- For Mellanox NICs, run `update port status` command

## Advanced Features

### RIB Manager
The RIB manager operates as an independent thread and provides the following functions:
- Automatic DPDK port status monitoring
- Virtual switch configuration management
- lcore queue assignment coordination
- System resource state synchronization

### RCU-based Thread Safety
RIB uses RCU (Read-Copy-Update) for thread-safe access:
```c
// Access RIB from worker threads
#if HAVE_LIBURCU_QSBR
    urcu_qsbr_read_lock();
    rib = (struct rib *) rcu_dereference(rcu_global_ptr_rib);
#endif
    // Use rib data...
#if HAVE_LIBURCU_QSBR
    urcu_qsbr_read_unlock();
#endif
```

### Related Workers
- **rib-manager** - Worker that manages RIB updates and synchronization
- **enhanced-repeater** - Uses RIB for VLAN switching and port status
- **l2-repeater** - Uses RIB for basic packet forwarding decisions
- **l3-tap-handler** - Accesses RIB for TAP interface management

## Additional RIB Commands

### Virtual Switch Information
```bash
# Show virtual switch configurations
show vswitch_rib
show vswitch-link

# Show router and capture interfaces
show rib vswitch router-if
show rib vswitch capture-if
```

## Definition Location

This command is defined in the following files:
- `sdplane/rib.c` - Main RIB display functions
- `sdplane/rib.h` - RIB data structure definitions
- `sdplane/rib_manager.h` - RIB manager interface

## Related Topics

- [Enhanced Repeater](enhanced-repeater.md) - Virtual switch configuration
- [Port Management & Statistics](port-management.md) - DPDK port management  
- [Worker & lcore Management](worker-management.md) - Worker thread assignments
- [System Information & Monitoring](system-monitoring.md) - System monitoring commands