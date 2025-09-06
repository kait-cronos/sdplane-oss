# Interfaz TAP

**Idioma / Language:** [English](../tap-interface.md) | [日本語](../ja/tap-interface.md) | [Français](../fr/tap-interface.md) | [中文](../zh/tap-interface.md) | [Deutsch](../de/tap-interface.md) | [Italiano](../it/tap-interface.md) | [한국어](../ko/tap-interface.md) | [ไทย](../th/tap-interface.md) | **Español**

Comandos para la gestión de interfaces TAP.

## Lista de Comandos

### set_tap_capture_ifname - Configurar Nombre de Interfaz de Captura TAP
```
set tap capture ifname <WORD>
```

Configura el nombre de interfaz utilizado por la funcionalidad de captura TAP.

**Parámetros:**
- `<WORD>` - Nombre de interfaz

**Ejemplos de uso:**
```bash
# Configurar interfaz tap0
set tap capture ifname tap0

# Configurar interfaz tap1
set tap capture ifname tap1
```

### set_tap_capture_persistent - Habilitar Persistencia de Captura TAP
```
set tap capture persistent
```

Habilita la persistencia de captura TAP.

**Ejemplo de uso:**
```bash
# Habilitar persistencia
set tap capture persistent
```

### no_tap_capture_persistent - Deshabilitar Persistencia de Captura TAP
```
no tap capture persistent
```

Deshabilita la persistencia de captura TAP.

**Ejemplo de uso:**
```bash
# Deshabilitar persistencia
no tap capture persistent
```

### unset_tap_capture_persistent - Eliminar Configuración de Persistencia de Captura TAP
```
unset tap capture persistent
```

Elimina la configuración de persistencia de captura TAP.

**Ejemplo de uso:**
```bash
# Eliminar configuración de persistencia
unset tap capture persistent
```

## Resumen de Interfaz TAP

### Qué es una Interfaz TAP
La interfaz TAP (Network TAP) es una interfaz de red virtual utilizada para monitorizar y probar tráfico de red.

### Funciones Principales
- **Captura de Paquetes** - Captura de tráfico de red
- **Inyección de Paquetes** - Inyección de paquetes de prueba
- **Funcionalidad de Puente** - Puente entre diferentes redes
- **Funcionalidad de Monitorización** - Monitorización y análisis de tráfico

### Usos en sdplane
- **Depuración** - Depuración de flujo de paquetes
- **Pruebas** - Pruebas de funciones de red
- **Monitorización** - Monitorización de tráfico
- **Desarrollo** - Desarrollo y pruebas de nuevas funciones

## Configuración de Interfaz TAP

### Procedimiento de Configuración Básico
1. **Crear Interfaz TAP**
```bash
# Crear interfaz TAP a nivel de sistema
sudo ip tuntap add tap0 mode tap
sudo ip link set tap0 up
```

2. **Configuración en sdplane**
```bash
# Configurar nombre de interfaz de captura TAP
set tap capture ifname tap0

# Habilitar persistencia
set tap capture persistent
```

3. **Configurar Worker de Manejador TAP**
```bash
# Configurar worker de manejador TAP
set worker lcore 2 tap-handler
start worker lcore 2
```

### Ejemplos de Configuración

#### Configuración TAP Básica
```bash
# Configuración de interfaz TAP
set tap capture ifname tap0
set tap capture persistent

# Configuración de worker
set worker lcore 2 tap-handler
start worker lcore 2

# Verificar configuración
show worker
```

#### Configuración de Múltiples Interfaces TAP
```bash
# Configurar múltiples interfaces TAP
set tap capture ifname tap0
set tap capture ifname tap1

# Habilitar persistencia
set tap capture persistent
```

## Funcionalidad de Persistencia

### Qué es la Persistencia
Al habilitar la funcionalidad de persistencia, la configuración de la interfaz TAP se mantiene después del reinicio del sistema.

### Ventajas de la Persistencia
- **Conservación de Configuración** - La configuración permanece efectiva después del reinicio
- **Recuperación Automática** - Recuperación automática de fallos del sistema
- **Eficiencia Operativa** - Reducción de configuración manual

### Configuración de Persistencia
```bash
# Habilitar persistencia
set tap capture persistent

# Deshabilitar persistencia
no tap capture persistent

# Eliminar configuración de persistencia
unset tap capture persistent
```

## Ejemplos de Uso

### Uso para Depuración
```bash
# Configurar interfaz TAP para depuración
set tap capture ifname debug-tap
set tap capture persistent

# Configurar worker de manejador TAP
set worker lcore 3 tap-handler
start worker lcore 3

# Iniciar captura de paquetes
tcpdump -i debug-tap
```

### Uso para Pruebas
```bash
# Configurar interfaz TAP para pruebas
set tap capture ifname test-tap
set tap capture persistent

# Preparar inyección de paquetes de prueba
set worker lcore 4 tap-handler
start worker lcore 4
```

## Monitorización y Gestión

### Verificar Estado de Interfaz TAP
```bash
# Verificar estado de workers
show worker

# Verificar información de hilos
show thread

# Verificación a nivel de sistema
ip link show tap0
```

### Monitorización de Tráfico
```bash
# Monitorización usando tcpdump
tcpdump -i tap0

# Monitorización usando Wireshark
wireshark -i tap0
```

## Solución de Problemas

### Cuando no se Crea la Interfaz TAP
1. Verificación a nivel de sistema
```bash
# Verificar existencia de interfaz TAP
ip link show tap0

# Verificar permisos
sudo ip tuntap add tap0 mode tap
```

2. Verificación en sdplane
```bash
# Verificar configuración
show worker

# Verificar estado de workers
show thread
```

### Cuando los Paquetes no se Capturan
1. Verificar estado de interfaz
```bash
ip link show tap0
```

2. Verificar estado de workers
```bash
show worker
```

3. Reiniciar manejador TAP
```bash
restart worker lcore 2
```

### Cuando la Persistencia no Funciona
1. Verificar configuración de persistencia
```bash
# Verificar configuración actual (verificar con comandos show)
show worker
```

2. Verificar configuración del sistema
```bash
# Verificar configuración a nivel de sistema
systemctl status sdplane
```

## Funciones Avanzadas

### Cooperación con VLAN
La interfaz TAP puede usarse en cooperación con funcionalidad VLAN:
```bash
# Cooperación con worker de conmutador VLAN
set worker lcore 5 vlan-switch
start worker lcore 5
```

### Funcionalidad de Puente
Uso con puente de múltiples interfaces TAP:
```bash
# Configurar múltiples interfaces TAP
set tap capture ifname tap0
set tap capture ifname tap1
```

## Ubicación de Definición

Estos comandos están definidos en los siguientes archivos:
- `sdplane/tap_cmd.c`

## Elementos Relacionados

- [Gestión de Workers y lcore](worker-lcore-thread-management.md)
- [Gestión VTY y Shell](vty-shell.md)
- [Depuración y Registros](debug-logging.md)