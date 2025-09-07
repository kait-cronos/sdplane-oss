# Gestión de Dispositivos

**Language:** [English](../en/device-management.md) | [日本語](../ja/device-management.md) | [Français](../fr/device-management.md) | [中文](../zh/device-management.md) | [Deutsch](../de/device-management.md) | [Italiano](../it/device-management.md) | [한국어](../ko/device-management.md) | [ไทย](../th/device-management.md) | **Español**

Comandos para la gestión de dispositivos DPDK y controladores.

## Lista de Comandos

### show_devices - Mostrar Información de Dispositivos
```
show devices
```

Muestra información de dispositivos disponibles en el sistema.

**Ejemplo de uso:**
```bash
show devices
```

Este comando muestra la siguiente información:
- Nombre del dispositivo
- Tipo de dispositivo
- Controlador actual
- Estado del dispositivo
- Dirección PCI

### set_device_driver - Configurar Controlador de Dispositivo
```
set device <WORD> driver (ixgbe|igb|igc|uio_pci_generic|igb_uio|vfio-pci|unbound) (|bind|driver_override)
```

Configura un controlador para el dispositivo especificado.

**Parámetros:**
- `<WORD>` - Nombre del dispositivo o dirección PCI
- Tipos de controlador:
  - `ixgbe` - Controlador ixgbe de Intel 10GbE
  - `igb` - Controlador igb de Intel 1GbE
  - `igc` - Controlador igc de Intel 2.5GbE
  - `uio_pci_generic` - Controlador UIO genérico
  - `igb_uio` - Controlador DPDK UIO
  - `vfio-pci` - Controlador VFIO PCI
  - `unbound` - Eliminar controlador
- Modos de operación:
  - `bind` - Vincular controlador
  - `driver_override` - Anular controlador

**Ejemplos de uso:**
```bash
# Vincular dispositivo al controlador vfio-pci
set device 0000:01:00.0 driver vfio-pci bind

# Configurar dispositivo con controlador igb_uio
set device eth0 driver igb_uio

# Eliminar controlador del dispositivo
set device 0000:01:00.0 driver unbound
```

## Resumen de Gestión de Dispositivos

### Gestión de Dispositivos DPDK
DPDK utiliza controladores dedicados para usar dispositivos de red de manera eficiente.

### Tipos de Controladores

#### Controladores de Red
- **ixgbe** - Para tarjetas de red Intel 10GbE
- **igb** - Para tarjetas de red Intel 1GbE
- **igc** - Para tarjetas de red Intel 2.5GbE

#### Controladores UIO (Userspace I/O)
- **uio_pci_generic** - Controlador UIO genérico
- **igb_uio** - Controlador UIO dedicado para DPDK

#### Controladores VFIO (Virtual Function I/O)
- **vfio-pci** - I/O de alto rendimiento en entornos virtualizados

#### Configuraciones Especiales
- **unbound** - Eliminar controlador y deshabilitar dispositivo

## Procedimiento de Configuración de Dispositivos

### Procedimiento de Configuración Básico
1. **Verificar Dispositivos**
```bash
show devices
```

2. **Configurar Controlador**
```bash
set device <device> driver <driver> bind
```

3. **Verificar Configuración**
```bash
show devices
```

4. **Configurar Puerto**
```bash
show port
```

### Ejemplos de Configuración

#### Configuración de Tarjeta Intel 10GbE
```bash
# Verificar dispositivos
show devices

# Vincular controlador ixgbe
set device 0000:01:00.0 driver ixgbe bind
set device 0000:01:00.1 driver ixgbe bind

# Verificar configuración
show devices
show port
```

#### Configuración de Controlador DPDK UIO
```bash
# Verificar dispositivos
show devices

# Vincular controlador igb_uio
set device 0000:02:00.0 driver igb_uio bind
set device 0000:02:00.1 driver igb_uio bind

# Verificar configuración
show devices
show port
```

#### Configuración VFIO (Entorno Virtualizado)
```bash
# Verificar dispositivos
show devices

# Vincular controlador vfio-pci
set device 0000:03:00.0 driver vfio-pci bind
set device 0000:03:00.1 driver vfio-pci bind

# Verificar configuración
show devices
show port
```

## Directrices para Selección de Controladores

### ixgbe (Intel 10GbE)
- **Uso**: Tarjetas de red Intel 10GbE
- **Ventajas**: Alto rendimiento, estabilidad
- **Condición**: Requiere tarjeta Intel 10GbE

### igb (Intel 1GbE)
- **Uso**: Tarjetas de red Intel 1GbE
- **Ventajas**: Amplia compatibilidad, estabilidad
- **Condición**: Requiere tarjeta Intel 1GbE

### igc (Intel 2.5GbE)
- **Uso**: Tarjetas de red Intel 2.5GbE
- **Ventajas**: Rendimiento intermedio, nuevo estándar
- **Condición**: Requiere tarjeta Intel 2.5GbE

### uio_pci_generic
- **Uso**: Dispositivos genéricos
- **Ventajas**: Amplia compatibilidad
- **Desventajas**: Algunas limitaciones funcionales

### igb_uio
- **Uso**: Entornos dedicados a DPDK
- **Ventajas**: Optimizado para DPDK
- **Desventajas**: Requiere instalación por separado

### vfio-pci
- **Uso**: Entornos virtualizados, enfoque en seguridad
- **Ventajas**: Seguridad, soporte de virtualización
- **Condición**: Requiere habilitar IOMMU

## Solución de Problemas

### Cuando los Dispositivos no son Reconocidos
1. Verificar dispositivos
```bash
show devices
```

2. Verificación a nivel de sistema
```bash
lspci | grep Ethernet
```

3. Verificar módulos del kernel
```bash
lsmod | grep uio
lsmod | grep vfio
```

### Cuando Falla la Vinculación de Controlador
1. Verificar controlador actual
```bash
show devices
```

2. Eliminar controlador existente
```bash
set device <device> driver unbound
```

3. Vincular controlador objetivo
```bash
set device <device> driver <target_driver> bind
```

### Cuando los Puertos no están Disponibles
1. Verificar estado de dispositivos
```bash
show devices
show port
```

2. Re-vincular controlador
```bash
set device <device> driver unbound
set device <device> driver <driver> bind
```

3. Verificar configuración de puertos
```bash
show port
update port status
```

## Funciones Avanzadas

### Anulación de Controlador
```bash
# Usar anulación de controlador
set device <device> driver <driver> driver_override
```

### Configuración en Lote de Múltiples Dispositivos
```bash
# Configurar múltiples dispositivos secuencialmente
set device 0000:01:00.0 driver vfio-pci bind
set device 0000:01:00.1 driver vfio-pci bind
set device 0000:02:00.0 driver vfio-pci bind
set device 0000:02:00.1 driver vfio-pci bind
```

## Consideraciones de Seguridad

### Precauciones al Usar VFIO
- Requiere habilitar IOMMU
- Configuración de grupos de seguridad
- Configuración apropiada de permisos

### Precauciones al Usar UIO
- Requiere privilegios root
- Comprensión de riesgos de seguridad
- Control de acceso apropiado

## Integración del Sistema

### Cooperación con Servicios systemd
```bash
# Configuración automática con servicio systemd
# Configurar en /etc/systemd/system/sdplane.service
```

### Configuración Automática al Inicio
```bash
# Configuración en script de inicio
# /etc/init.d/sdplane o archivo de unidad systemd
```

## Ubicación de Definición

Estos comandos están definidos en los siguientes archivos:
- `sdplane/dpdk_devbind.c`

## Elementos Relacionados

- [Gestión de Puertos y Estadísticas](port-management.md)
- [Gestión de Workers y lcore](worker-lcore-thread-management.md)
- [Información del Sistema y Monitorización](system-monitoring.md)