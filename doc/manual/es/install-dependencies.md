# Instalación de Dependencias

**Language:** [English](../en/install-dependencies.md) | [Japanese](../ja/install-dependencies.md) | [Français](../fr/install-dependencies.md) | [中文](../zh/install-dependencies.md) | [Deutsch](../de/install-dependencies.md) | [Italiano](../it/install-dependencies.md) | [한국어](../ko/install-dependencies.md) | [ไทย](../th/install-dependencies.md) | **Español**

## Dependencias

sdplane-oss requiere los siguientes componentes:
- **lthread** (yasuhironet/lthread): Threading cooperativo ligero
- **liburcu-qsbr**: Librería RCU de espacio de usuario  
- **libpcap**: Librería de captura de paquetes
- **DPDK 23.11.1**: Data Plane Development Kit

## Instalación de paquetes debian de dependencias de sdplane

```bash
sudo apt update && sudo apt install liburcu-dev libpcap-dev
```

## Instalación de herramientas de construcción y prerrequisitos de DPDK

```bash
sudo apt install build-essential cmake autotools-dev autoconf automake \
                 libtool pkg-config python3 python3-pip meson ninja-build \
                 python3-pyelftools libnuma-dev pkgconf
```

## Instalación de lthread

```bash
git clone https://github.com/yasuhironet/lthread
cd lthread
cmake .
make
sudo make install
cd ..
```

## Instalación de DPDK 23.11.1

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