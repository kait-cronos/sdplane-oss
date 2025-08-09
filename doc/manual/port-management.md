# Port Management & Statistics

**Language / 言語:** **English** | [Japanese](ja/port-management.md)

Commands for managing DPDK ports and handling statistics.

## Command List

### start_port - Start Port
```
start port
```

Start DPDK ports (default behavior).

**Examples:**
```bash
# Start port (default)
start port
```

### start_port_specific - Start Specific Port
```
start port <0-16>
```

Start a specific DPDK port.

**Examples:**
```bash
# Start port 0
start port 0

# Start port 1
start port 1
```

### start_port_all - Start All Ports
```
start port all
```

Start all DPDK ports.

**Examples:**
```bash
# Start all ports
start port all
```

### stop_port - Stop Port
```
stop port
```

Stop DPDK ports (default behavior).

**Examples:**
```bash
# Stop port (default)
stop port
```

### stop_port_specific - Stop Specific Port
```
stop port <0-16>
```

Stop a specific DPDK port.

**Examples:**
```bash
# Stop port 0
stop port 0

# Stop port 1
stop port 1
```

### stop_port_all - Stop All Ports
```
stop port all
```

Stop all DPDK ports.

**Examples:**
```bash
# Stop all ports
stop port all
```

### reset_port - Reset Port
```
reset port
```

Reset DPDK ports (default behavior).

**Examples:**
```bash
# Reset port (default)
reset port
```

### reset_port_specific - Reset Specific Port
```
reset port <0-16>
```

Reset a specific DPDK port.

**Examples:**
```bash
# Reset port 0
reset port 0

# Reset port 1
reset port 1
```

### reset_port_all - Reset All Ports
```
reset port all
```

Reset all DPDK ports.

**Examples:**
```bash
# Reset all ports
reset port all
```

### show_port - Display Port Information
```
show port
```

Display basic information for all ports (default behavior).

**Examples:**
```bash
# Display information for all ports
show port
```

### show_port_specific - Display Specific Port Information
```
show port <0-16>
```

Display basic information for a specific port.

**Examples:**
```bash
# Display information for port 0
show port 0

# Display information for port 1
show port 1
```

### show_port_all - Display All Port Information
```
show port all
```

Explicitly display information for all ports.

**Examples:**
```bash
# Explicitly display information for all ports
show port all
```

### show_port_statistics - Display Port Statistics
```
show port statistics
```

Display all port statistics information.

**Examples:**
```bash
# Display all statistics
show port statistics
```

### show_port_statistics_pps - Display PPS Statistics
```
show port statistics pps
```

Display packets per second statistics.

**Examples:**
```bash
# Display only PPS statistics
show port statistics pps
```

### show_port_statistics_total - Display Total Packet Statistics
```
show port statistics total
```

Display total packet count statistics.

**Examples:**
```bash
# Display total packet count
show port statistics total
```

### show_port_statistics_bps - Display BPS Statistics
```
show port statistics bps
```

Display bits per second statistics.

**Examples:**
```bash
# Display bits per second
show port statistics bps
```

### show_port_statistics_Bps - Display Bytes Per Second Statistics
```
show port statistics Bps
```

Display bytes per second statistics.

**Examples:**
```bash
# Display bytes per second
show port statistics Bps
```

### show_port_statistics_total_bytes - Display Total Bytes Statistics
```
show port statistics total-bytes
```

Display total byte count statistics.

**Examples:**
```bash
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

### set_port_nrxdesc - Set RX Descriptor Count
```
set port (<0-16>|all) nrxdesc <0-16384>
```

Set the number of receive descriptors for specified ports.

**Examples:**
```bash
# Set receive descriptor count to 1024 for port 0
set port 0 nrxdesc 1024

# Set receive descriptor count to 512 for all ports
set port all nrxdesc 512
```

### set_port_ntxdesc - Set TX Descriptor Count
```
set port (<0-16>|all) ntxdesc <0-16384>
```

Set the number of transmit descriptors for specified ports.

**Examples:**
```bash
# Set transmit descriptor count to 1024 for port 0
set port 0 ntxdesc 1024

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