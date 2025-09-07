# VTY & Shell Management

**Language:** **English** | [Japanese](ja/vty-shell.md) | [French](fr/vty-shell.md) | [Chinese](zh/vty-shell.md) | [German](de/vty-shell.md) | [Italian](it/vty-shell.md) | [Korean](ko/vty-shell.md) | [Thai](th/vty-shell.md) | [Spanish](es/vty-shell.md)

Commands for managing VTY (Virtual Terminal) and shell.

## Command List

### clear_cmd - Clear Screen
```
clear
```

Clear the VTY screen.

**Examples:**
```bash
clear
```

### vty_exit_cmd - Exit VTY
```
(exit|quit|logout)
```

Exit the VTY session. Multiple aliases are available.

**Examples:**
```bash
exit
# or
quit
# or
logout
```

### shutdown_cmd - System Shutdown
```
shutdown
```

Shutdown the sdplane system.

**Examples:**
```bash
shutdown
```

**Note:** This command stops the entire system. Make sure to save configurations and terminate active sessions before execution.

### exit_cmd - Exit Console
```
(exit|quit)
```

Exit the console shell.

**Examples:**
```bash
exit
# or
quit
```

## Difference Between VTY and Console

### VTY Shell
- Remote shell accessed via Telnet
- Multiple sessions can be used simultaneously
- Network access is possible

### Console Shell
- Access from local console
- Direct system access
- Mainly for local management

## Connection Methods

### VTY Connection
```bash
telnet localhost 9882
```

### Console Connection
```bash
# Run sdplane directly
sudo ./sdplane/sdplane
```

## Session Management

### Session Status Check
VTY session status can be checked with the following command:
```bash
show worker
```

### Session Termination
- Normal termination with `exit`, `quit`, `logout` commands
- System-wide shutdown with `shutdown` command for abnormal termination

## Security Considerations

### Access Control
- VTY allows access only from localhost (127.0.0.1) by default
- It is recommended to restrict access to port 9882 with firewall settings

### Session Monitoring
- Properly terminate unnecessary sessions
- Regularly check long-idle sessions

## Troubleshooting

### Cannot Connect to VTY
1. Check if sdplane is running normally
2. Check if port 9882 is available
3. Check firewall settings

### Session Not Responding
1. Check status from another VTY session with `show worker`
2. Use `shutdown` command for system restart if necessary

### Commands Not Recognized
1. Check if you are in the correct shell (VTY or console)
2. Display available commands with `?`

## Definition Location

These commands are defined in the following files:
- `sdplane/vty_shell.c` - VTY-related commands
- `sdplane/console_shell.c` - Console-related commands

## Related Topics

- [Debug & Logging](debug-logging.md)
- [System Information & Monitoring](system-monitoring.md)