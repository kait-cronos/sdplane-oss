# Port Management & Statistics

**Language / 言語:** **English** | [Japanese](ja/port-management.md)

Commands for managing DPDK ports and handling statistics.

## Command List

### start_stop_port - Start/Stop/Reset Ports
```
(start|stop|reset) port (|<0-16>|all)
```

Start, stop, or reset DPDK ports.

**Examples:**
```bash
# Start port 0
start port 0

# Stop all ports
stop port all

# Reset port 1
reset port 1
```

### show_port - Display Port Information
```
show port (|<0-16>|all)
```

Display basic information for specified ports.

**Examples:**
```bash
# Display information for all ports
show port

# Display information for port 0
show port 0

# Explicitly display information for all ports
show port all
```

### show_port_statistics - Display Port Statistics
```
show port statistics (|pps|total|bps|Bps|total-bytes)
```

Display port statistics information.

**Options:**
- `pps` - Packets per second
- `total` - Total packet count
- `bps` - Bits per second
- `Bps` - Bytes per second
- `total-bytes` - Total byte count

**Examples:**
```bash
# Display all statistics
show port statistics

# Display only PPS statistics
show port statistics pps

# Display total bytes
show port statistics total-bytes
```

### show_port_promiscuous - Display Promiscuous Mode
```
show port (<0-16>|all) promiscuous
```

Display the promiscuous mode status for specified ports.

**Examples:**
```bash
# Display promiscuous mode for port 0
show port 0 promiscuous

# Display promiscuous mode for all ports
show port all promiscuous
```

### show_port_flowcontrol - Display Flow Control Settings
```
show port (<0-16>|all) flowcontrol
```

Display flow control settings for specified ports.

**Examples:**
```bash
# Display flow control settings for port 0
show port 0 flowcontrol

# Display flow control settings for all ports
show port all flowcontrol
```

### set_port_promiscuous - Set Promiscuous Mode
```
set port (<0-16>|all) promiscuous (enable|disable)
```

Enable or disable promiscuous mode for specified ports.

**Examples:**
```bash
# Enable promiscuous mode for port 0
set port 0 promiscuous enable

# Disable promiscuous mode for all ports
set port all promiscuous disable
```

### set_port_flowcontrol - Set Flow Control
```
set port (<0-16>|all) flowcontrol (rx|tx|autoneg|send-xon|fwd-mac-ctrl) (on|off)
```

Configure flow control settings for specified ports.

**Options:**
- `rx` - Receive flow control
- `tx` - Transmit flow control
- `autoneg` - Auto-negotiation
- `send-xon` - XON transmission
- `fwd-mac-ctrl` - MAC control frame forwarding

**Examples:**
```bash
# Enable receive flow control for port 0
set port 0 flowcontrol rx on

# Disable auto-negotiation for all ports
set port all flowcontrol autoneg off
```

### set_port_dev_configure - Device Configuration
```
set port (<0-16>|all) dev-configure <0-64> <0-64>
```

Configure DPDK port device settings.

**Parameters:**
- 1st argument: Number of receive queues (0-64)
- 2nd argument: Number of transmit queues (0-64)

**Examples:**
```bash
# Configure port 0 with 4 RX queues and 4 TX queues
set port 0 dev-configure 4 4

# Configure all ports with 1 RX queue and 1 TX queue
set port all dev-configure 1 1
```

### set_port_txrx_desc - Set TX/RX Descriptor Count
```
set port (<0-16>|all) (nrxdesc|ntxdesc) <0-16384>
```

Set the number of transmit/receive descriptors.

**Options:**
- `nrxdesc` - Number of receive descriptors
- `ntxdesc` - Number of transmit descriptors

**Examples:**
```bash
# Set receive descriptor count to 1024 for port 0
set port 0 nrxdesc 1024

# Set transmit descriptor count to 512 for all ports
set port all ntxdesc 512
```

### set_port_link_updown - Set Link Up/Down
```
set port (<0-16>|all) link (up|down)
```

Force the link status up or down for specified ports.

**Examples:**
```bash
# Bring port 0 link up
set port 0 link up

# Bring all ports link down
set port all link down
```

## Definition Location

These commands are defined in the following file:
- `sdplane/dpdk_port_cmd.c`

## Related Topics

- [Worker & lcore Management](worker-management.md)
- [System Information & Monitoring](system-monitoring.md)
- [Queue Configuration](queue-configuration.md)