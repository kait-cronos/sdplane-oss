# Gestión de Workers y lcore

**Idioma / Language:** [English](../worker-lcore-thread-management.md) | [日本語](../ja/worker-lcore-thread-management.md) | [Français](../fr/worker-lcore-thread-management.md) | [中文](../zh/worker-lcore-thread-management.md) | [Deutsch](../de/worker-lcore-thread-management.md) | [Italiano](../it/worker-lcore-thread-management.md) | [한국어](../ko/worker-lcore-thread-management.md) | [ไทย](../th/worker-lcore-thread-management.md) | **Español**

Comandos para gestionar hilos de trabajo DPDK y lcore.

## Lista de Comandos

### set_worker - Configuración de Tipo de Worker
```
set worker lcore <0-16> (|none|l2fwd|l3fwd|l3fwd-lpm|tap-handler|l2-repeater|enhanced-repeater|vlan-switch|pktgen|linkflap-generator)
```

Configura el tipo de worker para el lcore especificado.

**Tipos de Worker:**
- `none` - Sin worker
- `l2fwd` - Reenvío Layer 2
- `l3fwd` - Reenvío Layer 3
- `l3fwd-lpm` - Reenvío Layer 3 (LPM)
- `tap-handler` - Manejador de interfaz TAP
- `l2-repeater` - Repetidor Layer 2
- `enhanced-repeater` - Repetidor mejorado con conmutación VLAN e interfaz TAP
- `vlan-switch` - Conmutador VLAN
- `pktgen` - Generador de paquetes
- `linkflap-generator` - Generador de fluctuación de enlaces

**Ejemplos de uso:**
```bash
# Configurar worker de reenvío L2 en lcore 1
set worker lcore 1 l2fwd

# Configurar worker de repetidor mejorado en lcore 1
set worker lcore 1 enhanced-repeater

# Configurar sin worker en lcore 2
set worker lcore 2 none

# Configurar worker de reenvío L3 (LPM) en lcore 3
set worker lcore 3 l3fwd-lpm
```

### reset_worker - Reinicio de Worker
```
reset worker lcore <0-16>
```

Reinicia el worker del lcore especificado.

**Ejemplo de uso:**
```bash
# Reiniciar worker de lcore 2
reset worker lcore 2
```

### start_worker - Inicio de Worker
```
start worker lcore <0-16>
```

Inicia el worker del lcore especificado.

**Ejemplo de uso:**
```bash
# Iniciar worker de lcore 1
start worker lcore 1
```

### restart_worker - Reinicio de Worker
```
restart worker lcore <0-16>
```

Reinicia el worker del lcore especificado.

**Ejemplo de uso:**
```bash
# Reiniciar worker de lcore 4
restart worker lcore 4
```

### start_worker_all - Inicio de Worker (con opción all)
```
start worker lcore (<0-16>|all)
```

Inicia el worker del lcore especificado o todos los lcore.

**Ejemplos de uso:**
```bash
# Iniciar worker de lcore 1
start worker lcore 1

# Iniciar todos los workers
start worker lcore all
```

### stop_worker - Detención de Worker
```
stop worker lcore (<0-16>|all)
```

Detiene el worker del lcore especificado o todos los lcore.

**Ejemplos de uso:**
```bash
# Detener worker de lcore 1
stop worker lcore 1

# Detener todos los workers
stop worker lcore all
```

### reset_worker_all - Reinicio de Worker (con opción all)
```
reset worker lcore (<0-16>|all)
```

Reinicia el worker del lcore especificado o todos los lcore.

**Ejemplos de uso:**
```bash
# Reiniciar worker de lcore 2
reset worker lcore 2

# Reiniciar todos los workers
reset worker lcore all
```

### restart_worker_all - Reinicio de Worker (con opción all)
```
restart worker lcore (<0-16>|all)
```

Reinicia el worker del lcore especificado o todos los lcore.

**Ejemplos de uso:**
```bash
# Reiniciar worker de lcore 3
restart worker lcore 3

# Reiniciar todos los workers
restart worker lcore all
```

### show_worker - Mostrar Información de Workers
```
show worker
```

Muestra el estado y configuración actuales de los workers.

**Ejemplo de uso:**
```bash
show worker
```

### set_mempool - Configuración de Pool de Memoria
```
set mempool
```

Configura el pool de memoria DPDK.

**Ejemplo de uso:**
```bash
set mempool
```

### set_rte_eal_argv - Configuración de Argumentos de Línea de Comandos RTE EAL
```
set rte_eal argv <WORD> <WORD> <WORD> <WORD> <WORD> <WORD>
```

Configura los argumentos de línea de comandos utilizados para la inicialización de RTE EAL (Environment Abstraction Layer).

**Ejemplo de uso:**
```bash
# Configurar argumentos EAL
set rte_eal argv -c 0x1 -n 4 --socket-mem 1024,1024 --huge-dir /mnt/huge
```

### rte_eal_init - Inicialización RTE EAL
```
rte_eal_init
```

Inicializa RTE EAL (Environment Abstraction Layer).

**Ejemplo de uso:**
```bash
rte_eal_init
```

## Descripción de Tipos de Worker

### Reenvío L2 (l2fwd)
Worker que realiza reenvío de paquetes a nivel Layer 2. Reenvía paquetes basándose en direcciones MAC.

### Reenvío L3 (l3fwd)
Worker que realiza reenvío de paquetes a nivel Layer 3. Realiza enrutamiento basado en direcciones IP.

### Reenvío L3 LPM (l3fwd-lpm)
Worker de reenvío Layer 3 que utiliza Longest Prefix Matching (LPM).

### Manejador TAP (tap-handler)
Worker que realiza reenvío de paquetes entre interfaces TAP y puertos DPDK.

### Repetidor L2 (l2-repeater)
Worker que realiza duplicación/repetición de paquetes a nivel Layer 2.


### Conmutador VLAN (vlan-switch)
Worker de conmutación que proporciona funcionalidad VLAN (Virtual LAN).

### Generador de Paquetes (pktgen)
Worker que genera paquetes para pruebas.

### Generador de Fluctuación de Enlaces (linkflap-generator)
Worker para probar cambios de estado de enlaces de red.

## Ubicación de Definición

Estos comandos están definidos en los siguientes archivos:
- `sdplane/dpdk_lcore_cmd.c`

## Elementos Relacionados

- [Gestión de Puertos y Estadísticas](port-management.md)
- [Información de Hilos](worker-lcore-thread-management.md)
- [Gestión lthread](lthread-management.md)