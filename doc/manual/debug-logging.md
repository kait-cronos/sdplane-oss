# Debug & Logging

**Language:** **English** | [Japanese](ja/debug-logging.md) | [French](fr/debug-logging.md) | [Chinese](zh/debug-logging.md)

Commands for controlling sdplane debug and logging functions.

## Command List

### debug_sdplane - sdplane Debug Configuration
```
debug sdplane [category] [level]
```

Configure sdplane debug logging. This command is dynamically generated, so available categories and levels are determined at runtime.

**Examples:**
```bash
# Enable debug configuration
debug sdplane

# Enable debug for specific category
debug sdplane [category] [level]
```

**Note:** Specific categories and levels can be checked using the `show debugging sdplane` command.

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

### 1. Check Current Debug Configuration
```bash
show debugging sdplane
```

### 2. Check Debug Categories
Use the `show debugging sdplane` command to check available categories.

### 3. Change Debug Configuration
```bash
# Enable debug for specific category
debug sdplane [category] [level]
```

### 4. Check Debug Logs
Debug logs are output to standard output or log files.

## Troubleshooting

### When Debug Logs Are Not Output
1. Check if debug category is correctly configured
2. Check if debug level is appropriately set
3. Check if log output destination is correctly configured

### Performance Impact
- Enabling debug logs may impact performance
- It is recommended to enable only minimal debugging in production environments

## Definition Location

These commands are defined in the following file:
- `sdplane/debug_sdplane.c`

## Related Topics

- [System Information & Monitoring](system-monitoring.md)
- [VTY & Shell Management](vty-shell.md)