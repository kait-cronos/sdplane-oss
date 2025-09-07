# Debug & Logging

**Language:** **English** | [Japanese](ja/debug-logging.md) | [French](fr/debug-logging.md) | [Chinese](zh/debug-logging.md) | [German](de/debug-logging.md) | [Italian](it/debug-logging.md) | [Korean](ko/debug-logging.md) | [Thai](th/debug-logging.md) | [Spanish](es/debug-logging.md)

Commands for controlling sdplane debug and logging functions.

## Command List

### log_file - Log File Output Configuration
```
log file <file-path>
```

Configure logging to output to a file.

**Parameters:**
- `<file-path>` - Path to the log output file

**Examples:**
```bash
# Output logs to specified file
log file /var/log/sdplane.log

# Debug log file
log file /tmp/sdplane-debug.log
```

### log_stdout - Standard Output Log Configuration
```
log stdout
```

Configure logging to output to standard output.

**Examples:**
```bash
# Display logs to standard output
log stdout
```

**Note:** `log file` and `log stdout` can be configured simultaneously, and logs will be output to both destinations.

### debug - Debug Configuration
```
debug <category> <target>
```

Enable debug logging for specific targets within the specified category.

**Categories:**
- `sdplane` - Main sdplane category
- `zcmdsh` - Command shell category

**sdplane Target List:**
- `lthread` - Lightweight threads
- `console` - Console
- `tap-handler` - TAP handler
- `l2fwd` - L2 forwarding
- `l3fwd` - L3 forwarding
- `vty-server` - VTY server
- `vty-shell` - VTY shell
- `stat-collector` - Statistics collector
- `packet` - Packet processing
- `fdb` - FDB (Forwarding Database)
- `fdb-change` - FDB changes
- `rib` - RIB (Routing Information Base)
- `vswitch` - Virtual switch
- `vlan-switch` - VLAN switch
- `pktgen` - Packet generator
- `enhanced-repeater` - Enhanced repeater
- `netlink` - Netlink interface
- `neighbor` - Neighbor management
- `all` - All targets

**Examples:**
```bash
# Enable debug for specific targets
debug sdplane rib
debug sdplane fdb-change
debug sdplane pktgen

# Enable all sdplane debug
debug sdplane all

# zcmdsh category debug
debug zcmdsh shell
debug zcmdsh command
```

### no debug - Debug Disable
```
no debug <category> <target>
```

Disable debug logging for specific targets within the specified category.

**Examples:**
```bash
# Disable debug for specific targets
no debug sdplane rib
no debug sdplane fdb-change

# Disable all sdplane debug (recommended)
no debug sdplane all

# Disable zcmdsh category debug
no debug zcmdsh all
```

### show_debug_sdplane - Display sdplane Debug Information
```
show debugging sdplane
```

Display current sdplane debug configuration.

**Examples:**
```bash
show debugging sdplane
```

This command displays the following information:
- Currently enabled debug categories
- Debug level for each category
- Available debug options

## Debug System Overview

The sdplane debug system has the following features:

### Category-based Debugging
- Debug categories are separated by different functional modules
- You can enable debug logs only for the necessary functions

### Level-based Control
- Debug messages are classified by importance level
- You can display only necessary information by setting appropriate levels

### Dynamic Configuration
- Debug configuration can be changed while the system is running
- Debug levels can be adjusted without restart

## Usage

### 1. Configure Log Output
```bash
# Configure log file output (recommended)
log file /var/log/sdplane.log

# Configure standard output
log stdout

# Enable both (convenient for debugging)
log file /var/log/sdplane.log
log stdout
```

### 2. Check Current Debug Configuration
```bash
show debugging sdplane
```

### 3. Check Debug Targets
Use the `show debugging sdplane` command to check available targets and their status.

### 4. Change Debug Configuration
```bash
# Enable debug for specific targets
debug sdplane rib
debug sdplane fdb-change

# Enable all targets at once
debug sdplane all
```

### 5. Check Debug Logs
Debug logs are output to the configured destinations (file or standard output).

## Troubleshooting

### When Debug Logs Are Not Output
1. Check if log output is configured (`log file` or `log stdout`)
2. Check if debug targets are correctly configured (`debug sdplane <target>`)
3. Check current debug status (`show debugging sdplane`)
4. Check log file disk space and permissions

### Log File Management
```bash
# Check log file size
ls -lh /var/log/sdplane.log

# Tail log file
tail -f /var/log/sdplane.log

# Check log file location (configuration file example)
grep "log file" /etc/sdplane/sdplane.conf
```

### Performance Impact
- Enabling debug logs may impact performance
- It is recommended to enable only minimal debugging in production environments
- Regularly rotate log files to prevent them from becoming too large

## Configuration Examples

### Basic Log Configuration
```bash
# Configuration file example (/etc/sdplane/sdplane.conf)
log file /var/log/sdplane.log
log stdout

# Enable debug at system startup
debug sdplane rib
debug sdplane fdb-change
```

### Debugging Configuration
```bash
# Detailed debug log configuration
log file /tmp/sdplane-debug.log
log stdout

# Enable all target debugging (development only)
debug sdplane all

# Enable specific targets only
debug sdplane rib
debug sdplane fdb-change
debug sdplane vswitch
```

### Production Environment Configuration
```bash
# Standard logging only in production
log file /var/log/sdplane.log

# Enable only critical targets as needed
# debug sdplane fdb-change
# debug sdplane rib
```

### Debug Cleanup Operations
```bash
# Disable all debugging
no debug sdplane all
no debug zcmdsh all
```

## Definition Location

These commands are defined in the following file:
- `sdplane/debug_sdplane.c`

## Related Topics

- [System Information & Monitoring](system-monitoring.md)
- [VTY & Shell Management](vty-shell.md)