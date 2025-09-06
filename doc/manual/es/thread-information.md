# Información de Hilos

**Idioma / Language:** [English](../worker-lcore-thread-management.md) | [日本語](../ja/worker-lcore-thread-management.md) | [Français](../fr/worker-lcore-thread-management.md) | [中文](../zh/worker-lcore-thread-management.md) | [Deutsch](../de/worker-lcore-thread-management.md) | [Italiano](../it/worker-lcore-thread-management.md) | [한국어](../ko/worker-lcore-thread-management.md) | [ไทย](../th/worker-lcore-thread-management.md) | **Español**

Comandos para información y monitorización de hilos.

## Lista de Comandos

### show_thread_cmd - Mostrar Información de Hilos
```
show thread
```

Muestra el estado e información actuales de los hilos.

**Ejemplo de uso:**
```bash
show thread
```

Este comando muestra la siguiente información:
- ID de hilo
- Estado del hilo
- Tarea en ejecución
- Uso de CPU
- Uso de memoria

### show_thread_counter - Mostrar Contador de Hilos
```
show thread counter
```

Muestra información del contador de hilos.

**Ejemplo de uso:**
```bash
show thread counter
```

Este comando muestra la siguiente información:
- Número de paquetes procesados
- Número de ejecuciones
- Número de errores
- Estadísticas de tiempo de procesamiento

## Resumen del Sistema de Hilos

### Arquitectura de Hilos de sdplane
En sdplane se utilizan los siguientes tipos de hilos:

#### 1. Hilos de Trabajo
- **Reenvío L2** - Transferencia de paquetes Layer 2
- **Reenvío L3** - Transferencia de paquetes Layer 3
- **Manejador TAP** - Procesamiento de interfaz TAP
- **PKTGEN** - Generación de paquetes

#### 2. Hilos de Gestión
- **Administrador RIB** - Gestión de información de enrutamiento
- **Recopilador de Estadísticas** - Recolección de información estadística
- **Hilo Netlink** - Procesamiento de comunicación Netlink

#### 3. Hilos del Sistema
- **Servidor VTY** - Procesamiento de conexiones VTY
- **Consola** - Entrada/salida de consola

### Relación con lthread
sdplane usa un modelo de multihilo cooperativo (cooperative threading):

- **lthread** - Implementación de hilos ligeros
- **Programación Cooperativa** - Control mediante yield explícito
- **Alta Eficiencia** - Reducción de sobrecarga de cambio de contexto

## Cómo Leer la Información de Hilos

### Ítems de Visualización Básicos
- **Thread ID** - Identificador del hilo
- **Name** - Nombre del hilo
- **State** - Estado del hilo
- **lcore** - Núcleo de CPU en ejecución
- **Type** - Tipo de hilo

### Estados de Hilos
- **Running** - En ejecución
- **Ready** - Ejecutable
- **Blocked** - Bloqueado
- **Terminated** - Terminado

### Información de Contadores
- **Packets** - Número de paquetes procesados
- **Loops** - Número de ejecuciones de bucles
- **Errors** - Número de errores
- **CPU Time** - Tiempo de uso de CPU

## Ejemplos de Uso

### Monitorización Básica
```bash
# Mostrar información de hilos
show thread

# Mostrar contadores de hilos
show thread counter
```

### Interpretación de Salida de Ejemplo
```bash
# Ejemplo de salida de show thread
Thread ID: 1
Name: l2fwd-worker
State: Running
lcore: 1
Type: L2FWD

Thread ID: 2
Name: rib-manager
State: Running
lcore: 2
Type: RIB_MANAGER
```

```bash
# Ejemplo de salida de show thread counter
Thread ID: 1
Packets: 1000000
Loops: 5000000
Errors: 0
CPU Time: 123.45s
```

## Monitorización y Solución de Problemas

### Monitorización Regular
```bash
# Comandos de monitorización regular
show thread
show thread counter
```

### Análisis de Rendimiento
```bash
# Información relacionada con rendimiento
show thread counter
show loop-count l2fwd pps
show worker
```

### Solución de Problemas

#### Cuando los Hilos No Responden
1. Verificar estado de hilos
```bash
show thread
```

2. Verificar estado de workers
```bash
show worker
```

3. Reiniciar si es necesario
```bash
restart worker lcore 1
```

#### Cuando el Rendimiento Disminuye
1. Verificar información de contadores
```bash
show thread counter
```

2. Verificar contadores de bucles
```bash
show loop-count l2fwd pps
```

3. Verificar número de errores
```bash
show thread counter
```

#### Cuando el Uso de Memoria es Alto
1. Verificar información de pool de memoria
```bash
show mempool
```

2. Verificar información de hilos
```bash
show thread
```

## Optimización de Hilos

### Configuración de Afinidad de CPU
- Colocar workers en lcore apropiados
- Colocación considerando nodos NUMA
- Equilibrio del uso de CPU

### Mejora de Eficiencia de Memoria
- Tamaño apropiado de pool de memoria
- Prevención de fugas de memoria
- Mejora de eficiencia de caché

## Funciones Avanzadas

### Gestión lthread
```bash
# Configurar workers lthread
set worker lthread stat-collector
set worker lthread rib-manager
set worker lthread netlink-thread
```

### Utilización de Información Estadística
- Monitorización de rendimiento
- Planificación de capacidad
- Detección de anomalías

## Ubicación de Definición

Estos comandos están definidos en los siguientes archivos:
- `sdplane/thread_info.c`

## Elementos Relacionados

- [Gestión de Workers y lcore](worker-lcore-thread-management.md)
- [Gestión lthread](lthread-management.md)
- [Información del Sistema y Monitorización](system-monitoring.md)