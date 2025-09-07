# Gestión lthread

**Idioma / Language:** [English](../en/lthread-management.md) | [日本語](../ja/lthread-management.md) | [Français](../fr/lthread-management.md) | [中文](../zh/lthread-management.md) | [Deutsch](../de/lthread-management.md) | [Italiano](../it/lthread-management.md) | [한국어](../ko/lthread-management.md) | [ไทย](../th/lthread-management.md) | **Español**

Comandos para la gestión de lthread (hilos ligeros).

## Lista de Comandos

### set_worker_lthread_stat_collector - Configurar Recopilador de Estadísticas lthread
```
set worker lthread stat-collector
```

Configura un worker lthread para recopilar información estadística.

**Ejemplo de uso:**
```bash
set worker lthread stat-collector
```

### set_worker_lthread_rib_manager - Configurar Administrador RIB lthread
```
set worker lthread rib-manager
```

Configura un worker lthread para gestionar RIB (Routing Information Base).

**Ejemplo de uso:**
```bash
set worker lthread rib-manager
```

### set_worker_lthread_netlink_thread - Configurar Hilo netlink lthread
```
set worker lthread netlink-thread
```

Configura un worker lthread para procesar comunicación Netlink.

**Ejemplo de uso:**
```bash
set worker lthread netlink-thread
```

## Resumen de lthread

### Qué es lthread
lthread (lightweight thread) es una implementación de hilos ligeros que realiza multihilo cooperativo.

### Características Principales
- **Ligereza** - Baja sobrecarga de memoria
- **Alta Velocidad** - Cambio de contexto rápido
- **Cooperación** - Control mediante yield explícito
- **Escalabilidad** - Gestión eficiente de gran cantidad de hilos

### Diferencias con Hilos Tradicionales
- **Preventivo vs Cooperativo** - Transferencia explícita de control
- **Hilos de Kernel vs Hilos de Usuario** - Ejecución sin intervención del kernel
- **Pesado vs Ligero** - Baja sobrecarga de creación y cambio

## Tipos de Workers lthread

### Recopilador de Estadísticas (stat-collector)
Worker que recopila periódicamente información estadística del sistema.

**Funciones:**
- Recolección de estadísticas de puertos
- Recolección de estadísticas de hilos
- Recolección de estadísticas del sistema
- Agregación de datos estadísticos

**Ejemplos de uso:**
```bash
# Configurar recopilador de estadísticas
set worker lthread stat-collector

# Verificar información estadística
show port statistics
show thread counter
```

### Administrador RIB (rib-manager)
Worker que gestiona RIB (Routing Information Base).

**Funciones:**
- Gestión de tabla de enrutamiento
- Adición, eliminación y actualización de rutas
- Distribución de información de rutas
- Monitorización del estado de enrutamiento

**Ejemplos de uso:**
```bash
# Configurar administrador RIB
set worker lthread rib-manager

# Verificar información RIB
show rib
```

### Hilo Netlink (netlink-thread)
Worker que procesa comunicación Netlink.

**Funciones:**
- Comunicación Netlink con el kernel
- Monitorización de configuración de red
- Monitorización del estado de interfaces
- Recepción de información de enrutamiento

**Ejemplos de uso:**
```bash
# Configurar hilo Netlink
set worker lthread netlink-thread

# Verificar estado de red
show port
show vswitch
```

## Configuración de lthread

### Procedimiento de Configuración Básico
1. **Configurar Workers lthread Necesarios**
```bash
# Configurar recopilador de estadísticas
set worker lthread stat-collector

# Configurar administrador RIB
set worker lthread rib-manager

# Configurar hilo Netlink
set worker lthread netlink-thread
```

2. **Verificar Configuración**
```bash
# Verificar estado de workers
show worker

# Verificar información de hilos
show thread
```

### Configuración Recomendada
Para uso general, se recomienda la siguiente combinación:
```bash
# Configuración básica de workers lthread
set worker lthread stat-collector
set worker lthread rib-manager
set worker lthread netlink-thread
```

## Ejemplos de Uso

### Configuración Básica
```bash
# Configurar workers lthread
set worker lthread stat-collector
set worker lthread rib-manager
set worker lthread netlink-thread

# Verificar configuración
show worker
show thread
```

### Configuración de Monitorización de Estadísticas
```bash
# Configurar recopilador de estadísticas
set worker lthread stat-collector

# Verificar información estadística periódicamente
show port statistics
show thread counter
show mempool
```

### Configuración de Gestión de Enrutamiento
```bash
# Configurar administrador RIB
set worker lthread rib-manager

# Verificar información de enrutamiento
show rib
show vswitch
```

## Monitorización y Gestión

### Verificar Estado de lthread
```bash
# Verificación general del estado
show worker
show thread

# Verificación de información estadística específica
show thread counter
show loop-count console pps
```

### Monitorización de Rendimiento
```bash
# Verificar rendimiento de lthread
show thread counter

# Verificar rendimiento general del sistema
show port statistics pps
show mempool
```

## Solución de Problemas

### Cuando lthread no Opera
1. Verificar configuración de workers
```bash
show worker
```

2. Verificar estado de hilos
```bash
show thread
```

3. Verificar estado del sistema
```bash
show rcu
show mempool
```

### Cuando la Información Estadística no se Actualiza
1. Verificar estado del recopilador de estadísticas
```bash
show worker
show thread
```

2. Verificar información estadística manualmente
```bash
show port statistics
show thread counter
```

### Cuando RIB no se Actualiza
1. Verificar estado del administrador RIB
```bash
show worker
show thread
```

2. Verificar información RIB
```bash
show rib
```

3. Verificar estado del hilo Netlink
```bash
show worker
```

## Funciones Avanzadas

### Operación Cooperativa de lthread
Como lthread opera de manera cooperativa, es necesario prestar atención a los siguientes puntos:

- **Yield Explícito** - Es necesario yield explícito para procesamiento de larga duración
- **Evitar Deadlock** - Evitar deadlock mediante diseño cooperativo
- **Equidad** - Todos los hilos se ejecutan apropiadamente

### Optimización de Rendimiento
- **Colocación Apropiada de Workers** - Considerar afinidad con núcleos de CPU
- **Eficiencia de Memoria** - Configurar tamaño apropiado de pool de memoria
- **Distribución de Carga** - Distribución de carga con múltiples workers

## Ubicación de Definición

Estos comandos están definidos en los siguientes archivos:
- `sdplane/lthread_main.c`

## Elementos Relacionados

- [Gestión de Workers y lcore](worker-lcore-thread-management.md)
- [Información de Hilos](worker-lcore-thread-management.md)
- [Información del Sistema y Monitorización](system-monitoring.md)
- [RIB y Enrutamiento](routing.md)