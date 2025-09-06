# Debug & Protokollierung

**Language:** [English](../debug-logging.md) | [日本語](../ja/debug-logging.md) | [Français](../fr/debug-logging.md) | [中文](../zh/debug-logging.md) | [Deutsch](../de/debug-logging.md) | [Italiano](../it/debug-logging.md) | [한국어](../ko/debug-logging.md) | [ไทย](../th/debug-logging.md) | [Español](../es/debug-logging.md)

Befehle zur Steuerung der sdplane Debug- und Protokollierungsfunktionen.

## Befehlsliste

### log_file - Protokolldatei-Ausgabekonfiguration
```
log file <file-path>
```

Konfiguriert die Protokollierung für die Ausgabe in eine Datei.

**Parameter:**
- <Dateipfad> - Pfad zur Protokollausgabedatei

**Beispiele:**
```bash
# Protokolle zur angegebenen Datei ausgeben
log file /var/log/sdplane.log

# Debug-Protokolldatei
log file /tmp/sdplane-debug.log
```

### log_stdout - Standard-Ausgabe Protokollkonfiguration
```
log stdout
```

Konfiguriert die Protokollierung für die Ausgabe zur Standardausgabe.

**Beispiele:**
```bash
# Protokolle auf Standardausgabe anzeigen
log stdout
```

**Hinweis:** `log file` und `log stdout` können gleichzeitig konfiguriert werden, und Protokolle werden an beide Ziele ausgegeben.

### debug - Debug-Konfiguration
```
debug <category> <target>
```

Aktiviert Debug-Protokollierung für spezifische Ziele innerhalb der angegebenen Kategorie.

**Kategorien:**
- `sdplane` - Haupt-sdplane-Kategorie
- `zcmdsh` - Befehlsshell-Kategorie

**sdplane Zielliste:**
- `lthread` - Leichtgewichtige Threads
- `console` - Konsole
- `tap-handler` - TAP-Handler
- `l2fwd` - L2-Weiterleitung
- `l3fwd` - L3-Weiterleitung
- `vty-server` - VTY-Server
- `vty-shell` - VTY-Shell
- `stat-collector` - Statistik-Sammler
- `packet` - Paketverarbeitung
- `fdb` - FDB (Weiterleitungsdatenbank)
- `fdb-change` - FDB-Änderungen
- `rib` - RIB (Routing-Informationsbasis)
- `vswitch` - Virtueller Switch
- `vlan-switch` - VLAN-Switch
- `pktgen` - Paketgenerator
- `enhanced-repeater` - Erweiterter Repeater
- `netlink` - Netlink-Schnittstelle
- `neighbor` - Nachbarverwaltung
- `all` - Alle Ziele

**Beispiele:**
```bash
# Debug für spezifische Ziele aktivieren
debug sdplane rib
debug sdplane fdb-change
debug sdplane pktgen

# Alle sdplane Debug aktivieren
debug sdplane all

# zcmdsh Kategorie Debug
debug zcmdsh shell
debug zcmdsh command
```

### no debug - Debug-Deaktivierung
```
no debug <category> <target>
```

Deaktiviert Debug-Protokollierung für spezifische Ziele innerhalb der angegebenen Kategorie.

**Beispiele:**
```bash
# Debug für spezifische Ziele deaktivieren
no debug sdplane rib
no debug sdplane fdb-change

# Alle sdplane Debug deaktivieren (empfohlen)
no debug sdplane all

# zcmdsh Kategorie Debug deaktivieren
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
