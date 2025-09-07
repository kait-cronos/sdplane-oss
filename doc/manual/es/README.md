# Guía de Usuario de sdplane-oss

**Idioma / Language:** [English](../en/README.md) | [日本語](../ja/README.md) | [Français](../fr/README.md) | [中文](../zh/README.md) | [Deutsch](../de/README.md) | [Italiano](../it/README.md) | [한국어](../ko/README.md) | [ไทย](../th/README.md) | **Español**

sdplane-oss es un enrutador de software de alto rendimiento basado en DPDK. Esta guía de usuario explica todos los comandos y funcionalidades de sdplane.

## Índice

1. [Gestión de Puertos y Estadísticas](port-management.md) - Gestión de puertos DPDK e información estadística
2. [Gestión Worker & lcore & Información Hilos](worker-lcore-thread-management.md) - Gestión de worker threads, lcore e información de hilos

3. [Depuración y Registros](debug-logging.md) - Funciones de depuración y registro
4. [Gestión VTY y Shell](vty-shell.md) - Gestión de VTY y shell
5. [Información del Sistema y Monitorización](system-monitoring.md) - Información del sistema y funciones de monitorización
6. [RIB y Enrutamiento](routing.md) - Funcionalidades de RIB y enrutamiento
7. [Configuración de Colas](queue-configuration.md) - Configuración y gestión de colas
8. [Generación de Paquetes](packet-generation.md) - Generación de paquetes usando PKTGEN

9. [Interfaz TAP](tap-interface.md) - Gestión de interfaz TAP
10. [Gestión lthread](lthread-management.md) - Gestión de lthread
11. [Gestión de Dispositivos](device-management.md) - Gestión de dispositivos y controladores

## Uso Básico

### Método de Conexión

Para conectar a sdplane:

```bash
# Iniciar sdplane
sudo ./sdplane/sdplane

# Conectar al CLI desde otro terminal
telnet localhost 9882
```

### Mostrar Ayuda

En cada comando puede usar `?` para mostrar ayuda:

```
sdplane# ?
sdplane# show ?
sdplane# set ?
```

### Comandos Básicos

- `show version` - Mostrar información de versión
- `show port` - Mostrar información de puertos
- `show worker` - Mostrar información de workers
- `exit` - Salir del CLI

## Clasificación de Comandos

sdplane tiene definidos 79 comandos, clasificados en las siguientes 13 categorías funcionales:

1. **Gestión de Puertos y Estadísticas** (10 comandos) - Control y estadísticas de puertos DPDK
2. **Gestión de Workers y lcore** (6 comandos) - Gestión de hilos de trabajo y lcore
3. **Depuración y Registros** (2 comandos) - Funciones de depuración y registro
4. **Gestión VTY y Shell** (4 comandos) - Control de VTY y shell
5. **Información del Sistema y Monitorización** (10 comandos) - Información del sistema y monitorización
6. **RIB y Enrutamiento** (1 comando) - Gestión de información de enrutamiento
7. **Configuración de Colas** (3 comandos) - Configuración de colas
8. **Generación de Paquetes** (3 comandos) - Generación de paquetes por PKTGEN
9. **Información de Hilos** (2 comandos) - Monitorización de hilos
10. **Interfaz TAP** (2 comandos) - Gestión de interfaz TAP
11. **Gestión lthread** (3 comandos) - Gestión de lthread
12. **Gestión de Dispositivos** (2 comandos) - Gestión de dispositivos y controladores

Para métodos de uso detallados, consulte la documentación de cada categoría.