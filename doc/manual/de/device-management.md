# Geräteverwaltung

**Language:** [English](../en/device-management.md) | [日本語](../ja/device-management.md) | [Français](../fr/device-management.md) | [中文](../zh/device-management.md) | **Deutsch** | [Italiano](../it/device-management.md) | [한국어](../ko/device-management.md) | [ไทย](../th/device-management.md) | [Español](../es/device-management.md)

Befehle zur Verwaltung von DPDK-Geräten und -Treibern.

## Befehlsübersicht

### show_devices - Geräteinformationen anzeigen
```
show devices
```

Zeigt Informationen über im System verfügbare Geräte an.

**Verwendungsbeispiel:**
```bash
show devices
```

Dieser Befehl zeigt folgende Informationen an:
- Gerätename
- Gerätetyp
- Aktueller Treiber
- Gerätestatus
- PCI-Adresse

### set_device_driver - Gerätetreiber konfigurieren
```
set device <WORD> driver (ixgbe|igb|igc|uio_pci_generic|igb_uio|vfio-pci|unbound) (|bind|driver_override)
```

Konfiguriert einen Treiber für das angegebene Gerät.

**Parameter:**
- `<WORD>` - Gerätename oder PCI-Adresse
- Treibertypen:
  - `ixgbe` - Intel 10GbE ixgbe-Treiber
  - `igb` - Intel 1GbE igb-Treiber
  - `igc` - Intel 2.5GbE igc-Treiber
  - `uio_pci_generic` - Generischer UIO-Treiber
  - `igb_uio` - DPDK UIO-Treiber
  - `vfio-pci` - VFIO PCI-Treiber
  - `unbound` - Treiber entfernen
- Betriebsmodus:
  - `bind` - Treiber binden
  - `driver_override` - Treiber-Override

**Verwendungsbeispiel:**
```bash
# Gerät an vfio-pci-Treiber binden
set device 0000:01:00.0 driver vfio-pci bind

# Gerät für igb_uio-Treiber konfigurieren
set device eth0 driver igb_uio

# Treiber von Gerät entfernen
set device 0000:01:00.0 driver unbound
```

## Übersicht der Geräteverwaltung

### DPDK-Geräteverwaltung
DPDK verwendet spezielle Treiber zur effizienten Nutzung von Netzwerkgeräten.

### Treibertypen

#### Netzwerktreiber
- **ixgbe** - Für Intel 10GbE-Netzwerkkarten
- **igb** - Für Intel 1GbE-Netzwerkkarten
- **igc** - Für Intel 2.5GbE-Netzwerkkarten

#### UIO (Userspace I/O) Treiber
- **uio_pci_generic** - Generischer UIO-Treiber
- **igb_uio** - DPDK-spezifischer UIO-Treiber

#### VFIO (Virtual Function I/O) Treiber
- **vfio-pci** - Hochleistungs-I/O in virtualisierten Umgebungen

#### Spezielle Konfigurationen
- **unbound** - Treiber entfernen und Gerät deaktivieren

## Schritte zur Gerätekonfiguration

### Grundlegende Konfigurationsschritte
1. **Geräte überprüfen**
```bash
show devices
```

2. **Treiber konfigurieren**
```bash
set device <device> driver <driver> bind
```

3. **Konfiguration überprüfen**
```bash
show devices
```

4. **Port-Konfiguration**
```bash
show port
```

### Konfigurationsbeispiele

#### Intel 10GbE-Karten-Konfiguration
```bash
# Geräte überprüfen
show devices

# ixgbe-Treiber binden
set device 0000:01:00.0 driver ixgbe bind
set device 0000:01:00.1 driver ixgbe bind

# Konfiguration überprüfen
show devices
show port
```

#### DPDK UIO-Treiber-Konfiguration
```bash
# Geräte überprüfen
show devices

# igb_uio-Treiber binden
set device 0000:02:00.0 driver igb_uio bind
set device 0000:02:00.1 driver igb_uio bind

# Konfiguration überprüfen
show devices
show port
```

#### VFIO-Konfiguration (Virtualisierungsumgebung)
```bash
# Geräte überprüfen
show devices

# vfio-pci-Treiber binden
set device 0000:03:00.0 driver vfio-pci bind
set device 0000:03:00.1 driver vfio-pci bind

# Konfiguration überprüfen
show devices
show port
```

## Richtlinien für Treiberauswahl

### ixgbe (Intel 10GbE)
- **Verwendung**: Intel 10GbE-Netzwerkkarten
- **Vorteile**: Hohe Leistung, Stabilität
- **Voraussetzungen**: Intel 10GbE-Karten erforderlich

### igb (Intel 1GbE)
- **Verwendung**: Intel 1GbE-Netzwerkkarten
- **Vorteile**: Breite Kompatibilität, Stabilität
- **Voraussetzungen**: Intel 1GbE-Karten erforderlich

### igc (Intel 2.5GbE)
- **Verwendung**: Intel 2.5GbE-Netzwerkkarten
- **Vorteile**: Mittlere Leistung, neuer Standard
- **Voraussetzungen**: Intel 2.5GbE-Karten erforderlich

### uio_pci_generic
- **Verwendung**: Universelle Geräte
- **Vorteile**: Breite Kompatibilität
- **Nachteile**: Einige Funktionseinschränkungen

### igb_uio
- **Verwendung**: DPDK-spezifische Umgebung
- **Vorteile**: DPDK-Optimierung
- **Nachteile**: Separate Installation erforderlich

### vfio-pci
- **Verwendung**: Virtualisierungsumgebung, sicherheitsorientiert
- **Vorteile**: Sicherheit, Virtualisierungsunterstützung
- **Voraussetzungen**: IOMMU-Aktivierung erforderlich

## Fehlerbehebung

### Wenn Geräte nicht erkannt werden
1. Geräte überprüfen
```bash
show devices
```

2. Systemebenen-Überprüfung
```bash
lspci | grep Ethernet
```

3. Kernel-Module überprüfen
```bash
lsmod | grep uio
lsmod | grep vfio
```

### Wenn Treiberbindung fehlschlägt
1. Aktuellen Treiber überprüfen
```bash
show devices
```

2. Bestehenden Treiber entfernen
```bash
set device <device> driver unbound
```

3. Ziel-Treiber binden
```bash
set device <device> driver <target_driver> bind
```

### Wenn Ports nicht verfügbar sind
1. Gerätestatus überprüfen
```bash
show devices
show port
```

2. Treiber neu binden
```bash
set device <device> driver unbound
set device <device> driver <driver> bind
```

3. Port-Konfiguration überprüfen
```bash
show port
update port status
```

## Erweiterte Funktionen

### Treiber-Override
```bash
# Treiber-Override verwenden
set device <device> driver <driver> driver_override
```

### Batch-Konfiguration mehrerer Geräte
```bash
# Mehrere Geräte nacheinander konfigurieren
set device 0000:01:00.0 driver vfio-pci bind
set device 0000:01:00.1 driver vfio-pci bind
set device 0000:02:00.0 driver vfio-pci bind
set device 0000:02:00.1 driver vfio-pci bind
```

## Sicherheitsüberlegungen

### Hinweise bei VFIO-Verwendung
- IOMMU-Aktivierung erforderlich
- Sicherheitsgruppen-Konfiguration
- Angemessene Berechtigungseinstellungen

### Hinweise bei UIO-Verwendung
- Root-Berechtigung erforderlich
- Verständnis der Sicherheitsrisiken
- Angemessene Zugriffskontrolle

## Systemintegration

### Integration mit systemd-Services
```bash
# Automatische Konfiguration in systemd-Service
# Konfiguration in /etc/systemd/system/sdplane.service
```

### Automatische Konfiguration beim Start
```bash
# Konfiguration in Start-Script
# /etc/init.d/sdplane oder systemd unit file
```

## Definitionsort

Diese Befehle sind in den folgenden Dateien definiert:
- `sdplane/dpdk_devbind.c`

## Verwandte Themen

- [Port-Management und Statistiken](port-management.md)
- [Worker- und lcore-Management](worker-lcore-thread-management.md)
- [Systeminformationen und Überwachung](system-monitoring.md)