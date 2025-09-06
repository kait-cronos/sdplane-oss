# Gestión de Puertos y Estadísticas

**Idioma / Language:** [English](../port-management.md) | [日本語](../ja/port-management.md) | [Français](../fr/port-management.md) | [中文](../zh/port-management.md) | [Deutsch](../de/port-management.md) | [Italiano](../it/port-management.md) | [한국어](../ko/port-management.md) | [ไทย](../th/port-management.md) | **Español**

Comandos para manejar puertos DPDK y gestionar información estadística.

## Lista de Comandos

### **start port**

Inicia puertos DPDK (operación por defecto).

**Ejemplo de uso:**
```bash
# Iniciar puerto (por defecto)
start port
```

---

### **start port \<0-16\>**

Inicia un puerto DPDK específico.

**Ejemplo de uso:**
```bash
# Iniciar puerto 0
start port 0

# Iniciar puerto 1
start port 1
```

---

### **start port all**

Inicia todos los puertos DPDK.

**Ejemplo de uso:**
```bash
# Iniciar todos los puertos
start port all
```

---

### **stop port**

Detiene puertos DPDK (operación por defecto).

**Ejemplo de uso:**
```bash
# Detener puerto (por defecto)
stop port
```

---

### **stop port \<0-16\>**

Detiene un puerto DPDK específico.

**Ejemplo de uso:**
```bash
# Detener puerto 0
stop port 0

# Detener puerto 1
stop port 1
```

---

### **stop port all**

Detiene todos los puertos DPDK.

**Ejemplo de uso:**
```bash
# Detener todos los puertos
stop port all
```

---

### **reset port**

Reinicia puertos DPDK (operación por defecto).

**Ejemplo de uso:**
```bash
# Reiniciar puerto (por defecto)
reset port
```

---

### **reset port \<0-16\>**

Reinicia un puerto DPDK específico.

**Ejemplo de uso:**
```bash
# Reiniciar puerto 0
reset port 0

# Reiniciar puerto 1
reset port 1
```

---

### **reset port all**

Reinicia todos los puertos DPDK.

**Ejemplo de uso:**
```bash
# Reiniciar todos los puertos
reset port all
```

---

### **show port**

Muestra información básica de todos los puertos (operación por defecto).

**Ejemplo de uso:**
```bash
# Mostrar información de todos los puertos
show port
```

---

### **show port \<0-16\>**

Muestra información básica de un puerto específico.

**Ejemplo de uso:**
```bash
# Mostrar información del puerto 0
show port 0

# Mostrar información del puerto 1
show port 1
```

---

### **show port all**

Muestra explícitamente información de todos los puertos.

**Ejemplo de uso:**
```bash
# Mostrar explícitamente información de todos los puertos
show port all
```

---

### **show port statistics**

Muestra todas las estadísticas de puertos.

**Ejemplo de uso:**
```bash
# Mostrar todas las estadísticas
show port statistics
```

---

### **show port statistics pps**

Muestra estadísticas de paquetes por segundo.

**Ejemplo de uso:**
```bash
# Mostrar solo estadísticas PPS
show port statistics pps
```

---

### **show port statistics total**

Muestra estadísticas de número total de paquetes.

**Ejemplo de uso:**
```bash
# Mostrar número total de paquetes
show port statistics total
```

---

### **show port statistics bps**

Muestra estadísticas de bits por segundo.

**Ejemplo de uso:**
```bash
# Mostrar bits por segundo
show port statistics bps
```

---

### **show port statistics Bps**

Muestra estadísticas de bytes por segundo.

**Ejemplo de uso:**
```bash
# Mostrar bytes por segundo
show port statistics Bps
```

---

### **show port statistics total-bytes**

Muestra estadísticas de número total de bytes.

**Ejemplo de uso:**
```bash
# Mostrar número total de bytes
show port statistics total-bytes
```

---

### **show port \<0-16\> promiscuous**

Muestra el estado del modo promiscuo del puerto especificado.

**Ejemplo de uso:**
```bash
# Mostrar modo promiscuo del puerto 0
show port 0 promiscuous

# Mostrar modo promiscuo del puerto 1
show port 1 promiscuous
```

---

### **show port all promiscuous**

Muestra el estado del modo promiscuo de todos los puertos.

**Ejemplo de uso:**
```bash
# Mostrar modo promiscuo de todos los puertos
show port all promiscuous
```

---

### **show port \<0-16\> flowcontrol**

Muestra la configuración de control de flujo del puerto especificado.

**Ejemplo de uso:**
```bash
# Mostrar configuración de control de flujo del puerto 0
show port 0 flowcontrol

# Mostrar configuración de control de flujo del puerto 1
show port 1 flowcontrol
```

---

### **show port all flowcontrol**

Muestra la configuración de control de flujo de todos los puertos.

**Ejemplo de uso:**
```bash
# Mostrar configuración de control de flujo de todos los puertos
show port all flowcontrol
```

---

### **set port \<0-16\> promiscuous enable**

Habilita el modo promiscuo del puerto especificado.

**Ejemplo de uso:**
```bash
# Habilitar modo promiscuo del puerto 0
set port 0 promiscuous enable

# Habilitar modo promiscuo del puerto 1
set port 1 promiscuous enable
```

---

### **set port \<0-16\> promiscuous disable**

Deshabilita el modo promiscuo del puerto especificado.

**Ejemplo de uso:**
```bash
# Deshabilitar modo promiscuo del puerto 0
set port 0 promiscuous disable

# Deshabilitar modo promiscuo del puerto 1
set port 1 promiscuous disable
```

---

### **set port all promiscuous enable**

Habilita el modo promiscuo de todos los puertos.

**Ejemplo de uso:**
```bash
# Habilitar modo promiscuo de todos los puertos
set port all promiscuous enable
```

---

### **set port all promiscuous disable**

Deshabilita el modo promiscuo de todos los puertos.

**Ejemplo de uso:**
```bash
# Deshabilitar modo promiscuo de todos los puertos
set port all promiscuous disable
```

---

### **set port \<0-16\> flowcontrol (rx|tx|autoneg|send-xon|fwd-mac-ctrl) (on|off)**

Cambia la configuración de control de flujo del puerto especificado.

**Opciones:**
- `rx` - Control de flujo de recepción
- `tx` - Control de flujo de transmisión
- `autoneg` - Autonegociación
- `send-xon` - Envío XON
- `fwd-mac-ctrl` - Reenvío de tramas de control MAC

**Ejemplo de uso:**
```bash
# Habilitar control de flujo de recepción del puerto 0
set port 0 flowcontrol rx on

# Deshabilitar autonegociación del puerto 1
set port 1 flowcontrol autoneg off
```

---

### **set port all flowcontrol (rx|tx|autoneg|send-xon|fwd-mac-ctrl) (on|off)**

Cambia la configuración de control de flujo de todos los puertos.

**Opciones:**
- `rx` - Control de flujo de recepción
- `tx` - Control de flujo de transmisión
- `autoneg` - Autonegociación
- `send-xon` - Envío XON
- `fwd-mac-ctrl` - Reenvío de tramas de control MAC

**Ejemplo de uso:**
```bash
# Deshabilitar autonegociación de todos los puertos
set port all flowcontrol autoneg off

# Habilitar control de flujo de transmisión de todos los puertos
set port all flowcontrol tx on
```

---

### **set port \<0-16\> dev-configure \<0-64\> \<0-64\>**


Configura el dispositivo DPDK del puerto especificado.

**Parámetros:**
- 1er argumento: Número de colas de recepción (0-64)
- 2do argumento: Número de colas de transmisión (0-64)

**Ejemplo de uso:**
```bash
# Configurar puerto 0 con 4 colas de recepción, 4 colas de transmisión
set port 0 dev-configure 4 4

# Configurar puerto 1 con 2 colas de recepción, 2 colas de transmisión
set port 1 dev-configure 2 2
```

---

### **set port all dev-configure \<0-64\> \<0-64\>**

Configura el dispositivo DPDK de todos los puertos.

**Parámetros:**
- 1er argumento: Número de colas de recepción (0-64)
- 2do argumento: Número de colas de transmisión (0-64)

**Ejemplo de uso:**
```bash
# Configurar todos los puertos con 1 cola de recepción, 1 cola de transmisión
set port all dev-configure 1 1
```

---

### **set port \<0-16\> nrxdesc \<0-16384\>**


Configura el número de descriptores de recepción del puerto especificado.

**Ejemplo de uso:**
```bash
# Configurar 1024 descriptores de recepción para el puerto 0
set port 0 nrxdesc 1024

# Configurar 512 descriptores de recepción para el puerto 1
set port 1 nrxdesc 512
```

---

### **set port all nrxdesc \<0-16384\>**

Configura el número de descriptores de recepción de todos los puertos.

**Ejemplo de uso:**
```bash
# Configurar 512 descriptores de recepción para todos los puertos
set port all nrxdesc 512
```

---

### **set port \<0-16\> ntxdesc \<0-16384\>**


Configura el número de descriptores de transmisión del puerto especificado.

**Ejemplo de uso:**
```bash
# Configurar 1024 descriptores de transmisión para el puerto 0
set port 0 ntxdesc 1024

# Configurar 512 descriptores de transmisión para el puerto 1
set port 1 ntxdesc 512
```

---

### **set port all ntxdesc \<0-16384\>**

Configura el número de descriptores de transmisión de todos los puertos.

**Ejemplo de uso:**
```bash
# Configurar 512 descriptores de transmisión para todos los puertos
set port all ntxdesc 512
```

---

### **set port \<0-16\> link up**


Activa el enlace del puerto especificado.

**Ejemplo de uso:**
```bash
# Activar enlace del puerto 0
set port 0 link up

# Activar enlace del puerto 1
set port 1 link up
```

---

### **set port \<0-16\> link down**

Desactiva el enlace del puerto especificado.

**Ejemplo de uso:**
```bash
# Desactivar enlace del puerto 0
set port 0 link down

# Desactivar enlace del puerto 1
set port 1 link down
```

---

### **set port all link up**

Activa el enlace de todos los puertos.

**Ejemplo de uso:**
```bash
# Activar enlace de todos los puertos
set port all link up
```

---

### **set port all link down**

Desactiva el enlace de todos los puertos.

**Ejemplo de uso:**
```bash
# Desactivar enlace de todos los puertos
set port all link down
```

## Ubicación de Definición

Estos comandos están definidos en los siguientes archivos:
- `sdplane/dpdk_port_cmd.c`

## Elementos Relacionados

- [Gestión de Workers y lcore](worker-management.md)
- [Información del Sistema y Monitorización](system-monitoring.md)
- [Configuración de Colas](queue-configuration.md)