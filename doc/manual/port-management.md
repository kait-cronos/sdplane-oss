# Port Management & Statistics

**Language:** **English** | [Japanese](ja/port-management.md) | [French](fr/port-management.md) | [Chinese](zh/port-management.md)

Commands for managing DPDK ports and handling statistics.

## Command List

### **start port**

Start DPDK ports (default behavior).

**Examples:**
```bash
# Start port (default)
start port
```

---

### **start port \<0-16\>**

Start a specific DPDK port.

**Examples:**
```bash
# Start port 0
start port 0

# Start port 1
start port 1
```

---

### **start port all**

Start all DPDK ports.

**Examples:**
```bash
# Start all ports
start port all
```

---

### **stop port**

Stop DPDK ports (default behavior).

**Examples:**
```bash
# Stop port (default)
stop port
```

---

### **stop port \<0-16\>**

Stop a specific DPDK port.

**Examples:**
```bash
# Stop port 0
stop port 0

# Stop port 1
stop port 1
```

---

### **stop port all**

Stop all DPDK ports.

**Examples:**
```bash
# Stop all ports
stop port all
```

---

### **reset port**

Reset DPDK ports (default behavior).

**Examples:**
```bash
# Reset port (default)
reset port
```

---

### **reset port \<0-16\>**

Reset a specific DPDK port.

**Examples:**
```bash
# Reset port 0
reset port 0

# Reset port 1
reset port 1
```

---

### **reset port all**

Reset all DPDK ports.

**Examples:**
```bash
# Reset all ports
reset port all
```

---

### **show port**

Display basic information for all ports (default behavior).

**Examples:**
```bash
# Display information for all ports
show port
```

---

### **show port \<0-16\>**

Display basic information for a specific port.

**Examples:**
```bash
# Display information for port 0
show port 0

# Display information for port 1
show port 1
```

---

### **show port all**

Explicitly display information for all ports.

**Examples:**
```bash
# Explicitly display information for all ports
show port all
```

---

### **show port statistics**

Display all port statistics information.

**Examples:**
```bash
# Display all statistics
show port statistics
```

---

### **show port statistics pps**

Display packets per second statistics.

**Examples:**
```bash
# Display only PPS statistics
show port statistics pps
```

---

### **show port statistics total**

Display total packet count statistics.

**Examples:**
```bash
# Display total packet count
show port statistics total
```

---

### **show port statistics bps**

Display bits per second statistics.

**Examples:**
```bash
# Display bits per second
show port statistics bps
```

---

### **show port statistics Bps**

Display bytes per second statistics.

**Examples:**
```bash
# Display bytes per second
show port statistics Bps
```

---

### **show port statistics total-bytes**

Display total byte count statistics.

**Examples:**
```bash
# Display total bytes
show port statistics total-bytes
```

---

### **show port \<0-16\> promiscuous**

Display the promiscuous mode status for specified port.

**Examples:**
```bash
# Display promiscuous mode for port 0
show port 0 promiscuous

# Display promiscuous mode for port 1
show port 1 promiscuous
```

---

### **show port all promiscuous**

Display the promiscuous mode status for all ports.

**Examples:**
```bash
# Display promiscuous mode for all ports
show port all promiscuous
```

---

### **show port \<0-16\> flowcontrol**

Display flow control settings for specified port.

**Examples:**
```bash
# Display flow control settings for port 0
show port 0 flowcontrol

# Display flow control settings for port 1
show port 1 flowcontrol
```

---

### **show port all flowcontrol**

Display flow control settings for all ports.

**Examples:**
```bash
# Display flow control settings for all ports
show port all flowcontrol
```

---

### **set port \<0-16\> promiscuous enable**

Enable promiscuous mode for specified port.

**Examples:**
```bash
# Enable promiscuous mode for port 0
set port 0 promiscuous enable

# Enable promiscuous mode for port 1
set port 1 promiscuous enable
```

---

### **set port \<0-16\> promiscuous disable**

Disable promiscuous mode for specified port.

**Examples:**
```bash
# Disable promiscuous mode for port 0
set port 0 promiscuous disable

# Disable promiscuous mode for port 1
set port 1 promiscuous disable
```

---

### **set port all promiscuous enable**

Enable promiscuous mode for all ports.

**Examples:**
```bash
# Enable promiscuous mode for all ports
set port all promiscuous enable
```

---

### **set port all promiscuous disable**

Disable promiscuous mode for all ports.

**Examples:**
```bash
# Disable promiscuous mode for all ports
set port all promiscuous disable
```

---

### **set port \<0-16\> flowcontrol (rx|tx|autoneg|send-xon|fwd-mac-ctrl) (on|off)**

Configure flow control settings for specified port.

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

# Disable auto-negotiation for port 1
set port 1 flowcontrol autoneg off
```

---

### **set port all flowcontrol (rx|tx|autoneg|send-xon|fwd-mac-ctrl) (on|off)**

Configure flow control settings for all ports.

**Options:**
- `rx` - Receive flow control
- `tx` - Transmit flow control
- `autoneg` - Auto-negotiation
- `send-xon` - XON transmission
- `fwd-mac-ctrl` - MAC control frame forwarding

**Examples:**
```bash
# Disable auto-negotiation for all ports
set port all flowcontrol autoneg off

# Enable transmit flow control for all ports
set port all flowcontrol tx on
```

---

### **set port \<0-16\> dev-configure \<0-64\> \<0-64\>**

Configure DPDK port device settings for specified port.

**Parameters:**
- 1st argument: Number of receive queues (0-64)
- 2nd argument: Number of transmit queues (0-64)

**Examples:**
```bash
# Configure port 0 with 4 RX queues and 4 TX queues
set port 0 dev-configure 4 4

# Configure port 1 with 2 RX queues and 2 TX queues
set port 1 dev-configure 2 2
```

---

### **set port all dev-configure \<0-64\> \<0-64\>**

Configure DPDK port device settings for all ports.

**Parameters:**
- 1st argument: Number of receive queues (0-64)
- 2nd argument: Number of transmit queues (0-64)

**Examples:**
```bash
# Configure all ports with 1 RX queue and 1 TX queue
set port all dev-configure 1 1
```

---

### **set port \<0-16\> nrxdesc \<0-16384\>**

Set the number of receive descriptors for specified port.

**Examples:**
```bash
# Set receive descriptor count to 1024 for port 0
set port 0 nrxdesc 1024

# Set receive descriptor count to 512 for port 1
set port 1 nrxdesc 512
```

---

### **set port all nrxdesc \<0-16384\>**

Set the number of receive descriptors for all ports.

**Examples:**
```bash
# Set receive descriptor count to 512 for all ports
set port all nrxdesc 512
```

---

### **set port \<0-16\> ntxdesc \<0-16384\>**

Set the number of transmit descriptors for specified port.

**Examples:**
```bash
# Set transmit descriptor count to 1024 for port 0
set port 0 ntxdesc 1024

# Set transmit descriptor count to 512 for port 1
set port 1 ntxdesc 512
```

---

### **set port all ntxdesc \<0-16384\>**

Set the number of transmit descriptors for all ports.

**Examples:**
```bash
# Set transmit descriptor count to 512 for all ports
set port all ntxdesc 512
```

---

### **set port \<0-16\> link up**

Bring the link up for specified port.

**Examples:**
```bash
# Bring port 0 link up
set port 0 link up

# Bring port 1 link up
set port 1 link up
```

---

### **set port \<0-16\> link down**

Bring the link down for specified port.

**Examples:**
```bash
# Bring port 0 link down
set port 0 link down

# Bring port 1 link down
set port 1 link down
```

---

### **set port all link up**

Bring the link up for all ports.

**Examples:**
```bash
# Bring all ports link up
set port all link up
```

---

### **set port all link down**

Bring the link down for all ports.

**Examples:**
```bash
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