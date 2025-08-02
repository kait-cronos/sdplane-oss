# Device Management

**Language / 言語:** **English** | [Japanese](ja/device-management.md)

Commands for managing DPDK devices and drivers.

## Command List

### show_devices - Display Device Information
```
show devices
```

Display information about devices available in the system.

**Examples:**
```bash
show devices
```

This command displays the following information:
- Device name
- Device type
- Current driver
- Device status
- PCI address

### set_device_driver - Set Device Driver
```
set device <WORD> driver (ixgbe|igb|igc|uio_pci_generic|igb_uio|vfio-pci|unbound) (|bind|driver_override)
```

Set driver for specified device.

**Parameters:**
- `<WORD>` - Device name or PCI address
- Driver types:
  - `ixgbe` - Intel 10GbE ixgbe driver
  - `igb` - Intel 1GbE igb driver
  - `igc` - Intel 2.5GbE igc driver
  - `uio_pci_generic` - Generic UIO driver
  - `igb_uio` - DPDK UIO driver
  - `vfio-pci` - VFIO PCI driver
  - `unbound` - Remove driver
- Operation modes:
  - `bind` - Bind driver
  - `driver_override` - Driver override

**Examples:**
```bash
# Bind device to vfio-pci driver
set device 0000:01:00.0 driver vfio-pci bind

# Set device to igb_uio driver
set device eth0 driver igb_uio

# Remove driver from device
set device 0000:01:00.0 driver unbound
```

## Device Management Overview

### DPDK Device Management
DPDK uses dedicated drivers for efficient use of network devices.

### Driver Types

#### Network Drivers
- **ixgbe** - For Intel 10GbE network cards
- **igb** - For Intel 1GbE network cards
- **igc** - For Intel 2.5GbE network cards

#### UIO (Userspace I/O) Drivers
- **uio_pci_generic** - Generic UIO driver
- **igb_uio** - DPDK-specific UIO driver

#### VFIO (Virtual Function I/O) Drivers
- **vfio-pci** - High-performance I/O in virtualization environments

#### Special Settings
- **unbound** - Remove driver and disable device

## Device Configuration Procedures

### Basic Configuration Steps
1. **Check Devices**
```bash
show devices
```

2. **Set Driver**
```bash
set device <device> driver <driver> bind
```

3. **Verify Configuration**
```bash
show devices
```

4. **Configure Ports**
```bash
show port
```

### Configuration Examples

#### Intel 10GbE Card Configuration
```bash
# Check devices
show devices

# Bind ixgbe driver
set device 0000:01:00.0 driver ixgbe bind
set device 0000:01:00.1 driver ixgbe bind

# Verify configuration
show devices
show port
```

#### DPDK UIO Driver Configuration
```bash
# Check devices
show devices

# Bind igb_uio driver
set device 0000:02:00.0 driver igb_uio bind
set device 0000:02:00.1 driver igb_uio bind

# Verify configuration
show devices
show port
```

#### VFIO Configuration (Virtualization Environment)
```bash
# Check devices
show devices

# Bind vfio-pci driver
set device 0000:03:00.0 driver vfio-pci bind
set device 0000:03:00.1 driver vfio-pci bind

# Verify configuration
show devices
show port
```

## Driver Selection Guidelines

### ixgbe (Intel 10GbE)
- **Use**: Intel 10GbE network cards
- **Benefits**: High performance, stability
- **Requirements**: Intel 10GbE card required

### igb (Intel 1GbE)
- **Use**: Intel 1GbE network cards
- **Benefits**: Wide compatibility, stability
- **Requirements**: Intel 1GbE card required

### igc (Intel 2.5GbE)
- **Use**: Intel 2.5GbE network cards
- **Benefits**: Intermediate performance, new standard
- **Requirements**: Intel 2.5GbE card required

### uio_pci_generic
- **Use**: Generic devices
- **Benefits**: Wide compatibility
- **Limitations**: Some feature restrictions

### igb_uio
- **Use**: DPDK-specific environments
- **Benefits**: DPDK optimization
- **Limitations**: Separate installation required

### vfio-pci
- **Use**: Virtualization environments, security-focused
- **Benefits**: Security, virtualization support
- **Requirements**: IOMMU enablement required

## Troubleshooting

### When Device Is Not Recognized
1. Check devices
```bash
show devices
```

2. System-level check
```bash
lspci | grep Ethernet
```

3. Check kernel modules
```bash
lsmod | grep uio
lsmod | grep vfio
```

### When Driver Bind Fails
1. Check current driver
```bash
show devices
```

2. Remove existing driver
```bash
set device <device> driver unbound
```

3. Bind target driver
```bash
set device <device> driver <target_driver> bind
```

### When Port Is Not Available
1. Check device status
```bash
show devices
show port
```

2. Re-bind driver
```bash
set device <device> driver unbound
set device <device> driver <driver> bind
```

3. Check port configuration
```bash
show port
update port status
```

## Advanced Features

### Driver Override
```bash
# Use driver override
set device <device> driver <driver> driver_override
```

### Batch Configuration of Multiple Devices
```bash
# Configure multiple devices sequentially
set device 0000:01:00.0 driver vfio-pci bind
set device 0000:01:00.1 driver vfio-pci bind
set device 0000:02:00.0 driver vfio-pci bind
set device 0000:02:00.1 driver vfio-pci bind
```

## Security Considerations

### VFIO Usage Considerations
- IOMMU enablement required
- Security group configuration
- Appropriate permission settings

### UIO Usage Considerations
- Root privileges required
- Understanding security risks
- Appropriate access control

## System Integration

### systemd Service Integration
```bash
# Automatic configuration with systemd service
# Configure in /etc/systemd/system/sdplane.service
```

### Automatic Configuration at Startup
```bash
# Configuration in startup scripts
# /etc/init.d/sdplane or systemd unit file
```

## Definition Location

These commands are defined in the following file:
- `sdplane/dpdk_devbind.c`

## Related Topics

- [Port Management & Statistics](port-management.md)
- [Worker & lcore Management](worker-management.md)
- [System Information & Monitoring](system-monitoring.md)