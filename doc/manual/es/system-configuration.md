# Configuración del Sistema

**Language:** [English](../en/system-configuration.md) | [Japanese](../ja/system-configuration.md) | [Français](../fr/system-configuration.md) | [中文](../zh/system-configuration.md) | [Deutsch](../de/system-configuration.md) | [Italiano](../it/system-configuration.md) | [한국어](../ko/system-configuration.md) | [ไทย](../th/system-configuration.md) | **Español**

- **Hugepages**: Configurar hugepages del sistema para DPDK
- **Red**: Usar netplan para configuración de interfaces de red
- **Firewall**: Se requiere el puerto telnet 9882/tcp para CLI

**⚠️ El CLI no tiene autenticación. Se recomienda permitir conexiones solo desde localhost ⚠️**

## Configurar Hugepages
```bash
# Editar configuración GRUB
sudo vi /etc/default/grub

# Agregar una de estas líneas:
# Para hugepages de 2MB (1536 páginas = ~3GB):
GRUB_CMDLINE_LINUX="hugepages=1536"

# O para hugepages de 1GB (8 páginas = 8GB):
GRUB_CMDLINE_LINUX="default_hugepagesz=1G hugepagesz=1G hugepages=8"

# Actualizar GRUB y reiniciar
sudo update-grub
sudo reboot
```

## Instalación del Módulo del Kernel DPDK IGB (Opcional)

Para NIC que no funcionan con vfio-pci, instalar opcionalmente igb_uio:

```bash
git clone http://dpdk.org/git/dpdk-kmods
cd dpdk-kmods/linux/igb_uio
make
sudo mkdir -p /lib/modules/`uname -r`/extra/dpdk/
sudo cp igb_uio.ko /lib/modules/`uname -r`/extra/dpdk/
echo igb_uio | sudo tee /etc/modules-load.d/igb_uio.conf
```

## Configurar Drivers UIO de DPDK

DPDK requiere drivers User I/O (UIO) para acceder a interfaces de red desde aplicaciones de usuario.

### NIC Estándar

Para la mayoría de tarjetas de red estándar, tienes tres opciones de drivers UIO:

1. **vfio-pci** (Recomendado)
   - Opción más segura y moderna
   - Requiere soporte IOMMU (Intel VT-d o AMD-Vi)
   - No se requiere compilación adicional de módulos del kernel

2. **uio_pci_generic**
   - Driver UIO genérico incluido en el kernel Linux
   - No se requiere instalación adicional
   - Funcionalidad limitada comparado con vfio-pci

3. **igb_uio**
   - Driver UIO específico de DPDK
   - Requiere compilación e instalación manual (ver arriba)
   - Proporciona características adicionales para hardware más antiguo

### NIC con Drivers Bifurcados

Algunas NIC, como la **serie Mellanox ConnectX**, proporcionan drivers bifurcados que permiten que el mismo puerto físico sea usado simultáneamente por la red del kernel y aplicaciones DPDK. Para estas NIC:

- No se requiere configuración de driver UIO
- La NIC puede permanecer vinculada a su driver del kernel
- Las aplicaciones DPDK pueden acceder directamente al hardware

### Configurar Drivers UIO

Para configurar drivers UIO, necesitas:

1. **Cargar el módulo del driver**:
```bash
# Para vfio-pci (requiere IOMMU habilitado en BIOS/UEFI y kernel)
sudo modprobe vfio-pci

# Para uio_pci_generic
sudo modprobe uio_pci_generic

# Para igb_uio (después de la instalación)
sudo modprobe igb_uio
```

2. **Hacer que el driver se cargue al arranque** creando un archivo de configuración en `/etc/modules-load.d/`:
```bash
# Ejemplo para vfio-pci
echo "vfio-pci" | sudo tee /etc/modules-load.d/vfio-pci.conf

# Ejemplo para uio_pci_generic
echo "uio_pci_generic" | sudo tee /etc/modules-load.d/uio_pci_generic.conf

# Ejemplo para igb_uio
echo "igb_uio" | sudo tee /etc/modules-load.d/igb_uio.conf
```

3. **Vincular NIC al driver UIO** usando el script `dpdk-devbind.py` de DPDK:
```bash
# Desvincular del driver del kernel y vincular al driver UIO
sudo dpdk-devbind.py --bind=vfio-pci 0000:01:00.0
sudo dpdk-devbind.py --bind=uio_pci_generic 0000:01:00.1
sudo dpdk-devbind.py --bind=igb_uio 0000:01:00.2
```

**Nota**: Reemplaza `0000:01:00.0` con la dirección PCI real de tu dispositivo. Usa `lspci` o `dpdk-devbind.py --status` para identificar tus NIC.

**Alternativa**: En lugar de usar `dpdk-devbind.py` manualmente, puedes configurar las vinculaciones de dispositivo en tu archivo `sdplane.conf` usando los comandos `set device XX:XX.X driver ...` al inicio del archivo de configuración. Esto permite que sdplane gestione automáticamente la vinculación de dispositivos al arranque.