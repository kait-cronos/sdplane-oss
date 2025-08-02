# NetTLP

**Language / 言語:** **English** | [Japanese](ja/nettlp.md)

Commands for managing Network TLP (Transaction Layer Protocol) functions.

## Command List

### nettlp_send_dma_write_read - NetTLP DMA Transmission
```
nettlp-send (dma-write|dma-read)
```

Execute DMA (Direct Memory Access) write or read using NetTLP.

**Commands:**
- `dma-write` - DMA write
- `dma-read` - DMA read

**Examples:**
```bash
# Execute DMA write
nettlp-send dma-write

# Execute DMA read
nettlp-send dma-read
```

### show_nettlp - Display NetTLP Information
```
show nettlp
```

Display current NetTLP configuration and status.

**Examples:**
```bash
show nettlp
```

### set_nettlp_ether_local_remote - Set NetTLP Ethernet Addresses
```
set nettlp ether (local-addr|remote-addr) <WORD>
```

Set local or remote Ethernet addresses for NetTLP.

**Parameters:**
- `local-addr` - Local Ethernet address
- `remote-addr` - Remote Ethernet address
- `<WORD>` - MAC address (e.g., 00:11:22:33:44:55)

**Examples:**
```bash
# Set local Ethernet address
set nettlp ether local-addr 00:11:22:33:44:55

# Set remote Ethernet address
set nettlp ether remote-addr 00:aa:bb:cc:dd:ee
```

### set_nettlp_ipv4_local_remote - Set NetTLP IPv4 Addresses
```
set nettlp ipv4 (local-addr|remote-addr) A.B.C.D
```

Set local or remote IPv4 addresses for NetTLP.

**Parameters:**
- `local-addr` - Local IP address
- `remote-addr` - Remote IP address
- `A.B.C.D` - IPv4 address

**Examples:**
```bash
# Set local IP address
set nettlp ipv4 local-addr 192.168.1.10

# Set remote IP address
set nettlp ipv4 remote-addr 192.168.1.20
```

### set_nettlp_bus_number - Set NetTLP Bus Number
```
set nettlp bus-number <0-65535> device-number <0-65535>
```

Set PCI bus number and device number for NetTLP.

**Parameters:**
- `<0-65535>` (bus-number) - PCI bus number
- `<0-65535>` (device-number) - PCI device number

**Examples:**
```bash
# Set bus number 1, device number 0
set nettlp bus-number 1 device-number 0

# Set bus number 2, device number 1
set nettlp bus-number 2 device-number 1
```

### set_nettlp_pci_tag - Set NetTLP PCI Tag
```
set nettlp pci-tag <0-255>
```

Set PCI tag for NetTLP.

**Parameters:**
- `<0-255>` - PCI tag value

**Examples:**
```bash
# Set PCI tag to 10
set nettlp pci-tag 10

# Set PCI tag to 255
set nettlp pci-tag 255
```

### set_nettlp_txportid - Set NetTLP Transmission Port ID
```
set nettlp tx-portid <0-128>
```

Set transmission port ID for NetTLP.

**Parameters:**
- `<0-128>` - Transmission port ID

**Examples:**
```bash
# Set transmission port ID to 0
set nettlp tx-portid 0

# Set transmission port ID to 1
set nettlp tx-portid 1
```

### set_nettlp_udp_port - Set NetTLP UDP Ports
```
set nettlp udp (src-port|dst-port) <0-65535>
```

Set UDP source or destination port for NetTLP.

**Parameters:**
- `src-port` - Source port
- `dst-port` - Destination port
- `<0-65535>` - Port number

**Examples:**
```bash
# Set source port to 16384
set nettlp udp src-port 16384

# Set destination port to 16385
set nettlp udp dst-port 16385
```

### set_nettlp_memory_addr - Set NetTLP Memory Address
```
set nettlp memory-address <WORD>
```

Set memory address for NetTLP.

**Parameters:**
- `<WORD>` - Memory address (hexadecimal)

**Examples:**
```bash
# Set memory address
set nettlp memory-address 0x1000000

# Set another memory address
set nettlp memory-address 0x2000000
```

### set_nettlp_payload_size - Set NetTLP Payload Size
```
set nettlp payload-size <0-4096>
```

Set payload size for NetTLP.

**Parameters:**
- `<0-4096>` - Payload size (bytes)

**Examples:**
```bash
# Set payload size to 1024 bytes
set nettlp payload-size 1024

# Set payload size to 64 bytes
set nettlp payload-size 64
```

### set_nettlp_max_payload_size - Set NetTLP Maximum Payload Size
```
set nettlp max-payload-size <0-4096>
```

Set maximum payload size for NetTLP.

**Parameters:**
- `<0-4096>` - Maximum payload size (bytes)

**Examples:**
```bash
# Set maximum payload size to 2048 bytes
set nettlp max-payload-size 2048

# Set maximum payload size to 4096 bytes
set nettlp max-payload-size 4096
```

### set_nettlp_payload_string - Set NetTLP Payload String
```
set nettlp payload-string <LINE>
```

Set payload string for NetTLP.

**Parameters:**
- `<LINE>` - Payload string

**Examples:**
```bash
# Set payload string
set nettlp payload-string "Hello NetTLP"

# Set another payload string
set nettlp payload-string "Test Data 123"
```

### set_nettlp_psmem_addr - Set NetTLP Pseudo Memory Address
```
set nettlp psmem-address <WORD>
```

Set pseudo memory address for NetTLP.

**Parameters:**
- `<WORD>` - Pseudo memory address (hexadecimal)

**Examples:**
```bash
# Set pseudo memory address
set nettlp psmem-address 0x10000000

# Set another pseudo memory address
set nettlp psmem-address 0x20000000
```

### set_nettlp_psmem_size - Set NetTLP Pseudo Memory Size
```
set nettlp psmem-size (<WORD>|256M)
```

Set pseudo memory size for NetTLP.

**Parameters:**
- `<WORD>` - Memory size (numeric value)
- `256M` - Preset value (256MB)

**Examples:**
```bash
# Set pseudo memory size to 256MB
set nettlp psmem-size 256M

# Set pseudo memory size to 1GB
set nettlp psmem-size 1024M
```

### show_nettlp_psmem - Display NetTLP Pseudo Memory
```
show nettlp psmem (|<WORD>)
```

Display NetTLP pseudo memory contents.

**Parameters:**
- `<WORD>` - Address to display (shows all if omitted)

**Examples:**
```bash
# Display entire pseudo memory
show nettlp psmem

# Display specific address
show nettlp psmem 0x1000
```

## NetTLP Overview

### What is NetTLP
NetTLP (Network Transaction Layer Protocol) is a technology for transferring PCIe TLP (Transaction Layer Protocol) over networks.

### Main Functions
- **Remote Memory Access** - Memory access over networks
- **DMA Transfer** - Direct Memory Access transfer
- **PCIe Emulation** - PCIe functionality over networks
- **High-Speed Data Transfer** - High-speed transfer with low latency

### Applications
- **Distributed Systems** - High-speed communication between systems
- **Virtualization Environments** - Virtual PCIe device implementation
- **Test Environments** - PCIe device emulation
- **Research and Development** - Research on new network architectures

## NetTLP Configuration

### Basic Configuration Steps
1. **Network Configuration**
```bash
# Ethernet address configuration
set nettlp ether local-addr 00:11:22:33:44:55
set nettlp ether remote-addr 00:aa:bb:cc:dd:ee

# IP address configuration
set nettlp ipv4 local-addr 192.168.1.10
set nettlp ipv4 remote-addr 192.168.1.20

# UDP port configuration
set nettlp udp src-port 16384
set nettlp udp dst-port 16385
```

2. **PCIe Configuration**
```bash
# Bus and device number configuration
set nettlp bus-number 1 device-number 0

# PCI tag configuration
set nettlp pci-tag 10

# Transmission port ID configuration
set nettlp tx-portid 0
```

3. **Memory Configuration**
```bash
# Memory address configuration
set nettlp memory-address 0x1000000

# Payload size configuration
set nettlp payload-size 1024
set nettlp max-payload-size 2048

# Pseudo memory configuration
set nettlp psmem-address 0x10000000
set nettlp psmem-size 256M
```

4. **Worker Configuration**
```bash
# Configure NetTLP worker
set worker lcore 3 nettlp-thread
start worker lcore 3
```

### Configuration Example

#### Basic NetTLP Configuration
```bash
# Network configuration
set nettlp ether local-addr 00:11:22:33:44:55
set nettlp ether remote-addr 00:aa:bb:cc:dd:ee
set nettlp ipv4 local-addr 192.168.1.10
set nettlp ipv4 remote-addr 192.168.1.20
set nettlp udp src-port 16384
set nettlp udp dst-port 16385

# PCIe configuration
set nettlp bus-number 1 device-number 0
set nettlp pci-tag 10
set nettlp tx-portid 0

# Memory configuration
set nettlp memory-address 0x1000000
set nettlp payload-size 1024
set nettlp max-payload-size 2048

# Pseudo memory configuration
set nettlp psmem-address 0x10000000
set nettlp psmem-size 256M

# Worker configuration
set worker lcore 3 nettlp-thread
start worker lcore 3

# Verify configuration
show nettlp
```

## Usage Examples

### DMA Transfer Execution
```bash
# DMA write
nettlp-send dma-write

# DMA read
nettlp-send dma-read

# Check results
show nettlp
```

### Pseudo Memory Operations
```bash
# Check pseudo memory contents
show nettlp psmem

# Check specific address
show nettlp psmem 0x1000

# Set payload string
set nettlp payload-string "Test Data"

# Transfer data with DMA write
nettlp-send dma-write
```

## Monitoring and Management

### NetTLP Status Check
```bash
# Display NetTLP configuration and status
show nettlp

# Display pseudo memory
show nettlp psmem

# Check worker status
show worker
```

### Performance Monitoring
```bash
# Check thread counters
show thread counter

# Check port statistics
show port statistics
```

## Troubleshooting

### When NetTLP Does Not Operate
1. Check configuration
```bash
show nettlp
```

2. Check worker status
```bash
show worker
```

3. Check network connection
```bash
show port
```

### When DMA Transfer Fails
1. Check memory address configuration
2. Check payload size configuration
3. Check network configuration

### When Pseudo Memory Cannot Be Accessed
1. Check address configuration
2. Check memory size configuration
3. Check permission settings

## Definition Location

These commands are defined in the following file:
- `sdplane/nettlp.c`

## Related Topics

- [Worker & lcore Management](worker-management.md)
- [Port Management & Statistics](port-management.md)
- [System Information & Monitoring](system-monitoring.md)