# Construcción e Instalación desde Código Fuente

**Language:** [English](../en/build-install-source.md) | [Japanese](../ja/build-install-source.md) | [Français](../fr/build-install-source.md) | [中文](../zh/build-install-source.md) | [Deutsch](../de/build-install-source.md) | [Italiano](../it/build-install-source.md) | [한국어](../ko/build-install-source.md) | [ไทย](../th/build-install-source.md) | **Español**

**En general, por favor sigue este procedimiento.**

## Instalación de Paquetes Ubuntu Prerrequisitos

### Para Construcción desde Código Fuente
```bash
# Herramientas de construcción esenciales
sudo apt install build-essential cmake autotools-dev autoconf automake libtool pkg-config

# Prerrequisitos de DPDK
sudo apt install python3 python3-pip meson ninja-build python3-pyelftools libnuma-dev pkgconf
```

### Paquetes Opcionales
```bash
sudo apt install etckeeper tig bridge-utils \
                 iptables-persistent fail2ban dmidecode screen ripgrep
```

## Construcción de sdplane-oss desde Código Fuente

```bash
# Clonar el repositorio
git clone https://github.com/kait-cronos/sdplane-oss
cd sdplane-oss

# Generar archivos de construcción
sh autogen.sh

# Configurar y compilar
mkdir build
cd build
CFLAGS="-g -O0" sh ../configure
make

# Instalar en el sistema
# Esto instalará en $prefix (predeterminado: /usr/local/sbin)
sudo make install
```