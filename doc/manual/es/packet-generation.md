# Generación de Paquetes

**Idioma / Language:** [English](../en/packet-generation.md) | [日本語](../ja/packet-generation.md) | [Français](../fr/packet-generation.md) | [中文](../zh/packet-generation.md) | [Deutsch](../de/packet-generation.md) | [Italiano](../it/packet-generation.md) | [한국어](../ko/packet-generation.md) | [ไทย](../th/packet-generation.md) | **Español**

Comandos para la funcionalidad de generación de paquetes usando PKTGEN (Generador de Paquetes).

## Lista de Comandos

### show_pktgen - Mostrar Información PKTGEN
```
show pktgen
```

Muestra el estado actual y configuración de PKTGEN (Generador de Paquetes).

**Ejemplo de uso:**
```bash
show pktgen
```

Este comando muestra la siguiente información:
- Estado de inicialización de PKTGEN
- Parámetros de configuración actuales
- Tareas en ejecución
- Información estadística

### pktgen_init - Inicialización PKTGEN
```
pktgen init argv-list <0-7>
```

Inicializa PKTGEN usando el argv-list especificado.

**Parámetros:**
- `<0-7>` - Índice del argv-list a usar

**Ejemplos de uso:**
```bash
# Inicializar PKTGEN usando argv-list 0
pktgen init argv-list 0

# Inicializar PKTGEN usando argv-list 2
pktgen init argv-list 2
```

### pktgen_do_start - Inicio PKTGEN
```
pktgen do start (<0-7>|all)
```

Inicia la generación de paquetes en el puerto especificado.

**Objetivo:**
- `<0-7>` - Número de puerto específico
- `all` - Todos los puertos

**Ejemplos de uso:**
```bash
# Iniciar generación de paquetes en puerto 0
pktgen do start 0

# Iniciar generación de paquetes en todos los puertos
pktgen do start all
```

### pktgen_do_stop - Detención PKTGEN
```
pktgen do stop (<0-7>|all)
```

Detiene la generación de paquetes en el puerto especificado.

**Objetivo:**
- `<0-7>` - Número de puerto específico
- `all` - Todos los puertos

**Ejemplos de uso:**
```bash
# Detener generación de paquetes en puerto 1
pktgen do stop 1

# Detener generación de paquetes en todos los puertos
pktgen do stop all
```

## Resumen de PKTGEN

### Qué es PKTGEN
PKTGEN (Generador de Paquetes) es una herramienta de generación de paquetes para pruebas de red. Proporciona las siguientes funciones:

- **Generación de Paquetes de Alta Velocidad** - Generación de paquetes de alto rendimiento
- **Formatos de Paquetes Diversos** - Compatible con varios protocolos
- **Configuración Flexible** - Configuración detallada de paquetes posible
- **Funcionalidad Estadística** - Proporciona información estadística detallada

### Usos Principales
- **Pruebas de Rendimiento de Red** - Medición de rendimiento y latencia
- **Pruebas de Carga** - Pruebas de resistencia a carga del sistema
- **Pruebas Funcionales** - Verificación de funciones de red
- **Benchmarking** - Pruebas de comparación de rendimiento

## Configuración de PKTGEN

### Procedimiento de Configuración Básico
1. **Configurar argv-list**
```bash
# Configurar parámetros para PKTGEN
set argv-list 0 "-c 0x3 -n 4"
set argv-list 1 "--socket-mem 1024"
set argv-list 2 "--huge-dir /mnt/huge"
```

2. **Inicializar PKTGEN**
```bash
pktgen init argv-list 0
```

3. **Configurar Worker**
```bash
set worker lcore 1 pktgen
start worker lcore 1
```

4. **Iniciar Generación de Paquetes**
```bash
pktgen do start 0
```

### Parámetros de Configuración
Ejemplos de parámetros configurables en argv-list:

- **-c** - Máscara de CPU
- **-n** - Número de canales de memoria
- **--socket-mem** - Tamaño de memoria de socket
- **--huge-dir** - Directorio de hugepages
- **--file-prefix** - Prefijo de archivo

## Ejemplos de Uso

### Generación de Paquetes Básica
```bash
# Configuración
set argv-list 0 "-c 0x3 -n 4 --socket-mem 1024"

# Inicialización
pktgen init argv-list 0

# Configuración de worker
set worker lcore 1 pktgen
start worker lcore 1

# Inicio de generación de paquetes
pktgen do start 0

# Verificación de estado
show pktgen

# Detención de generación de paquetes
pktgen do stop 0
```

### Generación en Múltiples Puertos
```bash
# Iniciar en múltiples puertos
pktgen do start all

# Verificar estado
show pktgen

# Detener en múltiples puertos
pktgen do stop all
```

## Monitorización y Estadísticas

### Verificación de Información Estadística
```bash
# Mostrar estadísticas PKTGEN
show pktgen

# Mostrar estadísticas de puertos
show port statistics

# Mostrar estadísticas de workers
show worker
```

### Monitorización de Rendimiento
```bash
# Verificar PPS (Paquetes Por Segundo)
show port statistics pps

# Verificar número total de paquetes
show port statistics total

# Verificar bytes/segundo
show port statistics Bps
```

## Solución de Problemas

### Cuando PKTGEN No Se Inicia
1. Verificar estado de inicialización
```bash
show pktgen
```

2. Verificar estado de workers
```bash
show worker
```

3. Verificar estado de puertos
```bash
show port
```

### Cuando la Generación de Paquetes No Se Detiene
1. Detener explícitamente
```bash
pktgen do stop all
```

2. Reiniciar worker
```bash
restart worker lcore 1
```

### Cuando el Rendimiento es Bajo
1. Verificar uso de CPU
2. Verificar configuración de memoria
3. Verificar configuración de puertos

## Funciones Avanzadas

### Configuración de Formatos de Paquetes
PKTGEN puede generar varios formatos de paquetes:
- **Ethernet** - Tramas Ethernet básicas
- **IP** - Paquetes IPv4/IPv6
- **UDP/TCP** - Paquetes UDP/TCP
- **VLAN** - Paquetes con etiquetas VLAN

### Control de Carga
- **Control de Tasa** - Control de tasa de generación de paquetes
- **Control de Ráfaga** - Generación de paquetes en ráfaga
- **Control de Tamaño** - Control del tamaño de paquetes

## Ubicación de Definición

Estos comandos están definidos en los siguientes archivos:
- `sdplane/pktgen_cmd.c`

## Elementos Relacionados

- [Gestión de Workers y lcore](worker-lcore-thread-management.md)
- [Gestión de Puertos y Estadísticas](port-management.md)
- [Información del Sistema y Monitorización](system-monitoring.md)