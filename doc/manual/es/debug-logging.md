# Debug y Logging

**Language:** [English](../debug-logging.md) | [日本語](../ja/debug-logging.md) | [Français](../fr/debug-logging.md) | [中文](../zh/debug-logging.md) | [Deutsch](../de/debug-logging.md) | [Italiano](../it/debug-logging.md) | [한국어](../ko/debug-logging.md) | [ไทย](../th/debug-logging.md) | [Español](../es/debug-logging.md)

Comandos para controlar las funciones de debug y logging de sdplane.

## Lista de Comandos

### log_file - Configuración de Salida de Archivo de Log
```
log file <file-path>
```

Configura el logging para salida a archivo.

**Parámetros:**
- <ruta-archivo> - Ruta al archivo de salida del log

**Ejemplos:**
```bash
# Salida de logs al archivo especificado
log file /var/log/sdplane.log

# Archivo de log de debug
log file /tmp/sdplane-debug.log
```

### log_stdout - Configuración de Log de Salida Estándar
```
log stdout
```

Configura el logging para salida a salida estándar.

**Ejemplos:**
```bash
# Mostrar logs en salida estándar
log stdout
```

**Nota:** `log file` y `log stdout` pueden configurarse simultáneamente, y los logs se enviarán a ambos destinos.

### debug - Configuración de Debug
```
debug <category> <target>
```

Habilita el logging de debug para objetivos específicos dentro de la categoría especificada.

**Categorías:**
- `sdplane` - Categoría principal sdplane
- `zcmdsh` - Categoría shell de comandos

**Lista de Objetivos sdplane:**
- `lthread` - Hilos ligeros
- `console` - Consola
- `tap-handler` - Manejador TAP
- `l2fwd` - Reenvío L2
- `l3fwd` - Reenvío L3
- `vty-server` - Servidor VTY
- `vty-shell` - Shell VTY
- `stat-collector` - Recolector de estadísticas
- `packet` - Procesamiento de paquetes
- `fdb` - FDB (Base de Datos de Reenvío)
- `fdb-change` - Cambios FDB
- `rib` - RIB (Base de Información de Enrutamiento)
- `vswitch` - Switch virtual
- `vlan-switch` - Switch VLAN
- `pktgen` - Generador de paquetes
- `enhanced-repeater` - Repetidor mejorado
- `netlink` - Interfaz Netlink
- `neighbor` - Gestión de vecinos
- `all` - Todos los objetivos

**Ejemplos:**
```bash
# Habilitar debug para objetivos específicos
debug sdplane rib
debug sdplane fdb-change
debug sdplane pktgen

# Habilitar todo el debug sdplane
debug sdplane all

# Debug de categoría zcmdsh
debug zcmdsh shell
debug zcmdsh command
```

### no debug - Deshabilitación de Debug
```
no debug <category> <target>
```

Deshabilita el logging de debug para objetivos específicos dentro de la categoría especificada.

**Ejemplos:**
```bash
# Deshabilitar debug para objetivos específicos
no debug sdplane rib
no debug sdplane fdb-change

# Deshabilitar todo el debug sdplane (recomendado)
no debug sdplane all

# Deshabilitar debug de categoría zcmdsh
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
