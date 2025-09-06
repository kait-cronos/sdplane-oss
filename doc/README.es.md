<div align="center">
<img src="../sdplane-logo.png" alt="sdplane-oss Logo" width="160">
</div>

# sdplane-oss (Plano de Datos de Software)

Un "Entorno de Desarrollo DPDK-dock" que consta de una shell interactiva que puede controlar las operaciones de hilos DPDK y un entorno de ejecución de hilos DPDK (sd-plane)

**Language:** [English](../README.md) | [日本語](README.ja.md) | [Français](README.fr.md) | [中文](README.zh.md) | [Deutsch](README.de.md) | [Italiano](README.it.md) | [한국어](README.ko.md) | [ไทย](README.th.md) | **Español**

## Características

- **Procesamiento de Paquetes de Alto Rendimiento**:
  Procesamiento de paquetes zero-copy en espacio de usuario utilizando DPDK
- **Reenvío de Capa 2/3**:
  Reenvío L2 y L3 integrado con soporte ACL, LPM y FIB
- **Generación de Paquetes**:
  Generador de paquetes integrado para pruebas y benchmarking
- **Virtualización de Red**:
  Soporte de interfaz TAP y capacidades de conmutación VLAN
- **Gestión CLI**:
  Interfaz de línea de comandos interactiva para configuración y monitoreo
- **Multi-hilo**:
  Modelo de hilos cooperativo con workers por núcleo

### Arquitectura
- **Aplicación Principal**: Lógica del router central e inicialización
- **Módulos DPDK**: Reenvío L2/L3 y generación de paquetes
- **Sistema CLI**: Interfaz de línea de comandos con completado y ayuda
- **Hilos**: Multitarea cooperativa basada en lthread
- **Virtualización**: Interfaces TAP y conmutación virtual

## Sistemas Soportados

### Requisitos de Software
- **SO**:
  Ubuntu 24.04 LTS (actualmente soportado)
- **NIC**:
  [Controladores](https://doc.dpdk.org/guides/nics/) | [NIC Soportadas](https://core.dpdk.org/supported/)
- **Memoria**:
  Se requiere soporte de hugepage
- **CPU**:
  Se recomienda procesador multi-núcleo

### Plataformas de Hardware Objetivo

El proyecto ha sido probado en:
- **Topton (N305/N100)**: Mini-PC con NIC de 10G
- **Partaker (J3160)**: Mini-PC con NIC de 1G
- **PC Intel Genérico**: Con Intel x520 / Mellanox ConnectX5
- **Otras CPU**: Debería funcionar con procesadores AMD, ARM, etc.

## 1. Instalación de Dependencias

### Dependencias

sdplane-oss requiere los siguientes componentes:
- **lthread** (yasuhironet/lthread): Hilos cooperativos ligeros
- **liburcu-qsbr**: Biblioteca RCU de espacio de usuario  
- **libpcap**: Biblioteca de captura de paquetes
- **DPDK 23.11.1**: Kit de Desarrollo de Plano de Datos

### Instalar paquetes debian de dependencias sdplane

```bash
sudo apt update && sudo apt install liburcu-dev libpcap-dev
```

### Instalar Herramientas de Compilación y Prerrequisitos DPDK

```bash
sudo apt install build-essential cmake autotools-dev autoconf automake \
                 libtool pkg-config python3 python3-pip meson ninja-build \
                 python3-pyelftools libnuma-dev pkgconf
```

### Instalar lthread

```bash
git clone https://github.com/yasuhironet/lthread
cd lthread
cmake .
make
sudo make install
cd ..
```

### Instalar DPDK 23.11.1

```bash
# Descargar DPDK 23.11.1
wget https://fast.dpdk.org/rel/dpdk-23.11.1.tar.xz
tar xf dpdk-23.11.1.tar.xz
cd dpdk-23.11.1

# Compilar e instalar DPDK
meson setup -Dprefix=/usr/local build
cd build
ninja install
cd ../..

# Verificar instalación
pkg-config --modversion libdpdk
# Debería mostrar: 23.11.1
```

## 2. Inicio Rápido con Paquete Debian para Intel Core i3-n305/Celeron j3160

Para Intel Core i3-n305/Celeron j3160, es posible una instalación rápida con paquetes Debian.

Descargar e instalar el paquete Debian precompilado:

```bash
# Descargar el paquete más reciente para n305
wget https://www.yasuhironet.net/download/n305/sdplane_0.1.4-36_amd64.deb
wget https://www.yasuhironet.net/download/n305/sdplane-dbgsym_0.1.4-36_amd64.ddeb

# o para j3160
wget https://www.yasuhironet.net/download/j3160/sdplane_0.1.4-35_amd64.deb
wget https://www.yasuhironet.net/download/j3160/sdplane-dbgsym_0.1.4-35_amd64.ddeb

# Instalar el paquete
sudo apt install ./sdplane_0.1.4-*_amd64.deb
sudo apt install ./sdplane-dbgsym_0.1.4-*_amd64.ddeb
```

**Nota**: Consultar [descargas yasuhironet.net](https://www.yasuhironet.net/download/) para la versión de paquete más reciente.

Saltar a 5. Configuración del Sistema.

## 3. Compilación desde el Código Fuente

**En general, siga este procedimiento.**

### Instalar Paquetes de Ubuntu Prerrequisitos

#### Para Compilación desde el Código Fuente
```bash
# Herramientas de compilación esenciales
sudo apt install build-essential cmake autotools-dev autoconf automake libtool pkg-config

# Prerrequisitos DPDK
sudo apt install python3 python3-pip meson ninja-build python3-pyelftools libnuma-dev pkgconf
```

#### Paquetes Opcionales
```bash
sudo apt install etckeeper tig bridge-utils \
                 iptables-persistent fail2ban dmidecode screen ripgrep
```

### Compilar sdplane-oss desde el Código Fuente

```bash
# Clonar el repositorio
git clone https://github.com/kait-cronos/sdplane-oss
cd sdplane-oss

# Generar archivos de compilación
sh autogen.sh

# Configurar y compilar
mkdir build
cd build
CFLAGS="-g -O0" sh ../configure
make
```

## 4. Crear e Instalar Paquete Debian sdplane-oss

### Instalar paquetes prerrequisitos
```bash
sudo apt install build-essential cmake devscripts debhelper
```

### Compilar Paquete Debian sdplane-oss
```bash
# Primero asegurarse de empezar en un espacio limpio
(cd build && make distclean)
make distclean

# Compilar paquete Debian desde código fuente
bash build-debian.sh

# Instalar el paquete generado (se producirá en el directorio padre)
sudo apt install ../sdplane_*.deb
```

## 5. Configuración del Sistema

- **Hugepages**: Configurar hugepages del sistema para DPDK
- **Red**: Usar netplan para configuración de interfaz de red
- **Firewall**: se requiere puerto telnet 9882/tcp para CLI

**⚠️ El CLI no tiene autenticación. Se recomienda permitir conexiones solo desde localhost ⚠️**

### Configurar Hugepages
```bash
# Editar configuración GRUB
sudo vi /etc/default/grub

# Agregar hugepages al parámetro GRUB_CMDLINE_LINUX
# Ejemplo de agregar hugepages=1024:
GRUB_CMDLINE_LINUX="hugepages=1024"

# Actualizar GRUB
sudo update-grub

# Reiniciar el sistema
sudo reboot

# Verificar hugepages después del reinicio
cat /proc/meminfo | grep -E "^HugePages|^Hugepagesize"
```

### Instalación opcional del Módulo Kernel IGB DPDK

Si su NIC no funciona con vfio-pci, instale igb_uio.

```bash
git clone http://dpdk.org/git/dpdk-kmods
cd dpdk-kmods/linux/igb_uio
make
sudo make install
cd ../../..

# El módulo se instalará en /lib/modules/$(uname -r)/extra/igb_uio.ko
```

## 6. Configuración sdplane

### Archivos de Configuración

sdplane utiliza archivos de configuración para definir el comportamiento de inicio y el entorno de ejecución.

#### Ejemplos de Configuración del SO (`etc/`)
- `systemd/sdplane.service`: Archivo de servicio systemd
- `netplan/01-netcfg.yaml`: Configuración de red con netplan

#### Ejemplos de Configuración de Aplicación (`example-config/`)
- Archivos de configuración ejemplo para varias aplicaciones
- Scripts de inicio y perfiles de configuración

## 7. Ejecutar Aplicaciones usando sdplane-oss

```bash
# Ejecución básica
sudo ./sdplane/sdplane

# Ejecutar con archivo de configuración
sudo ./sdplane/sdplane -f /path/to/config-file

# Conectar al CLI (desde otro terminal)
telnet localhost 9882

# La shell interactiva está ahora disponible
sdplane> help
sdplane> show version
```

## Consejos

### Se requiere IOMMU cuando se usa vfio-pci como controlador NIC

Las capacidades de virtualización deben estar habilitadas:
- Intel: Intel VT-d
- AMD: AMD IOMMU / AMD-V

Estas opciones deben estar habilitadas en la configuración del BIOS.
La configuración GRUB también puede necesitar ser cambiada:

```conf
# /etc/default/grub
GRUB_CMDLINE_LINUX="iommu=pt intel_iommu=on"
```

Aplicar los cambios:
```bash
sudo update-grub
sudo reboot
```

### Configuración para cargar permanentemente el módulo kernel Linux vfio-pci

```bash
# Crear archivo de configuración para carga automática
sudo tee /etc/modules-load.d/vfio-pci.conf > /dev/null <<EOF
vfio-pci
EOF
```

### Para Serie Mellanox ConnectX

Se requiere instalación del controlador desde el siguiente enlace:

https://network.nvidia.com/products/ethernet-drivers/linux/mlnx_en/

Durante la instalación, ejecute `./install --dpdk`.
**La opción `--dpdk` es obligatoria.**

Comente las siguientes configuraciones en sdplane.conf ya que no son necesarias:

```conf
#set device {pcie-id} driver unbind
#set device {pcie-id} driver {driver-name} driver_override  
#set device {pcie-id} driver {driver-name} bind
```

Para NIC Mellanox, necesita ejecutar el comando `update port status` para actualizar la información de puerto.

### Cómo verificar números de bus PCIe

Puede usar el comando dpdk-devbind.py en DPDK para verificar los números de bus PCIe de las NIC:

```bash
# Mostrar estado de dispositivos de red  
dpdk-devbind.py -s

# Ejemplo de salida:
Network devices using kernel driver
===================================
0000:04:00.0 'NetXtreme BCM5720 Gigabit Ethernet PCIe 165f' numa_node=0 if=eno8303 drv=tg3 unused= *Active*
0000:b1:00.0 'MT27800 Family [ConnectX-5] 1017' numa_node=1 if=enp177s0np0 drv=mlx5_core unused= *Active*
```

### Orden de hilos worker en archivo de configuración

Si configura workers `rib-manager`, `neigh-manager` y `netlink-thread`, deben configurarse en este orden si se usan.

### Inicialización DPDK

Solo un comando que llame a `rte_eal_init()` debe invocarse desde el archivo de configuración. La función `rte_eal_init()` es llamada por comandos como `rte_eal_init`, `pktgen init`, `l2fwd init` y `l3fwd init`.

## Guía del Usuario (Manual)

Guías de usuario completas y referencias de comandos están disponibles:

- [Guía del Usuario](manual/es/README.md) - Visión general completa y clasificación de comandos

**Guías de Aplicación:**
- [Aplicación Repetidor L2](manual/es/l2-repeater-application.md) - Reenvío simple de paquetes Capa 2 con aprendizaje MAC
- [Aplicación Repetidor Mejorado](manual/es/enhanced-repeater-application.md) - Conmutación consciente de VLAN con interfaces TAP  
- [Aplicación Generador de Paquetes](manual/es/packet-generator-application.md) - Generación de tráfico de alto rendimiento y pruebas

**Guías de Configuración:**
- [RIB & Enrutamiento](manual/es/routing.md) - Funciones RIB y enrutamiento
- [Repetidor Mejorado](manual/es/enhanced-repeater.md) - Configuración repetidor mejorado
- [Gestión de Puertos & Estadísticas](manual/es/port-management.md) - Gestión de puertos DPDK y estadísticas
- [Gestión Worker & lcore & Información Hilos](manual/es/worker-lcore-thread-management.md) - Gestión de worker threads, lcore e información de hilos
- [Información del Sistema & Monitoreo](manual/es/system-monitoring.md) - Información del sistema y monitoreo
- [Gestión de Dispositivos](manual/es/device-management.md) - Gestión de dispositivos y controladores

**Guías de Desarrollador:**
- [Guía de Integración DPDK](manual/es/dpdk-integration-guide.md) - Cómo integrar aplicaciones DPDK
- [Debug & Logging](manual/es/debug-logging.md) - Funciones debug y logging
- [Configuración de Colas](manual/es/queue-configuration.md) - Configuración y gestión de colas
- [Interfaz TAP](manual/es/tap-interface.md) - Gestión de interfaz TAP
- [Gestión VTY & Shell](manual/es/vty-shell.md) - Gestión VTY y shell
- [Gestión lthread](manual/es/lthread-management.md) - Gestión de hilos ligeros cooperativos
- [Generación de Paquetes](manual/es/packet-generation.md) - Generación de paquetes usando PKTGEN

## Guía del Desarrollador

### Guía de Integración
- [Guía de Integración de Aplicaciones DPDK](manual/es/dpdk-integration-guide.md) - Cómo integrar sus aplicaciones DPDK con sdplane-oss usando el enfoque DPDK-dock

### Documentación
- Toda la documentación del desarrollador está incluida en `doc/`
- Las guías de integración y ejemplos están en `doc/manual/es/`

### Estilo de Código

Este proyecto sigue los Estándares de Codificación GNU. Para verificar y formatear código:

```bash
# Verificar estilo (requiere clang-format 18.1.3+)
./style/check_gnu_style.sh check

# Formatear código automáticamente
./style/check_gnu_style.sh update  

# Mostrar diferencias
./style/check_gnu_style.sh diff
```

Instalar herramientas requeridas:
```bash
# Para Ubuntu 24.04
sudo apt install clang-format-18
```

## Licencia

Este proyecto está bajo la licencia Apache 2.0 - ver el archivo [LICENSE](LICENSE) para detalles.

## Contacto

- GitHub: https://github.com/kait-cronos/sdplane-oss  
- Issues: https://github.com/kait-cronos/sdplane-oss/issues

## Compra de Equipo de Evaluación

El equipo de evaluación puede incluir características adicionales y modificaciones de software.

Para consultas sobre equipo de evaluación, contáctenos a través de GitHub Issues o directamente por correo electrónico.