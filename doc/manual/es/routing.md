# RIB y Enrutamiento

**Idioma / Language:** [English](../routing.md) | [日本語](../ja/routing.md) | [Français](../fr/routing.md) | [中文](../zh/routing.md) | [Deutsch](../de/routing.md) | [Italiano](../it/routing.md) | [한국어](../ko/routing.md) | [ไทย](../th/routing.md) | **Español**

Comandos para gestionar RIB (Routing Information Base) e información de recursos del sistema.

## Lista de Comandos

### show_rib - Mostrar Información RIB
```
show rib
```

Muestra información de RIB (Routing Information Base).

**Ejemplo de uso:**
```bash
show rib
```

Este comando muestra la siguiente información:
- Versión RIB y puntero de memoria
- Configuración de conmutador virtual y asignación VLAN
- Estado de puertos DPDK y configuración de colas
- Asignación de colas lcore-to-port
- Tablas de vecinos para reenvío L2/L3

## Resumen de RIB

### Qué es RIB
RIB (Routing Information Base) es una base de datos central que almacena recursos del sistema e información de red. En sdplane, gestiona la siguiente información:

- **Configuración de conmutador virtual** - Conmutación VLAN y asignación de puertos
- **Información de puertos DPDK** - Estado de enlaces, configuración de colas, información de funciones
- **Configuración de colas lcore** - Asignación de procesamiento de paquetes por núcleo de CPU
- **Tablas de vecinos** - Entradas de base de datos de reenvío L2/L3

### Estructura de RIB
RIB está compuesto por dos estructuras principales:

```c
struct rib {
    struct rib_info *rib_info;  // Puntero a datos reales
};

struct rib_info {
    uint32_t ver;                                    // Número de versión
    uint8_t vswitch_size;                           // Número de conmutadores virtuales
    uint8_t port_size;                              // Número de puertos DPDK
    uint8_t lcore_size;                             // Número de lcore
    struct vswitch_conf vswitch[MAX_VSWITCH];       // Configuración de conmutador virtual
    struct vswitch_link vswitch_link[MAX_VSWITCH_LINK]; // Enlaces de puertos VLAN
    struct port_conf port[MAX_ETH_PORTS];           // Configuración de puertos DPDK
    struct lcore_qconf lcore_qconf[RTE_MAX_LCORE];  // Asignación de colas lcore
    struct neigh_table neigh_tables[NEIGH_NR_TABLES]; // Tablas de vecinos/reenvío
};
```

## Cómo Leer la Información RIB

### Ítems de Visualización Básicos
- **Destination** - Dirección de red de destino
- **Netmask** - Máscara de red
- **Gateway** - Gateway (siguiente salto)
- **Interface** - Interfaz de salida
- **Metric** - Valor métrico de la ruta
- **Status** - Estado de la ruta

### Estados de Ruta
- **Active** - Ruta activa
- **Inactive** - Ruta inactiva
- **Pending** - Ruta en configuración
- **Invalid** - Ruta inválida

## Ejemplos de Uso

### Uso Básico
```bash
# Mostrar información RIB
show rib
```

### Interpretación de Salida de Ejemplo
```
rib information version: 21 (0x55555dd42010)
vswitches: 
dpdk ports: 
  dpdk_port[0]: 
    link: speed=1000Mbps duplex=full autoneg=on status=up
    nb_rxd=1024 nb_txd=1024
    queues: nrxq=1 ntxq=4
  dpdk_port[1]: 
    link: speed=0Mbps duplex=half autoneg=on status=down
    nb_rxd=1024 nb_txd=1024
    queues: nrxq=1 ntxq=4
  dpdk_port[2]: 
    link: speed=0Mbps duplex=half autoneg=off status=down
    nb_rxd=1024 nb_txd=1024
    queues: nrxq=1 ntxq=4
lcores: 
  lcore[0]: nrxq=0
  lcore[1]: nrxq=1
    rxq[0]: dpdk_port[0], queue_id=0
  lcore[2]: nrxq=1
    rxq[0]: dpdk_port[1], queue_id=0
  lcore[3]: nrxq=1
    rxq[0]: dpdk_port[2], queue_id=0
  lcore[4]: nrxq=0
  lcore[5]: nrxq=0
  lcore[6]: nrxq=0
  lcore[7]: nrxq=0
```

En este ejemplo:
- La versión RIB 21 muestra el estado actual del sistema
- El puerto DPDK 0 está activo (up) con velocidad de enlace de 1Gbps
- Los puertos DPDK 1, 2 están inactivos (down) sin enlace
- lcore 1, 2, 3 se encargan del procesamiento de paquetes de los puertos 0, 1, 2 respectivamente
- Cada puerto usa 1 cola RX y 4 colas TX
- Los anillos de descriptores RX/TX están configurados con 1024 entradas

## Gestión de RIB

### Actualización Automática
RIB se actualiza automáticamente en los siguientes momentos:
- Cambios de estado de interfaz
- Cambios de configuración de red
- Recepción de información de protocolos de enrutamiento

### Actualización Manual
Para verificar manualmente la información RIB:
```bash
show rib
```

## Solución de Problemas

### Cuando el Enrutamiento No Funciona Correctamente
1. Verificar información RIB
```bash
show rib
```

2. Verificar estado de interfaces
```bash
show port
```

3. Verificar estado de workers
```bash
show worker
```

### Cuando las Rutas No Aparecen en RIB
- Verificar configuración de red
- Verificar estado de interfaces
- Verificar operación del administrador RIB

## Funciones Avanzadas

### Administrador RIB
El administrador RIB opera como un hilo independiente y proporciona las siguientes funciones:
- Actualización automática de información de enrutamiento
- Verificación de validez de rutas
- Monitorización del estado de la red

### Workers Relacionados
- **rib-manager** - Worker que gestiona RIB
- **l3fwd** - Utiliza RIB en reenvío Layer 3
- **l3fwd-lpm** - Cooperación entre tabla LPM y RIB

## Ubicación de Definición

Este comando está definido en los siguientes archivos:
- `sdplane/rib.c`

## Elementos Relacionados

- [Gestión de Workers y lcore](worker-management.md)
- [Gestión lthread](lthread-management.md)
- [Información del Sistema y Monitorización](system-monitoring.md)