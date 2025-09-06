# Información del Sistema y Monitorización

**Idioma / Language:** [English](../system-monitoring.md) | [日本語](../ja/system-monitoring.md) | [Français](../fr/system-monitoring.md) | [中文](../zh/system-monitoring.md) | [Deutsch](../de/system-monitoring.md) | [Italiano](../it/system-monitoring.md) | [한국어](../ko/system-monitoring.md) | [ไทย](../th/system-monitoring.md) | **Español**

Comandos que proporcionan información del sistema y funciones de monitorización de sdplane.

## Lista de Comandos

### show_version - Mostrar Versión
```
show version
```

Muestra información de versión de sdplane.

**Ejemplo de uso:**
```bash
show version
```

### set_locale - Configuración de Locale
```
set locale (C|C.utf8|en_US.utf8|POSIX)
```

Configura el locale del sistema.

**Locales disponibles:**
- `C` - Locale C estándar
- `C.utf8` - Locale C compatible con UTF-8
- `en_US.utf8` - Locale inglés UTF-8
- `POSIX` - Locale POSIX

**Ejemplos de uso:**
```bash
# Configurar a locale C compatible con UTF-8
set locale C.utf8

# Configurar a locale inglés UTF-8
set locale en_US.utf8
```

### set_argv_list_1 - Configuración argv-list
```
set argv-list <0-7> <WORD>
```

Configura la lista de argumentos de línea de comandos.

**Parámetros:**
- `<0-7>` - Índice (0-7)
- `<WORD>` - Cadena a configurar

**Ejemplos de uso:**
```bash
# Configurar argumento en índice 0
set argv-list 0 "--verbose"

# Configurar argumento en índice 1
set argv-list 1 "--config"
```

### **show argv-list**

Muestra toda la lista de argumentos de línea de comandos configurada.

**Ejemplo de uso:**
```bash
# Mostrar toda la argv-list
show argv-list
```

---

### **show argv-list \<0-7\>**

Muestra la argv-list de un índice específico.

**Ejemplos de uso:**
```bash
# Mostrar argv-list de un índice específico
show argv-list 0

# Mostrar argv-list del índice 3
show argv-list 3
```

### show_loop_count - Mostrar Contador de Bucles
```
show loop-count (console|vty-shell|l2fwd) (pps|total)
```

Muestra contadores de bucles de cada componente.

**Componentes:**
- `console` - Consola
- `vty-shell` - Shell VTY
- `l2fwd` - Reenvío L2

**Tipos de estadísticas:**
- `pps` - Número de bucles por segundo
- `total` - Número total de bucles

**Ejemplos de uso:**
```bash
# Mostrar PPS de consola
show loop-count console pps

# Mostrar número total de bucles de reenvío L2
show loop-count l2fwd total
```

### show_rcu - Mostrar Información RCU
```
show rcu
```

Muestra información de RCU (Read-Copy-Update).

**Ejemplo de uso:**
```bash
show rcu
```

### show_fdb - Mostrar Información FDB
```
show fdb
```

Muestra información de FDB (Forwarding Database).

**Ejemplo de uso:**
```bash
show fdb
```

### show_vswitch - Mostrar Información vswitch
```
show vswitch
```

Muestra información del conmutador virtual.

**Ejemplo de uso:**
```bash
show vswitch
```

### sleep_cmd - Comando Sleep
```
sleep <0-300>
```

Duerme durante el número especificado de segundos.

**Parámetros:**
- `<0-300>` - Tiempo de suspensión (segundos)

**Ejemplos de uso:**
```bash
# Dormir por 5 segundos
sleep 5

# Dormir por 30 segundos
sleep 30
```

### show_mempool - Mostrar Información de Pool de Memoria
```
show mempool
```

Muestra información del pool de memoria DPDK.

**Ejemplo de uso:**
```bash
show mempool
```

## Descripción de Ítems de Monitorización

### Información de Versión
- Versión de sdplane
- Información de compilación
- Versiones de librerías dependientes

### Contador de Bucles
- Número de bucles de procesamiento de cada componente
- Usado para monitorización de rendimiento
- Usado para calcular PPS (Paquetes Por Segundo)

### Información RCU
- Estado del mecanismo Read-Copy-Update
- Estado del procesamiento de sincronización
- Estado de gestión de memoria

### Información FDB
- Estado de la tabla de direcciones MAC
- Direcciones MAC aprendidas
- Información de aging

### Información vswitch
- Configuración del conmutador virtual
- Información de puertos
- Configuración VLAN

### Información del Pool de Memoria
- Memoria disponible
- Memoria en uso
- Estadísticas del pool de memoria

## Mejores Prácticas de Monitorización

### Monitorización Regular
```bash
# Comandos básicos de monitorización
show version
show mempool
show vswitch
show rcu
```

### Monitorización de Rendimiento
```bash
# Monitorización de rendimiento con contadores de bucles
show loop-count console pps
show loop-count l2fwd pps
```

### Solución de Problemas
```bash
# Verificación de estado del sistema
show fdb
show vswitch
show mempool
```

## Ubicación de Definición

Estos comandos están definidos en los siguientes archivos:
- `sdplane/sdplane.c`

## Elementos Relacionados

- [Gestión de Puertos y Estadísticas](port-management.md)
- [Gestión de Workers y lcore](worker-lcore-thread-management.md)
- [Información de Hilos](worker-lcore-thread-management.md)