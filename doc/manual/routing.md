# RIB & Routing

**Language:** **English** | [Japanese](ja/routing.md)

Commands for managing RIB (Routing Information Base) and routing functions.

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
- Current routing table
- Status of each route
- Next hop information
- Metric information

## RIB Overview

### What is RIB
RIB (Routing Information Base) is a database that stores routing information. In sdplane, it manages the following information:

- **Routing Table** - Correspondence between IP destinations and next hops
- **Route Status** - Active, inactive, pending deletion, etc.
- **Metrics** - Route priority and cost
- **Interface Information** - Output interface information

### RIB Structure
```
Destination Network → Next Hop → Interface → Metric
```

## Reading RIB Information

### Basic Display Items
- **Destination** - Destination network address
- **Netmask** - Network mask
- **Gateway** - Gateway (next hop)
- **Interface** - Output interface
- **Metric** - Route metric value
- **Status** - Route status

### Route Status
- **Active** - Active route
- **Inactive** - Inactive route
- **Pending** - Route being configured
- **Invalid** - Invalid route

## Usage Examples

### Basic Usage
```bash
# Display RIB information
show rib
```

### Output Example Interpretation
```
Destination     Netmask         Gateway         Interface    Metric  Status
192.168.1.0     255.255.255.0   192.168.1.1     eth0         1       Active
10.0.0.0        255.0.0.0       10.0.0.1        eth1         1       Active
0.0.0.0         0.0.0.0         192.168.1.1     eth0         1       Active
```

In this example:
- 192.168.1.0/24 network is accessible through eth0 interface
- 10.0.0.0/8 network is accessible through eth1 interface
- Default route (0.0.0.0/0) goes to 192.168.1.1 via eth0

## RIB Management

### Automatic Updates
RIB is automatically updated at the following times:
- Interface status changes
- Network configuration changes
- Information received from routing protocols

### Manual Updates
To manually check RIB information:
```bash
show rib
```

## Troubleshooting

### When Routing Is Not Working Properly
1. Check RIB information
```bash
show rib
```

2. Check interface status
```bash
show port
```

3. Check worker status
```bash
show worker
```

### When Routes Are Not Displayed in RIB
- Check network configuration
- Check interface status
- Check RIB manager operation

## Advanced Features

### RIB Manager
The RIB manager operates as an independent thread and provides the following functions:
- Automatic routing information updates
- Route validity checking
- Network status monitoring

### Related Workers
- **rib-manager** - Worker that manages RIB
- **l3fwd** - Uses RIB in Layer 3 forwarding
- **l3fwd-lpm** - Coordination between LPM table and RIB

## Definition Location

This command is defined in the following file:
- `sdplane/rib.c`

## Related Topics

- [Worker & lcore Management](worker-management.md)
- [lthread Management](lthread-management.md)
- [System Information & Monitoring](system-monitoring.md)