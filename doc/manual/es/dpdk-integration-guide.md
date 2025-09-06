# Guía de Integración de Aplicaciones DPDK

**Idioma / Language:** [English](../dpdk-integration-guide.md) | [日本語](../ja/dpdk-integration-guide.md) | [Français](../fr/dpdk-integration-guide.md) | [中文](../zh/dpdk-integration-guide.md) | [Deutsch](../de/dpdk-integration-guide.md) | [Italiano](../it/dpdk-integration-guide.md) | [한국어](../ko/dpdk-integration-guide.md) | [ไทย](../th/dpdk-integration-guide.md) | **Español**

Esta guía explica cómo integrar aplicaciones DPDK existentes y sus hilos de trabajo basados en pthread al marco sdplane utilizando el método DPDK-dock.

## Resumen

sdplane proporciona un marco de integración para ejecutar múltiples aplicaciones DPDK dentro de un solo espacio de proceso utilizando el **método DPDK-dock**. En lugar de que cada aplicación DPDK inicialice su propio entorno EAL, las aplicaciones pueden integrarse como módulos de worker dentro del modelo de hilos cooperativos de sdplane.

El método DPDK-dock permite que múltiples aplicaciones DPDK compartan recursos de manera eficiente y coexistan mediante:
- Centralización de la inicialización DPDK EAL
- Provisión de gestión de pool de memoria compartida
- Unificación de la configuración de puertos y colas
- Habilitación de multitarea cooperativa entre diferentes cargas de trabajo DPDK

## Puntos de Integración Clave

### Conversión de Hilos de Trabajo
- Convertir workers pthread DPDK tradicionales a workers lcore de sdplane
- Reemplazar `pthread_create()` con `set worker lcore <id> <worker-type>` de sdplane
- Integrar con el modelo de hilos basado en lcore de sdplane

### Integración de Inicialización
- Eliminar llamadas específicas de aplicación `rte_eal_init()`
- Utilizar la inicialización EAL centralizada de sdplane
- Registrar comandos CLI específicos de aplicación a través del marco de comandos de sdplane

### Gestión de Memoria y Puertos
- Aprovechar la gestión de pool de memoria compartida de sdplane
- Utilizar el sistema de configuración de puertos y asignación de colas de sdplane
- Integrar con RIB (Routing Information Base) de sdplane para estado de puertos

## Pasos de Integración

### 1. Identificar Funciones de Worker
Identificar los bucles principales de procesamiento de paquetes dentro de la aplicación DPDK. Estas son típicamente funciones que:
- Operan en un bucle infinito procesando paquetes
- Utilizan `rte_eth_rx_burst()` y `rte_eth_tx_burst()`
- Manejan lógica de reenvío o procesamiento de paquetes

### 2. Crear Módulos de Worker
Implementar funciones de worker siguiendo la interfaz de worker de sdplane:

```c
static __thread uint64_t loop_counter = 0;

int
my_worker_function(__rte_unused void *dummy)
{
    unsigned lcore_id = rte_lcore_id();
    int thread_id;
    
    // Registrar contador de bucle para monitorización
    thread_id = thread_lookup_by_lcore(my_worker_function, lcore_id);
    thread_register_loop_counter(thread_id, &loop_counter);
    
    while (!force_quit && !force_stop[lcore_id]) {
        // Lógica de procesamiento de paquetes aquí
        
        // Incrementar contador de bucle para monitorización
        loop_counter++;
    }
    
    return 0;
}
```

### Monitorización de Contador de Bucle de Worker

La variable `loop_counter` permite monitorear el rendimiento del worker desde el shell de sdplane:

- **Variable Thread-local**: Cada worker mantiene su propio contador de bucle
- **Registro**: El contador se registra en el sistema de monitorización de sdplane usando nombre de worker y ID lcore
- **Incremento**: El contador se incrementa en cada iteración del bucle principal
- **Monitorización**: Los valores del contador se pueden mostrar desde la CLI de sdplane para verificar actividad del worker

**Comandos CLI de Monitorización:**
```bash
# Mostrar información de contador de hilos incluyendo contadores de bucle
show thread counter

# Mostrar información general de hilos
show thread

# Mostrar configuración y estado de workers
show worker
```

Esto permite a los administradores verificar que los workers estén procesando activamente y detectar posibles problemas de rendimiento o paradas de workers observando incrementos en los contadores de bucle.

### 3. Acceso a Información RIB usando RCU

Para acceder a información y configuración de puertos dentro de workers de procesamiento de paquetes DPDK, sdplane proporciona acceso RIB (Routing Information Base) a través de RCU (Read-Copy-Update) para operaciones thread-safe.

#### Patrón de Acceso RIB

```c
#if HAVE_LIBURCU_QSBR
#include <urcu/urcu-qsbr.h>
#endif /*HAVE_LIBURCU_QSBR*/

static __thread struct rib *rib = NULL;

int
my_worker_function(__rte_unused void *dummy)
{
    unsigned lcore_id = rte_lcore_id();
    int thread_id;
    
    // Registrar contador de bucle para monitorización
    thread_id = thread_lookup_by_lcore(my_worker_function, lcore_id);
    thread_register_loop_counter(thread_id, &loop_counter);
    
#if HAVE_LIBURCU_QSBR
    urcu_qsbr_register_thread();
#endif /*HAVE_LIBURCU_QSBR*/

    while (!force_quit && !force_stop[lcore_id]) {
#if HAVE_LIBURCU_QSBR
        urcu_qsbr_read_lock();
        rib = (struct rib *) rcu_dereference(rcu_global_ptr_rib);
#endif /*HAVE_LIBURCU_QSBR*/

        // Lógica de procesamiento de paquetes aquí
        // Acceso a información de puertos a través de rib->rib_info->port[portid]
        
#if HAVE_LIBURCU_QSBR
        urcu_qsbr_read_unlock();
        urcu_qsbr_quiescent_state();
#endif /*HAVE_LIBURCU_QSBR*/

        loop_counter++;
    }

#if HAVE_LIBURCU_QSBR
    urcu_qsbr_unregister_thread();
#endif /*HAVE_LIBURCU_QSBR*/
    
    return 0;
}
```

#### Acceso a Información de Puertos

Una vez obtenido el RIB, acceder a información específica del puerto:

```c
// Verificar estado del enlace del puerto
if (!rib->rib_info->port[portid].link.link_status) {
    // Puerto está down, omitir procesamiento
    continue;
}

// Verificar si el puerto está detenido
if (unlikely(rib->rib_info->port[portid].is_stopped)) {
    // Puerto está administrativamente detenido
    continue;
}

// Acceder a configuración del puerto
struct port_config *port_config = &rib->rib_info->port[portid];

// Obtener configuración de cola lcore
struct lcore_qconf *lcore_qconf = &rib->rib_info->lcore_qconf[lcore_id];
for (i = 0; i < lcore_qconf->nrxq; i++) {
    portid = lcore_qconf->rx_queue_list[i].port_id;
    queueid = lcore_qconf->rx_queue_list[i].queue_id;
    // Procesar paquetes de este puerto/cola
}
```

#### Directrices de Seguridad RCU

- **Registro de Hilos**: Siempre registrar hilos con `urcu_qsbr_register_thread()`
- **Bloqueo de Lectura**: Adquirir bloqueo de lectura antes de acceder a datos RIB
- **Desreferencia**: Usar `rcu_dereference()` para acceso seguro a punteros protegidos por RCU
- **Estado Quiescente**: Llamar `urcu_qsbr_quiescent_state()` para indicar puntos seguros
- **Limpieza de Hilos**: Desregistrar hilos con `urcu_qsbr_unregister_thread()`

#### Estructuras de Datos RIB

Información clave disponible a través del RIB:
- **Información de Puertos**: Estado de enlace, configuración, estadísticas
- **Configuración de Colas**: Asignaciones lcore a puerto/cola
- **Configuración VLAN**: Configuraciones de conmutador virtual y VLAN (para características avanzadas)
- **Configuración de Interfaces**: Información de interfaces TAP y enrutamiento

### 4. Agregar Comandos CLI
Registrar comandos específicos de aplicación en el sistema CLI de sdplane:

```c
CLI_COMMAND2(my_app_command,
             "my-app command <WORD>",
             "my application\n",
             "command help\n")
{
    // Implementación del comando
    return 0;
}

// En función de inicialización
INSTALL_COMMAND2(cmdset, my_app_command);
```

### 4. Configuración del Sistema de Construcción
Actualizar configuración de construcción para incluir módulo:

```makefile
# Agregar a sdplane/Makefile.am
sdplane_SOURCES += my_worker.c my_worker.h
```

### 5. Pruebas de Integración
Verificar funcionalidad usando configuración de sdplane:

```bash
# Dentro de la configuración de sdplane
set worker lcore 1 my-worker-type
set thread 1 port 0 queue 0

# Iniciar worker
start worker lcore all
```

## Aplicaciones Integradas

Las siguientes aplicaciones DPDK han sido exitosamente integradas a sdplane usando el método DPDK-dock:

### Integración L2FWD (`module/l2fwd/`)
**Estado**: ✅ Integración Exitosa
- Aplicación DPDK l2fwd original adaptada para sdplane
- Demuestra reenvío básico de paquetes entre puertos
- **Material de Referencia**: Archivos de parche disponibles para código fuente DPDK l2fwd original
- Demuestra conversión pthread-to-lcore worker
- Archivos clave: `module/l2fwd/l2fwd.c`, parches de integración

### Integración PKTGEN (`module/pktgen/`)
**Estado**: ✅ Integración Exitosa
- Integración completa de aplicación DPDK PKTGEN
- Ejemplo de aplicación compleja multi-hilo
- Integración de biblioteca externa con CLI de sdplane
- Demuestra técnicas de integración avanzadas
- Archivos clave: `module/pktgen/app/`, `sdplane/pktgen_cmd.c`

### Integración L3FWD
**Estado**: ⚠️ Integración Parcial (no completamente funcional)
- Integración intentada pero requiere trabajo adicional
- No recomendado como referencia para nuevas integraciones
- Usar L2FWD y PKTGEN como ejemplos principales

## Referencias de Integración Recomendadas

Para desarrolladores que integren nuevas aplicaciones DPDK, se recomienda usar las siguientes como referencias principales:

### 1. Integración L2FWD (Recomendada)
- **Ubicación**: `module/l2fwd/`
- **Archivos de Parche**: Disponibles para fuente DPDK l2fwd original
- **Alcance**: Comprensión de conversión básica pthread-to-lcore
- **Patrón de Integración**: Worker simple de reenvío de paquetes

### 2. Integración PKTGEN (Referencia Avanzada)
- **Ubicación**: `module/pktgen/` y `sdplane/pktgen_cmd.c`
- **Alcance**: Integración de aplicación compleja con comandos CLI
- **Patrón de Integración**: Aplicación multi-componente con dependencias externas

### Archivos de Parche de Integración

Para la integración L2FWD, están disponibles archivos de parche que muestran las modificaciones exactas necesarias para adaptar la aplicación DPDK l2fwd original para integración con sdplane. Estos parches demuestran:
- Eliminación de inicialización EAL
- Adaptación del modelo de hilos
- Integración de comandos CLI
- Cambios en gestión de recursos

Los desarrolladores deben consultar estos archivos de parche para comprender un enfoque sistemático para la integración de aplicaciones DPDK.

## Ejemplos de Workers Personalizados

### Repetidor L2 (`sdplane/l2_repeater.c`)
Worker sdplane personalizado (no basado en aplicación DPDK existente):
- Worker simple de reenvío de paquetes
- Reenvío de difusión a todos los puertos activos
- Integración con gestión de puertos de sdplane

### Repetidor Mejorado (`sdplane/enhanced_repeater.c`)
Worker sdplane personalizado con características avanzadas:
- Conmutación con reconocimiento VLAN
- Integración de interfaz TAP
- Integración RIB para estado de puertos

### Conmutador VLAN (`sdplane/vlan_switch.c`)
Implementación de conmutación Layer 2 personalizada:
- Aprendizaje y reenvío MAC
- Procesamiento VLAN
- Procesamiento de paquetes multi-puerto

## Mejores Prácticas

### Consideraciones de Rendimiento
- Usar procesamiento en ráfagas para rendimiento óptimo
- Minimizar copias de paquetes cuando sea posible
- Aprovechar mecanismos de copia cero de DPDK
- Considerar topología NUMA en asignación de workers

### Manejo de Errores
- Verificar valores de retorno de funciones DPDK
- Implementar manejo adecuado de cierre
- Usar niveles de log apropiados
- Manejar limpieza de recursos apropiadamente

### Modelo de Hilos
- Comprender el modelo de hilos basado en lcore de sdplane
- Diseñar bucles eficientes de procesamiento de paquetes
- Usar mecanismos de sincronización apropiados
- Considerar afinidad de hilos y aislamiento de CPU

## Depuración y Monitorización

### Logs de Depuración
Habilitar logs de depuración para worker:

```bash
debug sdplane my-worker-type
```

### Recolección de Estadísticas
Integrar con marco de estadísticas de sdplane:

```c
// Actualizar estadísticas de puerto
port_statistics[portid].rx += nb_rx;
port_statistics[portid].tx += nb_tx;
```

### Monitorización CLI
Proporcionar comandos de estado para monitorización:

```bash
show my-worker status
show my-worker statistics
```

## Patrones de Integración Comunes

### Pipeline de Procesamiento de Paquetes
```c
// Procesamiento típico de paquetes en worker sdplane
while (!force_quit && !force_stop[lcore_id]) {
    // 1. Recepción de paquetes
    nb_rx = rte_eth_rx_burst(portid, queueid, pkts_burst, MAX_PKT_BURST);
    
    // 2. Procesamiento de paquetes
    for (i = 0; i < nb_rx; i++) {
        // Lógica de procesamiento
        process_packet(pkts_burst[i]);
    }
    
    // 3. Transmisión de paquetes
    rte_eth_tx_burst(dst_port, queueid, pkts_burst, nb_rx);
}
```

### Integración de Configuración
```c
// Registrar en sistema de configuración de sdplane
struct worker_config my_worker_config = {
    .name = "my-worker",
    .worker_func = my_worker_function,
    .init_func = my_worker_init,
    .cleanup_func = my_worker_cleanup
};
```

## Solución de Problemas

### Problemas Comunes
- **Worker no inicia**: Verificar asignación lcore y configuración
- **Descarte de paquetes**: Verificar configuración de colas y tamaños de búfer
- **Problemas de rendimiento**: Verificar afinidad CPU y configuración NUMA
- **Comandos CLI no funcionan**: Verificar registro adecuado de comandos

### Técnicas de Depuración
- Usar sistema de logs de depuración de sdplane
- Monitorizar estadísticas y contadores de worker
- Verificar estado y configuración de enlace de puertos
- Validar asignaciones de pool de memoria

Para ejemplos de implementación detallados, consulte los módulos de worker existentes en el código base y sus definiciones de comandos CLI correspondientes.