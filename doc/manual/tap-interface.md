# TAP Interface

**Language:** **English** | [Japanese](ja/tap-interface.md)

Commands for managing TAP interfaces.

## Command List

### set_tap_capture_ifname - Set TAP Capture Interface Name
```
set tap capture ifname <WORD>
```

Set the interface name used by TAP capture function.

**Parameters:**
- `<WORD>` - Interface name

**Examples:**
```bash
# Set tap0 interface
set tap capture ifname tap0

# Set tap1 interface
set tap capture ifname tap1
```

### set_tap_capture_persistent - Enable TAP Capture Persistence
```
set tap capture persistent
```

Enable TAP capture persistence.

**Examples:**
```bash
# Enable persistence
set tap capture persistent
```

### no_tap_capture_persistent - Disable TAP Capture Persistence
```
no tap capture persistent
```

Disable TAP capture persistence.

**Examples:**
```bash
# Disable persistence
no tap capture persistent
```

### unset_tap_capture_persistent - Remove TAP Capture Persistence Setting
```
unset tap capture persistent
```

Remove TAP capture persistence setting.

**Examples:**
```bash
# Remove persistence setting
unset tap capture persistent
```

## TAP Interface Overview

### What is TAP Interface
TAP (Network TAP) interface is a virtual network interface used for monitoring and testing network traffic.

### Main Functions
- **Packet Capture** - Capture network traffic
- **Packet Injection** - Inject test packets
- **Bridge Function** - Bridge between different networks
- **Monitoring Function** - Monitor and analyze traffic

### Uses in sdplane
- **Debugging** - Debug packet flow
- **Testing** - Test network functions
- **Monitoring** - Monitor traffic
- **Development** - Develop and test new features

## TAP Interface Configuration

### Basic Configuration Steps
1. **Create TAP Interface**
```bash
# Create TAP interface at system level
sudo ip tuntap add tap0 mode tap
sudo ip link set tap0 up
```

2. **Configure in sdplane**
```bash
# Set TAP capture interface name
set tap capture ifname tap0

# Enable persistence
set tap capture persistent
```

3. **Configure TAP Handler Worker**
```bash
# Configure TAP handler worker
set worker lcore 2 tap-handler
start worker lcore 2
```

### Configuration Examples

#### Basic TAP Configuration
```bash
# TAP interface configuration
set tap capture ifname tap0
set tap capture persistent

# Worker configuration
set worker lcore 2 tap-handler
start worker lcore 2

# Verify configuration
show worker
```

#### Multiple TAP Interfaces Configuration
```bash
# Configure multiple TAP interfaces
set tap capture ifname tap0
set tap capture ifname tap1

# Enable persistence
set tap capture persistent
```

## Persistence Function

### What is Persistence
When persistence function is enabled, TAP interface configuration is retained after system restart.

### Benefits of Persistence
- **Configuration Retention** - Configuration remains effective after restart
- **Automatic Recovery** - Automatic recovery from system failures
- **Operational Efficiency** - Reduced manual configuration

### Persistence Configuration
```bash
# Enable persistence
set tap capture persistent

# Disable persistence
no tap capture persistent

# Remove persistence setting
unset tap capture persistent
```

## Usage Examples

### Debug Use
```bash
# Configure debug TAP interface
set tap capture ifname debug-tap
set tap capture persistent

# Configure TAP handler worker
set worker lcore 3 tap-handler
start worker lcore 3

# Start packet capture
tcpdump -i debug-tap
```

### Test Use
```bash
# Configure test TAP interface
set tap capture ifname test-tap
set tap capture persistent

# Prepare for test packet injection
set worker lcore 4 tap-handler
start worker lcore 4
```

## Monitoring and Management

### TAP Interface Status Check
```bash
# Check worker status
show worker

# Check thread information
show thread

# System-level check
ip link show tap0
```

### Traffic Monitoring
```bash
# Monitor using tcpdump
tcpdump -i tap0

# Monitor using Wireshark
wireshark -i tap0
```

## Troubleshooting

### When TAP Interface Is Not Created
1. System-level check
```bash
# Check TAP interface existence
ip link show tap0

# Check permissions
sudo ip tuntap add tap0 mode tap
```

2. Check in sdplane
```bash
# Check configuration
show worker

# Check worker status
show thread
```

### When Packets Are Not Captured
1. Check interface status
```bash
ip link show tap0
```

2. Check worker status
```bash
show worker
```

3. Restart TAP handler
```bash
restart worker lcore 2
```

### When Persistence Does Not Function
1. Check persistence setting
```bash
# Check current configuration (use show commands)
show worker
```

2. Check system configuration
```bash
# Check system-level configuration
systemctl status sdplane
```

## Advanced Features

### VLAN Integration
TAP interfaces can be used in conjunction with VLAN functions:
```bash
# Integration with VLAN switch worker
set worker lcore 5 vlan-switch
start worker lcore 5
```

### Bridge Function
Use multiple TAP interfaces in bridge mode:
```bash
# Configure multiple TAP interfaces
set tap capture ifname tap0
set tap capture ifname tap1
```

## Definition Location

These commands are defined in the following file:
- `sdplane/tap_cmd.c`

## Related Topics

- [Worker & lcore Management](worker-management.md)
- [VTY & Shell Management](vty-shell.md)
- [Debug & Logging](debug-logging.md)