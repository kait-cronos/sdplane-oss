# System Configuration

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