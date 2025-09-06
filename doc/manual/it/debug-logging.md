# Debug e Logging

**Language:** [English](../debug-logging.md) | [日本語](../ja/debug-logging.md) | [Français](../fr/debug-logging.md) | [中文](../zh/debug-logging.md) | [Deutsch](../de/debug-logging.md) | [Italiano](../it/debug-logging.md) | [한국어](../ko/debug-logging.md) | [ไทย](../th/debug-logging.md) | [Español](../es/debug-logging.md)

Comandi per controllare le funzioni di debug e logging di sdplane.

## Elenco Comandi

### log_file - Configurazione Output File Log
```
log file <file-path>
```

Configura il logging per l'output su file.

**Parametri:**
- <percorso-file> - Percorso del file di output del log

**Esempi:**
```bash
# Output dei log nel file specificato
log file /var/log/sdplane.log

# File di log di debug
log file /tmp/sdplane-debug.log
```

### log_stdout - Configurazione Log Output Standard
```
log stdout
```

Configura il logging per l'output su output standard.

**Esempi:**
```bash
# Visualizza i log sull'output standard
log stdout
```

**Nota:** `log file` e `log stdout` possono essere configurati simultaneamente, e i log saranno inviati ad entrambe le destinazioni.

### debug - Configurazione Debug
```
debug <category> <target>
```

Abilita il logging di debug per target specifici all'interno della categoria specificata.

**Categorie:**
- `sdplane` - Categoria principale sdplane
- `zcmdsh` - Categoria shell comandi

**Elenco Target sdplane:**
- `lthread` - Thread leggeri
- `console` - Console
- `tap-handler` - Gestore TAP
- `l2fwd` - Forwarding L2
- `l3fwd` - Forwarding L3
- `vty-server` - Server VTY
- `vty-shell` - Shell VTY
- `stat-collector` - Raccoglitore statistiche
- `packet` - Elaborazione pacchetti
- `fdb` - FDB (Database di Forwarding)
- `fdb-change` - Modifiche FDB
- `rib` - RIB (Base Informazioni Routing)
- `vswitch` - Switch virtuale
- `vlan-switch` - Switch VLAN
- `pktgen` - Generatore pacchetti
- `enhanced-repeater` - Ripetitore avanzato
- `netlink` - Interfaccia Netlink
- `neighbor` - Gestione vicini
- `all` - Tutti i target

**Esempi:**
```bash
# Abilita debug per target specifici
debug sdplane rib
debug sdplane fdb-change
debug sdplane pktgen

# Abilita tutto il debug sdplane
debug sdplane all

# Debug categoria zcmdsh
debug zcmdsh shell
debug zcmdsh command
```

### no debug - Disabilitazione Debug
```
no debug <category> <target>
```

Disabilita il logging di debug per target specifici all'interno della categoria specificata.

**Esempi:**
```bash
# Disabilita debug per target specifici
no debug sdplane rib
no debug sdplane fdb-change

# Disabilita tutto il debug sdplane (raccomandato)
no debug sdplane all

# Disabilita debug categoria zcmdsh
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
- Currently enabled debug targets
- Debug status for each target
- Available debug options

## Debug System Overview

The sdplane debug system has the following features:

### Category-based Debugging
- Debug categories are separated by different functional modules
- You can enable debug logs only for the necessary functions

### Target-based Control
- Debug messages are classified by target type
- You can display only necessary information by setting appropriate targets

### Dynamic Configuration
- Debug configuration can be changed while the system is running
- Debug targets can be adjusted without restart

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
