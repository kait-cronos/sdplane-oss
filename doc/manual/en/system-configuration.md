# System Configuration

**Language:** **English** | [Japanese](../ja/system-configuration.md) | [Français](../fr/system-configuration.md) | [中文](../zh/system-configuration.md) | [Deutsch](../de/system-configuration.md) | [Italiano](../it/system-configuration.md) | [한국어](../ko/system-configuration.md) | [ไทย](../th/system-configuration.md) | [Español](../es/system-configuration.md)

- **Hugepages**: Configure system hugepages for DPDK
- **Network**: Use netplan for network interface configuration
- **Firewall**: telnet 9882/tcp port is required for CLI

**⚠️ The CLI has no authentication. It is recommended to allow connections only from localhost ⚠️**

## Configure Hugepages
```bash
# Edit GRUB configuration
sudo vi /etc/default/grub

# Add one of the following lines:
# For 2MB hugepages (1536 pages = ~3GB):
GRUB_CMDLINE_LINUX="hugepages=1536"

# Or for 1GB hugepages (8 pages = 8GB):
GRUB_CMDLINE_LINUX="default_hugepagesz=1G hugepagesz=1G hugepages=8"

# Update GRUB and reboot
sudo update-grub
sudo reboot
```

## Install DPDK IGB Kernel Module (Optional)

For NICs that do not work with vfio-pci, optionally install igb_uio:

```bash
git clone http://dpdk.org/git/dpdk-kmods
cd dpdk-kmods/linux/igb_uio
make
sudo mkdir -p /lib/modules/`uname -r`/extra/dpdk/
sudo cp igb_uio.ko /lib/modules/`uname -r`/extra/dpdk/
echo igb_uio | sudo tee /etc/modules-load.d/igb_uio.conf
```

## DPDK UIO Driver Setup

DPDK requires userspace I/O (UIO) drivers to access network interfaces from userspace applications.

### Standard NICs

For most standard network interface cards, you have three UIO driver options:

1. **vfio-pci** (Recommended)
   - Most secure and modern option
   - Requires IOMMU support (Intel VT-d or AMD-Vi)
   - No additional kernel module compilation needed

2. **uio_pci_generic** 
   - Generic UIO driver included in the Linux kernel
   - No additional installation required
   - Limited functionality compared to vfio-pci

3. **igb_uio**
   - DPDK-specific UIO driver
   - Requires manual compilation and installation (see above)
   - Provides additional features for older hardware

### Bifurcated Driver NICs

Some NICs, such as **Mellanox ConnectX series**, provide bifurcated drivers that allow the same physical port to be used by both kernel networking and DPDK applications simultaneously. For these NICs:

- No UIO driver configuration is required
- The NIC can remain bound to its kernel driver
- DPDK applications can directly access the hardware

### UIO Driver Configuration

To configure UIO drivers, you need to:

1. **Load the driver module**:
```bash
# For vfio-pci (requires IOMMU enabled in BIOS/UEFI and kernel)
sudo modprobe vfio-pci

# For uio_pci_generic
sudo modprobe uio_pci_generic

# For igb_uio (after installation)
sudo modprobe igb_uio
```

2. **Make the driver load at boot** by creating a configuration file in `/etc/modules-load.d/`:
```bash
# Example for vfio-pci
echo "vfio-pci" | sudo tee /etc/modules-load.d/vfio-pci.conf

# Example for uio_pci_generic  
echo "uio_pci_generic" | sudo tee /etc/modules-load.d/uio_pci_generic.conf

# Example for igb_uio
echo "igb_uio" | sudo tee /etc/modules-load.d/igb_uio.conf
```

3. **Bind NICs to the UIO driver** using DPDK's `dpdk-devbind.py` script:
```bash
# Unbind from kernel driver and bind to UIO driver
sudo dpdk-devbind.py --bind=vfio-pci 0000:01:00.0
sudo dpdk-devbind.py --bind=uio_pci_generic 0000:01:00.1  
sudo dpdk-devbind.py --bind=igb_uio 0000:01:00.2
```

**Note**: Replace `0000:01:00.0` with your actual PCI device address. Use `lspci` or `dpdk-devbind.py --status` to identify your NICs.

**Alternative**: Instead of using `dpdk-devbind.py` manually, you can configure device bindings in your `sdplane.conf` file using `set device XX:XX.X driver ...` commands at the beginning of the configuration file. This allows sdplane to automatically handle the device binding during startup.