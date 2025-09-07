# Systemkonfiguration

**Language:** [English](../en/system-configuration.md) | [Japanese](../ja/system-configuration.md) | [Français](../fr/system-configuration.md) | [中文](../zh/system-configuration.md) | **Deutsch**

- **Hugepages**: System-Hugepages für DPDK konfigurieren
- **Netzwerk**: netplan für Netzwerk-Interface-Konfiguration verwenden
- **Firewall**: telnet 9882/tcp Port ist für CLI erforderlich

**⚠️ Das CLI hat keine Authentifizierung. Es wird empfohlen, Verbindungen nur von localhost zuzulassen ⚠️**

## Hugepages konfigurieren
```bash
# GRUB-Konfiguration bearbeiten
sudo vi /etc/default/grub

# Eine dieser Zeilen hinzufügen:
# Für 2MB Hugepages (1536 Seiten = ~3GB):
GRUB_CMDLINE_LINUX="hugepages=1536"

# Oder für 1GB Hugepages (8 Seiten = 8GB):
GRUB_CMDLINE_LINUX="default_hugepagesz=1G hugepagesz=1G hugepages=8"

# GRUB aktualisieren und neu starten
sudo update-grub
sudo reboot
```

## Installation des DPDK IGB Kernel-Moduls (Optional)

Für NICs, die nicht mit vfio-pci funktionieren, installieren Sie optional igb_uio:

```bash
git clone http://dpdk.org/git/dpdk-kmods
cd dpdk-kmods/linux/igb_uio
make
sudo mkdir -p /lib/modules/`uname -r`/extra/dpdk/
sudo cp igb_uio.ko /lib/modules/`uname -r`/extra/dpdk/
echo igb_uio | sudo tee /etc/modules-load.d/igb_uio.conf
```

## DPDK UIO-Treiber konfigurieren

DPDK benötigt User-I/O-(UIO)-Treiber, um vom Benutzerbereich aus auf Netzwerkschnittstellen zuzugreifen.

### Standard-NICs

Für die meisten Standard-Netzwerkkarten haben Sie drei UIO-Treiber-Optionen:

1. **vfio-pci** (Empfohlen)
   - Sicherste und modernste Option
   - Benötigt IOMMU-Unterstützung (Intel VT-d oder AMD-Vi)
   - Keine zusätzliche Kernel-Modul-Kompilierung erforderlich

2. **uio_pci_generic**
   - Generischer UIO-Treiber im Linux-Kernel enthalten
   - Keine zusätzliche Installation erforderlich
   - Eingeschränkte Funktionalität im Vergleich zu vfio-pci

3. **igb_uio**
   - DPDK-spezifischer UIO-Treiber
   - Erfordert manuelle Kompilierung und Installation (siehe oben)
   - Bietet zusätzliche Funktionen für ältere Hardware

### NICs mit gegabelten Treibern

Einige NICs, wie die **Mellanox ConnectX-Serie**, bieten gegabelte Treiber, die es ermöglichen, denselben physischen Port gleichzeitig vom Kernel-Netzwerk und von DPDK-Anwendungen zu verwenden. Für diese NICs:

- Keine UIO-Treiber-Konfiguration erforderlich
- Die NIC kann an ihren Kernel-Treiber gebunden bleiben
- DPDK-Anwendungen können direkt auf die Hardware zugreifen

### UIO-Treiber konfigurieren

Um UIO-Treiber zu konfigurieren, müssen Sie:

1. **Treibermodul laden**:
```bash
# Für vfio-pci (benötigt IOMMU aktiviert in BIOS/UEFI und Kernel)
sudo modprobe vfio-pci

# Für uio_pci_generic
sudo modprobe uio_pci_generic

# Für igb_uio (nach Installation)
sudo modprobe igb_uio
```

2. **Treiber beim Booten laden lassen**, indem Sie eine Konfigurationsdatei in `/etc/modules-load.d/` erstellen:
```bash
# Beispiel für vfio-pci
echo "vfio-pci" | sudo tee /etc/modules-load.d/vfio-pci.conf

# Beispiel für uio_pci_generic
echo "uio_pci_generic" | sudo tee /etc/modules-load.d/uio_pci_generic.conf

# Beispiel für igb_uio
echo "igb_uio" | sudo tee /etc/modules-load.d/igb_uio.conf
```

3. **NICs an den UIO-Treiber binden** mit dem DPDK-Skript `dpdk-devbind.py`:
```bash
# Vom Kernel-Treiber lösen und an UIO-Treiber binden
sudo dpdk-devbind.py --bind=vfio-pci 0000:01:00.0
sudo dpdk-devbind.py --bind=uio_pci_generic 0000:01:00.1
sudo dpdk-devbind.py --bind=igb_uio 0000:01:00.2
```

**Hinweis**: Ersetzen Sie `0000:01:00.0` durch Ihre tatsächliche PCI-Geräteadresse. Verwenden Sie `lspci` oder `dpdk-devbind.py --status`, um Ihre NICs zu identifizieren.

**Alternative**: Anstatt `dpdk-devbind.py` manuell zu verwenden, können Sie die Gerätebindungen in Ihrer `sdplane.conf`-Datei konfigurieren, indem Sie die Befehle `set device XX:XX.X driver ...` am Anfang der Konfigurationsdatei verwenden. Dies ermöglicht es sdplane, die Gerätebindung beim Start automatisch zu verwalten.