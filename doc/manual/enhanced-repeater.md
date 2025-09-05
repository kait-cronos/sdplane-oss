# Enhanced Repeater Configuration

The enhanced repeater provides advanced VLAN switching capabilities with TAP interfaces for L3 routing and packet capture. It supports virtual switch creation, DPDK port linking with VLAN tagging, router interfaces for kernel integration, and capture interfaces for monitoring.

## Virtual Switch Commands

### set vswitch
```
set vswitch <1-4094>
```
**Description**: Creates a virtual switch with specified VLAN ID

**Parameters**:
- `<1-4094>`: VLAN ID for the virtual switch

**Example**:
```
set vswitch 2031
set vswitch 2032
```

### delete vswitch
```
delete vswitch <0-3>
```
**Description**: Deletes a virtual switch by ID

**Parameters**:
- `<0-3>`: Virtual switch ID to delete

**Example**:
```
delete vswitch 0
```

### show vswitch_rib
```
show vswitch_rib
```
**Description**: Displays virtual switch RIB information including configuration and status

## Virtual Switch Link Commands

### set vswitch-link
```
set vswitch-link vswitch <0-3> port <0-7> tag <0-4094>
```
**Description**: Links a DPDK port to a virtual switch with VLAN tagging configuration

**Parameters**:
- `vswitch <0-3>`: Virtual switch ID (0-3)
- `port <0-7>`: DPDK port ID (0-7)  
- `tag <0-4094>`: VLAN tag ID (0: native/untagged, 1-4094: tagged VLAN)

**Examples**:
```
# Link port 0 to vswitch 0 with VLAN tag 2031
set vswitch-link vswitch 0 port 0 tag 2031

# Link port 0 to vswitch 1 with native/untagged
set vswitch-link vswitch 1 port 0 tag 0
```

### delete vswitch-link
```
delete vswitch-link <0-7>
```
**Description**: Deletes a virtual switch link by ID

**Parameters**:
- `<0-7>`: Virtual switch link ID

### show vswitch-link
```
show vswitch-link
```
**Description**: Displays all virtual switch link configurations

## Router Interface Commands

### set vswitch router-if
```
set vswitch <1-4094> router-if <WORD>
```
**Description**: Creates a router interface on specified virtual switch for L3 connectivity

**Parameters**:
- `<1-4094>`: VLAN ID of the virtual switch
- `<WORD>`: TAP interface name

**Examples**:
```
set vswitch 2031 router-if rif2031
set vswitch 2032 router-if rif2032
```

### no set vswitch router-if
```
no set vswitch <1-4094> router-if
```
**Description**: Deletes router interface from specified virtual switch

**Parameters**:
- `<1-4094>`: VLAN ID of the virtual switch

### show rib vswitch router-if
```
show rib vswitch router-if
```
**Description**: Displays router interface configurations including MAC addresses, IP addresses, and interface status

## Capture Interface Commands

### set vswitch capture-if
```
set vswitch <1-4094> capture-if <WORD>
```
**Description**: Creates a capture interface on specified virtual switch for packet monitoring

**Parameters**:
- `<1-4094>`: VLAN ID of the virtual switch
- `<WORD>`: TAP interface name

**Examples**:
```
set vswitch 2031 capture-if cif2031
set vswitch 2032 capture-if cif2032
```

### no set vswitch capture-if
```
no set vswitch <1-4094> capture-if
```
**Description**: Deletes capture interface from specified virtual switch

**Parameters**:
- `<1-4094>`: VLAN ID of the virtual switch

### show rib vswitch capture-if
```
show rib vswitch capture-if
```
**Description**: Displays capture interface configurations

## VLAN Processing Features

The enhanced repeater performs sophisticated VLAN processing:

- **VLAN Translation**: Modifies VLAN IDs based on vswitch-link configuration
- **VLAN Stripping**: Removes VLAN headers when tag is set to 0 (native)  
- **VLAN Insertion**: Adds VLAN headers when forwarding untagged packets to tagged ports
- **Split Horizon**: Prevents loops by not forwarding packets back to the receiving port

## Configuration Example

```bash
# Create virtual switches
set vswitch 2031
set vswitch 2032

# Link DPDK port to virtual switches with VLAN tags
set vswitch-link vswitch 0 port 0 tag 2031
set vswitch-link vswitch 1 port 0 tag 2032

# Create router interfaces for L3 processing
set vswitch 2031 router-if rif2031
set vswitch 2032 router-if rif2032

# Create capture interfaces for monitoring
set vswitch 2031 capture-if cif2031
set vswitch 2032 capture-if cif2032

# Configure worker to use enhanced-repeater
set worker lcore 1 enhanced-repeater
```

## Integration with TAP Interfaces

Router and capture interfaces create TAP interfaces that integrate with the Linux kernel networking stack:

- **Router Interfaces**: Allow L3 routing, IP addressing, and kernel network processing
- **Capture Interfaces**: Enable packet monitoring, analysis, and debugging
- **Ring Buffers**: Use DPDK rings for efficient packet transfer between data plane and kernel