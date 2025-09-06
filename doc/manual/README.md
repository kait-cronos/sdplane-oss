# sdplane-oss User's Guide

**Language:** **English** | [Japanese](ja/README.md) | [French](fr/README.md) | [Chinese](zh/README.md)

sdplane-oss is a high-performance DPDK-based software router. This user guide describes all commands and features of sdplane.

## Table of Contents

1. [Port Management & Statistics](port-management.md) - DPDK port management and statistics
2. [Worker & lcore Management](worker-management.md) - Worker threads and lcore management
3. [Debug & Logging](debug-logging.md) - Debug and logging functions
4. [VTY & Shell Management](vty-shell.md) - VTY and shell management
5. [System Information & Monitoring](system-monitoring.md) - System information and monitoring
6. [RIB & Routing](routing.md) - RIB and routing functions
7. [Queue Configuration](queue-configuration.md) - Queue configuration and management
8. [Packet Generation](packet-generation.md) - Packet generation using PKTGEN
9. [Thread Information](thread-information.md) - Thread information and monitoring
10. [TAP Interface](tap-interface.md) - TAP interface management
11. [lthread Management](lthread-management.md) - lthread management
12. [Device Management](device-management.md) - Device and driver management

## Basic Usage

### Connection

To connect to sdplane:

```bash
# Start sdplane
sudo ./sdplane/sdplane

# Connect to CLI from another terminal
telnet localhost 9882
```

### Help

You can use `?` to display help for any command:

```
sdplane# ?
sdplane# show ?
sdplane# set ?
```

### Basic Commands

- `show version` - Display version information
- `show port` - Display port information
- `show worker` - Display worker information
- `exit` - Exit from CLI

## Command Classification

sdplane has 79 commands defined, classified into the following 13 functional categories:

1. **Port Management & Statistics** (10 commands) - DPDK port control and statistics
2. **Worker & lcore Management** (6 commands) - Worker threads and lcore management
3. **Debug & Logging** (2 commands) - Debug and logging functions
4. **VTY & Shell Management** (4 commands) - VTY and shell control
5. **System Information & Monitoring** (10 commands) - System information and monitoring
6. **RIB & Routing** (1 command) - Routing information management
7. **Queue Configuration** (3 commands) - Queue configuration
8. **Packet Generation** (3 commands) - Packet generation using PKTGEN
9. **Thread Information** (2 commands) - Thread monitoring
10. **TAP Interface** (2 commands) - TAP interface management
11. **lthread Management** (3 commands) - lthread management
12. **Device Management** (2 commands) - Device and driver management

For detailed usage instructions, please refer to the documentation for each category.