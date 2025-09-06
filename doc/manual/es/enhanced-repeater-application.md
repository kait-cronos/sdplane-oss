# Aplicación de Repetidor Mejorado

**Idioma / Language:** [English](../enhanced-repeater-application.md) | [日本語](../ja/enhanced-repeater-application.md) | [Français](../fr/enhanced-repeater-application.md) | [中文](../zh/enhanced-repeater-application.md) | [Deutsch](../de/enhanced-repeater-application.md) | [Italiano](../it/enhanced-repeater-application.md) | [한국어](../ko/enhanced-repeater-application.md) | [ไทย](../th/enhanced-repeater-application.md) | **Español**

La aplicación de Repetidor Mejorado proporciona conmutación Layer 2 avanzada con reconocimiento VLAN con integración de interfaces TAP para enrutamiento L3 y funcionalidades de captura de paquetes.

## Resumen

El Repetidor Mejorado es una aplicación sofisticada de reenvío Layer 2 que extiende el reenvío L2 básico con las siguientes capacidades:
- **Conmutación con Reconocimiento VLAN**: Soporte completo de IEEE 802.1Q VLAN (etiquetado/desetiquetado)
- **Abstracción de Conmutador Virtual**: Múltiples conmutadores virtuales con tablas de reenvío independientes
- **Integración de Interfaz TAP**: Interfaces de enrutador y captura para integración con kernel
- **Procesamiento VLAN Avanzado**: Capacidades de traducción, inserción y eliminación VLAN

## Arquitectura

### Marco de Conmutador Virtual
El Repetidor Mejorado implementa una arquitectura de conmutador virtual:
- **Múltiples VLANs**: Soporte para múltiples dominios VLAN (1-4094)
- **Agregación de Puertos**: Múltiples puertos físicos por conmutador virtual
- **Reenvío Aislado**: Dominios de reenvío independientes por VLAN
- **Etiquetado Flexible**: Modos nativo, etiquetado y traducción por puerto

### Integración de Interfaz TAP
- **Interfaz de Enrutador**: Integración con pila de red del kernel para procesamiento L3
- **Interfaz de Captura**: Capacidades de monitorización y análisis de paquetes
- **Búferes de Anillo**: Transferencia eficiente de paquetes entre plano de datos y kernel
- **Bidireccional**: Procesamiento de paquetes en ambas direcciones

## Características Clave

### Procesamiento VLAN
- **Etiquetado VLAN**: Adición de encabezado 802.1Q a tramas sin etiquetar
- **Desetiquetado VLAN**: Eliminación de encabezado 802.1Q de tramas etiquetadas
- **Traducción VLAN**: Cambio de ID VLAN entre entrada y salida
- **VLAN Nativa**: Manejo de tráfico sin etiquetar en puertos troncales

### Conmutación Virtual
- **Aprendizaje**: Aprendizaje automático de direcciones MAC por VLAN
- **Inundación**: Manejo adecuado de unicast desconocido y broadcast
- **Horizonte Dividido**: Prevención de bucles dentro del conmutador virtual
- **Multi-dominio**: Tablas de reenvío independientes por VLAN

### Procesamiento de Paquetes
- **Copia Cero**: Procesamiento eficiente de paquetes DPDK con sobrecarga mínima
- **Procesamiento en Ráfagas**: Optimizado para altas tasas de paquetes
- **Manipulación de Encabezados**: Inserción/eliminación eficiente de encabezados VLAN
- **Optimización de Copia**: Copia selectiva de paquetes para interfaces TAP

## Configuración

### Configuración de Conmutador Virtual
Crear conmutadores virtuales para diferentes dominios VLAN:

```bash
# Crear conmutadores virtuales con ID VLAN
set vswitch 2031
set vswitch 2032
```

### Enlace de Puerto a Conmutador Virtual
Enlazar puertos físicos DPDK a conmutadores virtuales:

```bash
# Enlazar puerto 0 a conmutador virtual 0 con etiqueta VLAN 2031
set vswitch-link vswitch 0 port 0 tag 2031

# Enlazar puerto 0 a conmutador virtual 1 nativo/sin etiquetar
set vswitch-link vswitch 1 port 0 tag 0

# Ejemplo de traducción VLAN
set vswitch-link vswitch 0 port 1 tag 2040
```

### Interfaz de Enrutador
Crear interfaces de enrutador para conectividad L3:

```bash
# Crear interfaz de enrutador para procesamiento L3
set vswitch 2031 router-if rif2031
set vswitch 2032 router-if rif2032
```

### Interfaz de Captura
Configurar interfaz de captura para monitorización:

```bash
# Crear interfaz de captura para monitorización de paquetes
set vswitch 2031 capture-if cif2031
set vswitch 2032 capture-if cif2032
```

### Configuración de Worker
Asignar Repetidor Mejorado a un lcore:

```bash
# Configurar worker de Repetidor Mejorado
set worker lcore 1 enhanced-repeater

# Configurar asignación de cola
set thread 1 port 0 queue 0
```

## Ejemplo de Configuración

### Configuración Completa
Consulte [`example-config/sdplane_enhanced_repeater.conf`](../../example-config/sdplane_enhanced_repeater.conf):

```bash
# Configuración de dispositivo
set device 03:00.0 driver vfio-pci bind

# Inicialización DPDK
set rte_eal argv -c 0x7
rte_eal_init
set mempool

# Configuración de puertos
stop port all
set port all dev-configure 1 4
set port all nrxdesc 1024
set port all ntxdesc 1024

# Configuración de workers
set worker lthread stat-collector
set worker lthread rib-manager
set worker lcore 1 enhanced-repeater
set worker lcore 2 l3-tap-handler

# Habilitar modo promiscuo
set port all promiscuous enable
start port all

# Configuración de colas
set thread 1 port 0 queue 0

# Configuración de conmutador virtual
set vswitch 2031
set vswitch 2032

# Configuración de enlaces
set vswitch-link vswitch 0 port 0 tag 2031
set vswitch-link vswitch 1 port 0 tag 2032

# Creación de interfaces
set vswitch 2031 router-if rif2031
set vswitch 2032 router-if rif2032
set vswitch 2031 capture-if cif2031
set vswitch 2032 capture-if cif2032

# Iniciar workers
start worker lcore all
```

## Detalles de Procesamiento VLAN

### Modos de Etiquetado

#### Modo Etiquetado
- **Entrada**: Acepta paquetes con etiqueta VLAN específica
- **Salida**: Mantiene o traduce etiqueta VLAN
- **Uso**: Puertos troncales, traducción VLAN

#### Modo Nativo (tag 0)
- **Entrada**: Acepta paquetes sin etiquetar
- **Salida**: Elimina encabezado VLAN
- **Uso**: Puertos de acceso, dispositivos sin VLAN

#### Modo Traducción
- **Funcionalidad**: Cambio de ID VLAN entre puertos
- **Configuración**: Diferentes etiquetas en enlaces de entrada y salida
- **Uso**: Mapeo VLAN, redes de proveedor de servicios

### Flujo de Paquetes

1. **Procesamiento de Entrada**:
   - Recepción de paquetes en puerto DPDK
   - Determinación VLAN basada en configuración de etiqueta o nativa
   - Búsqueda de conmutador virtual de destino

2. **Búsqueda en Conmutador Virtual**:
   - Aprendizaje y búsqueda de direcciones MAC
   - Determinación de puerto de salida dentro del dominio VLAN
   - Manejo de inundación de unicast/broadcast desconocido

3. **Procesamiento de Salida**:
   - Manipulación de encabezado VLAN por configuración de puerto
   - Copia de paquetes para múltiples destinos
   - Integración de interfaz TAP

4. **Procesamiento de Interfaz TAP**:
   - Interfaz de enrutador: Procesamiento L3 del kernel
   - Interfaz de captura: Monitorización y análisis

## Monitorización y Gestión

### Comandos de Estado
```bash
# Mostrar configuración de conmutador virtual
show vswitch_rib

# Mostrar enlaces de conmutador virtual
show vswitch-link

# Mostrar interfaces de enrutador
show rib vswitch router-if

# Verificar interfaces de captura
show rib vswitch capture-if
```

### Estadísticas y Rendimiento
```bash
# Estadísticas a nivel de puerto
show port statistics all

# Rendimiento de workers
show worker statistics

# Información de hilos
show thread information
```

### Comandos de Depuración
```bash
# Depuración de Repetidor Mejorado
debug sdplane enhanced-repeater

# Depuración de conmutador VLAN (tipo de worker alternativo)
debug sdplane vlan-switch

# Depuración de RIB y reenvío
debug sdplane rib
debug sdplane fdb-change
```

## Casos de Uso

### Agregación VLAN
- Consolidación de múltiples VLANs en un solo enlace físico
- Funcionalidad de borde de proveedor
- Multiplexación de servicios

### Integración L3
- Configuración Router-on-a-stick
- Enrutamiento inter-VLAN vía interfaces TAP
- Reenvío híbrido L2/L3

### Monitorización de Red
- Captura de paquetes por VLAN
- Análisis de tráfico y depuración
- Monitorización de servicios

### Redes de Proveedor de Servicios
- Traducción VLAN para aislamiento de clientes
- Redes multi-tenant
- Ingeniería de tráfico

## Consideraciones de Rendimiento

### Escalabilidad
- **Asignación de Workers**: Dedicar lcore para rendimiento óptimo
- **Configuración de Colas**: Equilibrar asignación de colas entre núcleos
- **Pool de Memoria**: Dimensionar apropiadamente para tasa de paquetes y necesidades de almacenamiento

### Sobrecarga de Procesamiento VLAN
- **Manipulación de Encabezados**: Sobrecarga mínima para operaciones VLAN
- **Copia de Paquetes**: Copia selectiva solo cuando sea necesario
- **Optimización de Ráfagas**: Procesamiento de paquetes en ráfagas para eficiencia

### Rendimiento de Interfaz TAP
- **Dimensionado de Búfer de Anillo**: Configurar tamaños de anillo apropiados
- **Transferencia Selectiva**: Enviar solo paquetes relevantes a interfaces TAP
- **Integración con Kernel**: Considerar sobrecarga de procesamiento del kernel

## Solución de Problemas

### Problemas Comunes
- **Desajuste VLAN**: Verificar que configuraciones de etiquetas coincidan con configuración de red
- **Creación de Interfaz TAP**: Verificar permisos apropiados y soporte del kernel
- **Problemas de Rendimiento**: Verificar asignación de colas y distribución de workers

### Estrategias de Depuración
- **Habilitar Logs de Depuración**: Usar comandos de depuración para flujo detallado de paquetes
- **Monitorizar Estadísticas**: Monitorizar estadísticas de puerto y worker
- **Captura de Paquetes**: Usar interfaces de captura para análisis de tráfico

## Documentación Relacionada

- [Comandos de Repetidor Mejorado](enhanced-repeater.md) - Referencia completa de comandos
- [Gestión de Workers](worker-management.md) - Detalles de configuración de workers
- [Gestión de Puertos](port-management.md) - Configuración de puertos DPDK
- [Gestión de Interfaz TAP](tap-interface.md) - Detalles de interfaz TAP