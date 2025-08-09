# System Information & Monitoring

**Language / 言語:** **English** | [Japanese](ja/system-monitoring.md)

Commands for providing system information and monitoring functions in sdplane.

## Command List

### show_version - Display Version
```
show version
```

Display sdplane version information.

**Examples:**
```bash
show version
```

### set_locale - Set Locale
```
set locale (C|C.utf8|en_US.utf8|POSIX)
```

Set system locale.

**Available Locales:**
- `C` - Standard C locale
- `C.utf8` - UTF-8 enabled C locale
- `en_US.utf8` - English UTF-8 locale
- `POSIX` - POSIX locale

**Examples:**
```bash
# Set to UTF-8 enabled C locale
set locale C.utf8

# Set to English UTF-8 locale
set locale en_US.utf8
```

### set_argv_list_1 - Set argv-list
```
set argv-list <0-7> <WORD>
```

Set command line argument list.

**Parameters:**
- `<0-7>` - Index (0-7)
- `<WORD>` - String to set

**Examples:**
```bash
# Set argument at index 0
set argv-list 0 "--verbose"

# Set argument at index 1
set argv-list 1 "--config"
```

### show_argv_list - Display argv-list
```
show argv-list
```

Display all configured command line argument list.

**Examples:**
```bash
# Display all argv-list
show argv-list
```

### show_argv_list_specific - Display Specific argv-list
```
show argv-list <0-7>
```

Display specific index argv-list.

**Examples:**
```bash
# Display specific index argv-list
show argv-list 0

# Display argv-list index 3
show argv-list 3
```

### show_loop_count - Display Loop Counter
```
show loop-count (console|vty-shell|l2fwd) (pps|total)
```

Display loop counters for each component.

**Components:**
- `console` - Console
- `vty-shell` - VTY shell
- `l2fwd` - L2 forwarding

**Statistics Types:**
- `pps` - Loops per second
- `total` - Total loop count

**Examples:**
```bash
# Display console PPS
show loop-count console pps

# Display L2 forwarding total loop count
show loop-count l2fwd total
```

### show_rcu - Display RCU Information
```
show rcu
```

Display RCU (Read-Copy-Update) information.

**Examples:**
```bash
show rcu
```

### show_fdb - Display FDB Information
```
show fdb
```

Display FDB (Forwarding Database) information.

**Examples:**
```bash
show fdb
```

### show_vswitch - Display vswitch Information
```
show vswitch
```

Display virtual switch information.

**Examples:**
```bash
show vswitch
```

### sleep_cmd - Sleep Command
```
sleep <0-300>
```

Sleep for specified number of seconds.

**Parameters:**
- `<0-300>` - Sleep time (seconds)

**Examples:**
```bash
# Sleep for 5 seconds
sleep 5

# Sleep for 30 seconds
sleep 30
```

### show_mempool - Display Memory Pool Information
```
show mempool
```

Display DPDK memory pool information.

**Examples:**
```bash
show mempool
```

## Monitoring Item Descriptions

### Version Information
- sdplane version
- Build information
- Dependent library versions

### Loop Counter
- Processing loop count for each component
- Used for performance monitoring
- Used for PPS (Packets Per Second) calculation

### RCU Information
- Read-Copy-Update mechanism status
- Synchronization processing status
- Memory management status

### FDB Information
- MAC address table status
- Learned MAC addresses
- Aging information

### vswitch Information
- Virtual switch configuration
- Port information
- VLAN configuration

### Memory Pool Information
- Available memory
- Used memory
- Memory pool statistics

## Monitoring Best Practices

### Regular Monitoring
```bash
# Basic monitoring commands
show version
show mempool
show vswitch
show rcu
```

### Performance Monitoring
```bash
# Performance monitoring with loop counters
show loop-count console pps
show loop-count l2fwd pps
```

### Troubleshooting
```bash
# System status check
show fdb
show vswitch
show mempool
```

## Definition Location

These commands are defined in the following file:
- `sdplane/sdplane.c`

## Related Topics

- [Port Management & Statistics](port-management.md)
- [Worker & lcore Management](worker-management.md)
- [Thread Information](thread-information.md)