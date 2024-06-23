
ip link set down enp2s0
ip link set down enp3s0
modprobe uio_pci_generic
dpdk-devbind.py -b uio_pci_generic 02:00.0 03:00.0

