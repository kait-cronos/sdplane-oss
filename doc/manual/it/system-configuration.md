# Configurazione Sistema

**Language:** [English](../en/system-configuration.md) | [日本語](../ja/system-configuration.md) | [Français](../fr/system-configuration.md) | [中文](../zh/system-configuration.md) | [Deutsch](../de/system-configuration.md) | **Italiano** | [한국어](../ko/system-configuration.md) | [ไทย](../th/system-configuration.md) | [Español](../es/system-configuration.md)

- **Hugepages**: Configurare hugepages sistema per DPDK
- **Rete**: Utilizzare netplan per configurazione interfacce di rete
- **Firewall**: porta telnet 9882/tcp è richiesta per CLI

**⚠️ Il CLI non ha autenticazione. Si raccomanda di consentire connessioni solo da localhost ⚠️**

## Configurare Hugepages
```bash
# Modificare configurazione GRUB
sudo vi /etc/default/grub

# Aggiungere una di queste righe:
# Per hugepages da 2MB (1536 pagine = ~3GB):
GRUB_CMDLINE_LINUX="hugepages=1536"

# O per hugepages da 1GB (8 pagine = 8GB):
GRUB_CMDLINE_LINUX="default_hugepagesz=1G hugepagesz=1G hugepages=8"

# Aggiornare GRUB e riavviare
sudo update-grub
sudo reboot
```

## Installazione del Modulo Kernel DPDK IGB (Opzionale)

Per NIC che non funzionano con vfio-pci, installare opzionalmente igb_uio:

```bash
git clone http://dpdk.org/git/dpdk-kmods
cd dpdk-kmods/linux/igb_uio
make
sudo mkdir -p /lib/modules/`uname -r`/extra/dpdk/
sudo cp igb_uio.ko /lib/modules/`uname -r`/extra/dpdk/
echo igb_uio | sudo tee /etc/modules-load.d/igb_uio.conf
```

## Configurazione Driver UIO DPDK

DPDK richiede driver User I/O (UIO) per accedere alle interfacce di rete dalle applicazioni utente.

### NIC Standard

Per la maggior parte delle schede di rete standard, avete tre opzioni di driver UIO:

1. **vfio-pci** (Raccomandato)
   - Opzione più sicura e moderna
   - Richiede supporto IOMMU (Intel VT-d o AMD-Vi)
   - Nessuna compilazione aggiuntiva del modulo kernel richiesta

2. **uio_pci_generic**
   - Driver UIO generico incluso nel kernel Linux
   - Nessuna installazione aggiuntiva richiesta
   - Funzionalità limitata rispetto a vfio-pci

3. **igb_uio**
   - Driver UIO specifico per DPDK
   - Richiede compilazione e installazione manuale (vedi sopra)
   - Fornisce funzionalità aggiuntive per hardware più vecchio

### NIC con Driver Biforcati

Alcune NIC, come la **serie Mellanox ConnectX**, forniscono driver biforcati che permettono alla stessa porta fisica di essere utilizzata simultaneamente dalla rete kernel e dalle applicazioni DPDK. Per queste NIC:

- Nessuna configurazione driver UIO richiesta
- La NIC può rimanere legata al suo driver kernel
- Le applicazioni DPDK possono accedere direttamente all'hardware

### Configurare Driver UIO

Per configurare i driver UIO, è necessario:

1. **Caricare il modulo driver**:
```bash
# Per vfio-pci (richiede IOMMU abilitato in BIOS/UEFI e kernel)
sudo modprobe vfio-pci

# Per uio_pci_generic
sudo modprobe uio_pci_generic

# Per igb_uio (dopo l'installazione)
sudo modprobe igb_uio
```

2. **Far caricare il driver all'avvio** creando un file di configurazione in `/etc/modules-load.d/`:
```bash
# Esempio per vfio-pci
echo "vfio-pci" | sudo tee /etc/modules-load.d/vfio-pci.conf

# Esempio per uio_pci_generic
echo "uio_pci_generic" | sudo tee /etc/modules-load.d/uio_pci_generic.conf

# Esempio per igb_uio
echo "igb_uio" | sudo tee /etc/modules-load.d/igb_uio.conf
```

3. **Legare le NIC al driver UIO** utilizzando lo script `dpdk-devbind.py` di DPDK:
```bash
# Slegare dal driver kernel e legare al driver UIO
sudo dpdk-devbind.py --bind=vfio-pci 0000:01:00.0
sudo dpdk-devbind.py --bind=uio_pci_generic 0000:01:00.1
sudo dpdk-devbind.py --bind=igb_uio 0000:01:00.2
```

**Nota**: Sostituire `0000:01:00.0` con l'indirizzo PCI effettivo del vostro dispositivo. Utilizzare `lspci` o `dpdk-devbind.py --status` per identificare le vostre NIC.

**Alternativa**: Invece di utilizzare `dpdk-devbind.py` manualmente, potete configurare i binding dei dispositivi nel vostro file `sdplane.conf` utilizzando i comandi `set device XX:XX.X driver ...` all'inizio del file di configurazione. Questo permette a sdplane di gestire automaticamente il binding dei dispositivi all'avvio.