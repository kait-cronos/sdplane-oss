# Aplicación de Repetidor L2

**Idioma / Language:** [English](../en/l2-repeater-application.md) | [日本語](../ja/l2-repeater-application.md) | [Français](../fr/l2-repeater-application.md) | [中文](../zh/l2-repeater-application.md) | [Deutsch](../de/l2-repeater-application.md) | [Italiano](../it/l2-repeater-application.md) | [한국어](../ko/l2-repeater-application.md) | [ไทย](../th/l2-repeater-application.md) | **Español**

La aplicación de Repetidor L2 proporciona reenvío simple de paquetes Layer 2 entre puertos DPDK con funcionalidad básica de repetición puerto a puerto.

## Resumen

El Repetidor L2 es una aplicación intuitiva de reenvío Layer 2 con las siguientes capacidades:
- Reenvío de paquetes entre puertos DPDK emparejados (repetición simple puerto a puerto)
- Funcionalidad básica de repetición de paquetes sin aprendizaje de direcciones MAC
- Funcionalidad opcional de actualización de direcciones MAC (modificación de MAC fuente)
- Operación de alto rendimiento a través del procesamiento de paquetes copia cero de DPDK

## Características Clave

### Reenvío Layer 2
- **Repetición Puerto a Puerto**: Reenvío simple de paquetes entre pares de puertos preconfigurados
- **Sin Aprendizaje MAC**: Repetición directa de paquetes sin construir tablas de reenvío
- **Reenvío Transparente**: Reenvía todos los paquetes independientemente de la MAC de destino
- **Emparejamiento de Puertos**: Configuración de reenvío puerto a puerto fija

### Características de Rendimiento
- **Procesamiento Copia Cero**: Uso del procesamiento eficiente de paquetes de DPDK
- **Procesamiento en Ráfagas**: Procesamiento de paquetes en ráfagas para rendimiento óptimo
- **Baja Latencia**: Sobrecarga de procesamiento mínima para reenvío rápido
- **Soporte Multi-núcleo**: Ejecución en lcore dedicado para escalabilidad

## Configuración

### Configuración Básica
El Repetidor L2 se configura a través del sistema de configuración principal de sdplane:

```bash
# Configurar tipo de worker a Repetidor L2
set worker lcore 1 l2-repeater

# Configurar puertos y colas
set thread 1 port 0 queue 0  
set thread 1 port 1 queue 0

# Habilitar modo promiscuo para aprendizaje
set port all promiscuous enable
```

### Ejemplo de Archivo de Configuración
Para un ejemplo de configuración completa, consulte [`example-config/sdplane_l2_repeater.conf`](../../example-config/sdplane_l2_repeater.conf):

```bash
# Vinculación de dispositivos
set device 02:00.0 driver vfio-pci bind
set device 03:00.0 driver vfio-pci bind

# Inicialización DPDK
set rte_eal argv -c 0x7
rte_eal_init

# Workers de segundo plano (deben iniciarse antes de configurar colas)
set worker lthread stat-collector
set worker lthread rib-manager
set worker lthread netlink-thread

set mempool

# Configuración de colas (debe ejecutarse después de iniciar rib-manager)
set thread 1 port 0 queue 0
set thread 1 port 1 queue 0

# Configuración de puertos
stop port all
set port all dev-configure 1 4
set port all nrxdesc 1024
set port all ntxdesc 1024

# Configuración de workers
set worker lcore 1 l2-repeater
set worker lcore 2 tap-handler
set port all promiscuous enable
start port all

# Esperar arranque de puertos
sleep 5

# Iniciar workers
start worker lcore all
```

## Operación

### Reenvío de Difusión
El Repetidor L2 reenvía paquetes a todos los demás puertos activos:
- **Comportamiento de Difusión**: Reenvía todos los paquetes recibidos en un puerto a todos los demás puertos activos
- **Horizonte Dividido**: No devuelve paquetes al puerto de recepción
- **Sin Filtrado**: Reenvía todos los tipos de paquetes (unicast, broadcast, multicast)

### Comportamiento de Reenvío
- **Reenvío de Todo el Tráfico**: Repite todos los paquetes independientemente de la MAC de destino
- **Difusión a Todos los Puertos**: Reenvía paquetes a todos los puertos activos excepto el puerto de entrada
- **Transparente**: No modificación del contenido de paquetes (a menos que esté habilitada la actualización MAC)
- **Replicación Multi-puerto**: Crea copias de paquetes para cada puerto de destino

### Actualización de Dirección MAC
Cuando está habilitado, el Repetidor L2 puede modificar direcciones MAC de paquetes:
- **Actualización MAC Fuente**: Cambiar MAC fuente a la MAC del puerto de salida
- **Puenteo Transparente**: Mantener direcciones MAC originales (por defecto)

## Ajuste de Rendimiento

### Configuración de Búferes
```bash
# Optimizar número de descriptores para carga de trabajo
set port all nrxdesc 2048  # Aumentar para alta tasa de paquetes
set port all ntxdesc 2048  # Aumentar para almacenamiento en búfer
```

### Asignación de Workers
```bash
# Dedicar lcore específico para reenvío L2
set worker lcore 1 l2-repeater  # Asignar a núcleo dedicado
set worker lcore 2 tap-handler  # Separar procesamiento TAP
```

### Dimensionado de Pool de Memoria
El pool de memoria debe dimensionarse apropiadamente para el tráfico esperado:
- Considerar tasa de paquetes y requisitos de almacenamiento en búfer
- Tener en cuenta tamaño de ráfaga y almacenamiento temporal de paquetes

## Monitorización y Depuración

### Estadísticas de Puerto
```bash
# Mostrar estadísticas de reenvío
show port statistics all

# Monitorizar puertos específicos
show port statistics 0
show port statistics 1
```

### Comandos de Depuración
```bash
# Habilitar depuración del Repetidor L2
debug sdplane l2-repeater

# Depuración de conmutador VLAN (tipo de worker alternativo)
debug sdplane vlan-switch

# Depuración general de sdplane
debug sdplane fdb-change
debug sdplane rib
```

## Casos de Uso

### Repetidor Tipo Hub
- Replicar tráfico a todos los puertos conectados
- Repetición transparente Layer 2
- Funcionalidad básica de hub sin capacidades de aprendizaje

### Duplicación/Repetición de Puerto
- Duplicación de tráfico entre puertos
- Monitorización y análisis de red
- Replicación simple de paquetes

### Pruebas de Rendimiento
- Medición de rendimiento de reenvío
- Línea base para benchmarks de reenvío L2
- Validación de configuración de puertos DPDK

## Limitaciones

- **Sin Procesamiento VLAN**: Repetición simple L2 sin reconocimiento VLAN
- **Sin Aprendizaje MAC**: Reenvío de difusión a todos los puertos sin aprendizaje de direcciones
- **Sin Soporte STP**: Sin implementación de protocolo Spanning Tree
- **Sin Filtrado**: Reenvía todos los paquetes independientemente del destino

## Aplicaciones Relacionadas

- **Repetidor Mejorado**: Versión avanzada con soporte VLAN e interfaces TAP
- **Reenvío L3**: Funcionalidad de enrutamiento Layer 3
- **Conmutador VLAN**: Funcionalidad de conmutación con reconocimiento VLAN

Para funcionalidad Layer 2 más avanzada incluyendo soporte VLAN, consulte la documentación del [Repetidor Mejorado](enhanced-repeater.md).