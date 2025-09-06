<div align="center">
<img src="../sdplane-logo.png" alt="sdplane-oss Logo" width="160">
</div>

# sdplane-oss (Software-Datenebene)

Eine „DPDK-dock-Entwicklungsumgebung", bestehend aus einer interaktiven Shell, die DPDK-Thread-Operationen steuern kann, und einer DPDK-Thread-Ausführungsumgebung (sd-plane)

**Language:** [English](../README.md) | [日本語](README.ja.md) | [Français](README.fr.md) | [中文](README.zh.md) | **Deutsch** | [Italiano](README.it.md) | [한국어](README.ko.md) | [ไทย](README.th.md) | [Español](README.es.md)

## Funktionen

- **Hochleistungs-Paketverarbeitung**:
  Zero-Copy-Paketverarbeitung im Benutzerbereich mit DPDK
- **Layer 2/3-Weiterleitung**:
  Integrierte L2- und L3-Weiterleitung mit ACL-, LPM- und FIB-Unterstützung
- **Paketgenerierung**:
  Integrierter Paketgenerator für Tests und Benchmarking
- **Netzwerkvirtualisierung**:
  TAP-Interface-Unterstützung und VLAN-Switching-Funktionen
- **CLI-Verwaltung**:
  Interaktive Befehlszeilenschnittstelle für Konfiguration und Überwachung
- **Multi-Threading**:
  Kooperatives Threading-Modell mit Workern pro Kern

### Architektur
- **Hauptanwendung**: Zentrale Router-Logik und Initialisierung
- **DPDK-Module**: L2/L3-Weiterleitung und Paketgenerierung
- **CLI-System**: Befehlszeilenschnittstelle mit Vervollständigung und Hilfe
- **Threading**: lthread-basiertes kooperatives Multitasking
- **Virtualisierung**: TAP-Interfaces und virtuelles Switching

## Unterstützte Systeme

### Software-Anforderungen
- **OS**:
  Ubuntu 24.04 LTS (derzeit unterstützt)
- **NICs**:
  [Treiber](https://doc.dpdk.org/guides/nics/) | [Unterstützte NICs](https://core.dpdk.org/supported/)
- **Speicher**:
  Hugepage-Unterstützung erforderlich
- **CPU**:
  Multi-Core-Prozessor empfohlen

### Ziel-Hardware-Plattformen

Das Projekt wurde getestet auf:
- **Topton (N305/N100)**: Mini-PC mit 10G NICs
- **Partaker (J3160)**: Mini-PC mit 1G NICs
- **Intel Generic PC**: Mit Intel x520 / Mellanox ConnectX5
- **Andere CPUs**: Sollte mit AMD-, ARM-Prozessoren usw. funktionieren

## 1. Installation der Abhängigkeiten

### Abhängigkeiten

sdplane-oss benötigt die folgenden Komponenten:
- **lthread** (yasuhironet/lthread): Leichtgewichtiges kooperatives Threading
- **liburcu-qsbr**: Userspace RCU library  
- **libpcap**: Packet capture library
- **DPDK 23.11.1**: Data Plane Development Kit

### Installation der sdplane-Abhängigkeiten Debian-Pakete

```bash
sudo apt update && sudo apt install liburcu-dev libpcap-dev
```

### Installation der Build-Tools und DPDK-Voraussetzungen

```bash
sudo apt install build-essential cmake autotools-dev autoconf automake \
                 libtool pkg-config python3 python3-pip meson ninja-build \
                 python3-pyelftools libnuma-dev pkgconf
```

### Installation von lthread

```bash
git clone https://github.com/yasuhironet/lthread
cd lthread
cmake .
make
sudo make install
cd ..
```

### Installation von DPDK 23.11.1

```bash
# DPDK 23.11.1 herunterladen
wget https://fast.dpdk.org/rel/dpdk-23.11.1.tar.xz
tar xf dpdk-23.11.1.tar.xz
cd dpdk-23.11.1

# DPDK kompilieren und installieren
meson setup -Dprefix=/usr/local build
cd build
ninja install
cd ../..

# Installation überprüfen
pkg-config --modversion libdpdk
# Sollte ausgeben: 23.11.1
```

## 2. Schnellstart mit Debian-Paket für Intel Core i3-n305/Celeron j3160

Für Intel Core i3-n305/Celeron j3160 ist eine schnelle Installation mit Debian-Paketen möglich.

Das vorkompilierte Debian-Paket herunterladen und installieren:

```bash
# Das neueste Paket für n305 herunterladen
wget https://www.yasuhironet.net/download/n305/sdplane_0.1.4-36_amd64.deb
wget https://www.yasuhironet.net/download/n305/sdplane-dbgsym_0.1.4-36_amd64.ddeb

# oder für j3160
wget https://www.yasuhironet.net/download/j3160/sdplane_0.1.4-35_amd64.deb
wget https://www.yasuhironet.net/download/j3160/sdplane-dbgsym_0.1.4-35_amd64.ddeb

# Paket installieren
sudo apt install ./sdplane_0.1.4-*_amd64.deb
sudo apt install ./sdplane-dbgsym_0.1.4-*_amd64.ddeb
```

**Hinweis**: Überprüfen Sie [yasuhironet.net Downloads](https://www.yasuhironet.net/download/) für die neueste Paketversion.

Springen Sie zu 5. Systemkonfiguration.

## 3. Kompilierung aus dem Quellcode

**Bitte folgen Sie im Allgemeinen diesem Verfahren.**

### Installation der erforderlichen Ubuntu-Pakete

#### Für Kompilierung aus dem Quellcode
```bash
# Wichtige Build-Tools
sudo apt install build-essential cmake autotools-dev autoconf automake libtool pkg-config

# DPDK-Voraussetzungen
sudo apt install python3 python3-pip meson ninja-build python3-pyelftools libnuma-dev pkgconf
```

#### Optionale Pakete
```bash
sudo apt install etckeeper tig bridge-utils \
                 iptables-persistent fail2ban dmidecode screen ripgrep
```

### Kompilierung von sdplane-oss aus dem Quellcode

```bash
# Repository klonen
git clone https://github.com/kait-cronos/sdplane-oss
cd sdplane-oss

# Build-Dateien generieren
sh autogen.sh

# Konfigurieren und kompilieren
mkdir build
cd build
CFLAGS="-g -O0" sh ../configure
make
```

## 4. Erstellung und Installation des sdplane-oss Debian-Pakets

### Installation der erforderlichen Pakete
```bash
sudo apt install build-essential cmake devscripts debhelper
```

### Kompilierung des sdplane-oss Debian-Pakets
```bash
# Zunächst sicherstellen, dass wir in einem sauberen Arbeitsbereich beginnen
(cd build && make distclean)
make distclean

# Debian-Paket aus dem Quellcode kompilieren
bash build-debian.sh

# Das generierte Paket installieren (wird im übergeordneten Verzeichnis erstellt)
sudo apt install ../sdplane_*.deb
```

## 5. Systemkonfiguration

- **Hugepages**: System-Hugepages für DPDK konfigurieren
- **Netzwerk**: netplan für Netzwerk-Interface-Konfiguration verwenden
- **Firewall**: telnet 9882/tcp Port ist für CLI erforderlich

**⚠️ Das CLI hat keine Authentifizierung. Es wird empfohlen, Verbindungen nur von localhost zuzulassen ⚠️**

### Hugepages konfigurieren
```bash
# GRUB-Konfiguration bearbeiten
sudo vi /etc/default/grub

# Hugepages zum GRUB_CMDLINE_LINUX-Parameter hinzufügen
# Beispiel für das Hinzufügen von hugepages=1024:
GRUB_CMDLINE_LINUX="hugepages=1024"

# GRUB aktualisieren
sudo update-grub

# System neu starten
sudo reboot

# Hugepages nach dem Neustart überprüfen
cat /proc/meminfo | grep -E "^HugePages|^Hugepagesize"
```

### Optionale Installation des DPDK IGB Kernel-Moduls

Falls Ihre NIC nicht mit vfio-pci funktioniert, installieren Sie igb_uio.

```bash
git clone http://dpdk.org/git/dpdk-kmods
cd dpdk-kmods/linux/igb_uio
make
sudo make install
cd ../../..

# Modul wird in /lib/modules/$(uname -r)/extra/igb_uio.ko installiert
```

## 6. sdplane-Konfiguration

### Konfigurationsdateien

sdplane verwendet Konfigurationsdateien, um das Startverhalten und die Laufzeitumgebung zu definieren.

#### OS-Konfigurationsbeispiele (`etc/`)
- `systemd/sdplane.service`: systemd Service-Datei
- `netplan/01-netcfg.yaml`: Netzwerkkonfiguration mit netplan

#### Anwendungs-Konfigurationsbeispiele (`example-config/`)
- Beispiel-Konfigurationsdateien für verschiedene Anwendungen
- Start-Skripte und Konfigurationsprofile

## 7. Anwendungen mit sdplane-oss ausführen

```bash
# Grundlegende Ausführung
sudo ./sdplane/sdplane

# Mit Konfigurationsdatei ausführen
sudo ./sdplane/sdplane -f /path/to/config-file

# Verbindung zum CLI (von einem anderen Terminal)
telnet localhost 9882

# Die interaktive Shell ist jetzt verfügbar
sdplane> help
sdplane> show version
```

## Tipps

### IOMMU ist erforderlich bei Verwendung von vfio-pci als NIC-Treiber

Virtualisierungsfähigkeiten müssen aktiviert werden:
- Intel: Intel VT-d
- AMD: AMD IOMMU / AMD-V

Diese Optionen müssen in den BIOS-Einstellungen aktiviert werden.
Die GRUB-Konfiguration muss möglicherweise auch geändert werden:

```conf
# /etc/default/grub
GRUB_CMDLINE_LINUX="iommu=pt intel_iommu=on"
```

Änderungen anwenden:
```bash
sudo update-grub
sudo reboot
```

### Konfiguration zum dauerhaften Laden des Linux-Kernel-Moduls vfio-pci

```bash
# Konfigurationsdatei für automatisches Laden erstellen
sudo tee /etc/modules-load.d/vfio-pci.conf > /dev/null <<EOF
vfio-pci
EOF
```

### Für Mellanox ConnectX-Serie

Die Treiber-Installation ist über den folgenden Link erforderlich:

https://network.nvidia.com/products/ethernet-drivers/linux/mlnx_en/

Führen Sie während der Installation `./install --dpdk` aus.
**Die Option `--dpdk` ist obligatorisch.**

Kommentieren Sie die folgenden Einstellungen in sdplane.conf aus, da sie nicht benötigt werden:

```conf
#set device {pcie-id} driver unbind
#set device {pcie-id} driver {driver-name} driver_override  
#set device {pcie-id} driver {driver-name} bind
```

Für Mellanox-NICs müssen Sie den Befehl `update port status` ausführen, um die Port-Informationen zu aktualisieren.

### Wie man PCIe-Busnummern überprüft

Sie können den Befehl dpdk-devbind.py in DPDK verwenden, um die PCIe-Busnummern der NICs zu überprüfen:

```bash
# Netzwerkgeräte-Status anzeigen  
dpdk-devbind.py -s

# Beispiel-Ausgabe:
Network devices using kernel driver
===================================
0000:04:00.0 'NetXtreme BCM5720 Gigabit Ethernet PCIe 165f' numa_node=0 if=eno8303 drv=tg3 unused= *Active*
0000:b1:00.0 'MT27800 Family [ConnectX-5] 1017' numa_node=1 if=enp177s0np0 drv=mlx5_core unused= *Active*
```

### Reihenfolge der Worker-Threads in der Konfigurationsdatei

Wenn Sie `rib-manager`, `neigh-manager` und `netlink-thread` Worker konfigurieren, müssen sie in dieser Reihenfolge konfiguriert werden, falls sie verwendet werden.

### DPDK-Initialisierung

Nur ein Befehl, der `rte_eal_init()` aufruft, sollte aus der Konfigurationsdatei aufgerufen werden. Die Funktion `rte_eal_init()` wird von Befehlen wie `rte_eal_init`, `pktgen init`, `l2fwd init` und `l3fwd init` aufgerufen.

## Benutzerhandbuch (Manual)

Umfassende Benutzerhandbücher und Befehlsreferenzen sind verfügbar:

- [Benutzerhandbuch](manual/de/README.md) - Vollständige Übersicht und Befehlsklassifizierung

**Anwendungshandbücher:**
- [L2 Repeater-Anwendung](manual/de/l2-repeater-application.md) - Einfache Layer 2-Paketweiterleitung mit MAC-Lernen
- [Enhanced Repeater-Anwendung](manual/de/enhanced-repeater-application.md) - VLAN-bewusstes Switching mit TAP-Interfaces  
- [Paketgenerator-Anwendung](manual/de/packet-generator-application.md) - Hochleistungs-Traffic-Generierung und Tests

**Konfigurationshandbücher:**
- [RIB & Routing](manual/de/routing.md) - RIB- und Routing-Funktionen
- [Enhanced Repeater](manual/de/enhanced-repeater.md) - Enhanced Repeater-Konfiguration
- [Port-Verwaltung & Statistiken](manual/de/port-management.md) - DPDK-Port-Verwaltung und Statistiken
- [Worker & lcore Management & Thread-Informationen](manual/de/worker-lcore-thread-management.md) - Worker-Threads, lcore und Thread-Informationen Verwaltung
- [System-Information & Überwachung](manual/de/system-monitoring.md) - Systeminformationen und Überwachung
- [Geräteverwaltung](manual/de/device-management.md) - Geräte- und Treiberverwaltung

**Entwicklerhandbücher:**
- [DPDK-Integrationshandbuch](manual/de/dpdk-integration-guide.md) - Wie man DPDK-Anwendungen integriert
- [Debug & Logging](manual/de/debug-logging.md) - Debug- und Logging-Funktionen
- [Warteschlangen-Konfiguration](manual/de/queue-configuration.md) - Warteschlangen-Konfiguration und -verwaltung
- [TAP-Interface](manual/de/tap-interface.md) - TAP-Interface-Verwaltung
- [VTY & Shell-Verwaltung](manual/de/vty-shell.md) - VTY- und Shell-Verwaltung
- [lthread-Verwaltung](manual/de/lthread-management.md) - Kooperative leichte Thread-Verwaltung
- [Paketgenerierung](manual/de/packet-generation.md) - Paketgenerierung mit PKTGEN

## Entwicklerhandbuch

### Integrationshandbuch
- [DPDK-Anwendungs-Integrationshandbuch](manual/de/dpdk-integration-guide.md) - Wie man DPDK-Anwendungen mit dem DPDK-dock-Ansatz in sdplane integriert

### Dokumentation
- Alle Entwicklerdokumentation ist in `doc/` enthalten
- Integrationshandbücher und Beispiele sind in `doc/manual/de/`

### Code-Stil

Dieses Projekt folgt den GNU Coding Standards. Um den Code zu überprüfen und zu formatieren:

```bash
# Stil überprüfen (benötigt clang-format 18.1.3+)
./style/check_gnu_style.sh check

# Code automatisch formatieren
./style/check_gnu_style.sh update  

# Unterschiede anzeigen
./style/check_gnu_style.sh diff
```

Erforderliche Tools installieren:
```bash
# Für Ubuntu 24.04
sudo apt install clang-format-18
```

## Lizenz

Dieses Projekt steht unter der Apache 2.0-Lizenz - siehe die Datei [LICENSE](LICENSE) für Details.

## Kontakt

- GitHub: https://github.com/kait-cronos/sdplane-oss  
- Issues: https://github.com/kait-cronos/sdplane-oss/issues

## Kauf von Evaluierungsgeräten

Evaluierungsgeräte können zusätzliche Funktionen und Software-Modifikationen enthalten.

Für Anfragen zu Evaluierungsgeräten kontaktieren Sie uns über GitHub Issues oder direkt per E-Mail.