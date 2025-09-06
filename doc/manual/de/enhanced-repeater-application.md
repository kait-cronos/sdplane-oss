# Enhanced-Repeater-Anwendung

**Sprache / Language:** [English](../enhanced-repeater-application.md) | [日本語](../ja/enhanced-repeater-application.md) | [Français](../fr/enhanced-repeater-application.md) | [中文](../zh/enhanced-repeater-application.md) | **Deutsch**

Die Enhanced-Repeater-Anwendung bietet erweiterte VLAN-bewusste Layer-2-Switching mit TAP-Interface-Integration für L3-Routing und Paket-Capture-Funktionalität.

## Übersicht

Der Enhanced Repeater ist eine ausgeklügelte Layer-2-Weiterleitungsanwendung, die grundlegende L2-Weiterleitung um folgende Funktionen erweitert:
- **VLAN-bewusstes Switching**: Vollständige IEEE 802.1Q VLAN-Unterstützung (Tagging/Untagging)
- **Virtual Switch-Abstraktion**: Mehrere virtuelle Switches mit unabhängigen Weiterleitungstabellen
- **TAP-Interface-Integration**: Router-Interfaces und Capture-Interfaces für Kernel-Integration
- **Erweiterte VLAN-Verarbeitung**: VLAN-Translation, -Insertion und -Removal-Funktionen

## Architektur

### Virtual Switch Framework
Der Enhanced Repeater implementiert eine Virtual Switch-Architektur:
- **Mehrere VLANs**: Unterstützung mehrerer VLAN-Domänen (1-4094)
- **Port-Aggregation**: Mehrere physische Ports pro Virtual Switch
- **Isolierte Weiterleitung**: Unabhängige Weiterleitungsdomänen pro VLAN
- **Flexibles Tagging**: Native, Tagged und Translation-Modi pro Port

### TAP-Interface-Integration
- **Router-Interfaces**: Kernel-Netzwerkstack-Integration für L3-Verarbeitung
- **Capture-Interfaces**: Paketüberwachungs- und Analysefunktionen
- **Ring-Buffer**: Effiziente Paketübertragung zwischen Datenebene und Kernel
- **Bidirektional**: Paketverarbeitung in beide Richtungen

## Hauptfunktionen

### VLAN-Verarbeitung
- **VLAN-Tagging**: Hinzufügung von 802.1Q-Headern zu untagged Frames
- **VLAN-Untagging**: Entfernung von 802.1Q-Headern aus tagged Frames
- **VLAN-Translation**: Änderung von VLAN-IDs zwischen Ein- und Ausgang
- **Native VLAN**: Behandlung von untagged Traffic auf Trunk-Ports

### Virtual Switching
- **Lernen**: Automatisches MAC-Adress-Lernen pro VLAN
- **Flooding**: Angemessene Behandlung von unbekanntem Unicast und Broadcast
- **Split-Horizon**: Schleifenvermeidung innerhalb von Virtual Switches
- **Multi-Domain**: Unabhängige Weiterleitungstabellen pro VLAN

### Paketverarbeitung
- **Zero-Copy**: Effiziente DPDK-Paketverarbeitung mit minimalem Overhead
- **Burst-Verarbeitung**: Optimiert für hohe Paketraten
- **Header-Manipulation**: Effiziente VLAN-Header-Insertion/-Removal
- **Copy-Optimierung**: Selektive Paketkopierung für TAP-Interfaces

## Konfiguration

### Virtual Switch-Einrichtung
Virtual Switches für verschiedene VLAN-Domänen erstellen:

```bash
# Virtual Switches mit VLAN-IDs erstellen
set vswitch 2031
set vswitch 2032
```

### Port-zu-Virtual-Switch-Links
Physische DPDK-Ports mit Virtual Switches verknüpfen:

```bash
# Port 0 mit Virtual Switch 0 mit VLAN-Tag 2031 verknüpfen
set vswitch-link vswitch 0 port 0 tag 2031

# Port 0 mit Virtual Switch 1 als native/untagged verknüpfen
set vswitch-link vswitch 1 port 0 tag 0

# VLAN-Translation-Beispiel
set vswitch-link vswitch 0 port 1 tag 2040
```

### Router-Interfaces
Router-Interfaces für L3-Konnektivität erstellen:

```bash
# Router-Interfaces für L3-Verarbeitung erstellen
set vswitch 2031 router-if rif2031
set vswitch 2032 router-if rif2032
```

### Capture-Interfaces
Capture-Interfaces für Überwachung einrichten:

```bash
# Capture-Interfaces für Paketüberwachung erstellen
set vswitch 2031 capture-if cif2031
set vswitch 2032 capture-if cif2032
```

### Worker-Konfiguration
Enhanced Repeater einem lcore zuweisen:

```bash
# Enhanced Repeater-Worker konfigurieren
set worker lcore 1 enhanced-repeater

# Queue-Zuweisungen konfigurieren
set thread 1 port 0 queue 0
```

## Konfigurationsbeispiele

### Vollständige Einrichtung
Siehe [`example-config/sdplane_enhanced_repeater.conf`](../../example-config/sdplane_enhanced_repeater.conf):

```bash
# Device-Einrichtung
set device 03:00.0 driver vfio-pci bind

# DPDK-Initialisierung
set rte_eal argv -c 0x7
rte_eal_init
set mempool

# Port-Konfiguration
stop port all
set port all dev-configure 1 4
set port all nrxdesc 1024
set port all ntxdesc 1024

# Worker-Einrichtung
set worker lthread stat-collector
set worker lthread rib-manager
set worker lcore 1 enhanced-repeater
set worker lcore 2 l3-tap-handler

# Promiscuous-Modus aktivieren
set port all promiscuous enable
start port all

# Queue-Konfiguration
set thread 1 port 0 queue 0

# Virtual Switch-Einrichtung
set vswitch 2031
set vswitch 2032

# Link-Konfiguration
set vswitch-link vswitch 0 port 0 tag 2031
set vswitch-link vswitch 1 port 0 tag 2032

# Interface-Erstellung
set vswitch 2031 router-if rif2031
set vswitch 2032 router-if rif2032
set vswitch 2031 capture-if cif2031
set vswitch 2032 capture-if cif2032

# Worker starten
start worker lcore all
```

## VLAN-Verarbeitung im Detail

### Tagging-Modi

#### Tagged-Modus
- **Eingang**: Akzeptiert Pakete mit spezifischem VLAN-Tag
- **Ausgang**: Behält oder übersetzt VLAN-Tags bei
- **Verwendung**: Trunk-Ports, VLAN-Translation

#### Native-Modus (tag 0)
- **Eingang**: Akzeptiert untagged Pakete
- **Ausgang**: Entfernt VLAN-Header
- **Verwendung**: Access-Ports, VLAN-unaware Geräte

#### Translation-Modus
- **Funktion**: Ändert VLAN-IDs zwischen Ports
- **Konfiguration**: Verschiedene Tags auf Ein- und Ausgangs-Links
- **Verwendung**: VLAN-Mapping, Service-Provider-Netzwerke

### Paketfluss

1. **Eingangsverarbeitung**:
   - Paketempfang am DPDK-Port
   - VLAN-Bestimmung basierend auf Tag- oder Native-Konfiguration
   - Ziel-Virtual-Switch-Suche

2. **Virtual Switch-Suche**:
   - MAC-Adress-Lernen und -Suche
   - Ausgangsport-Bestimmung innerhalb der VLAN-Domäne
   - Behandlung von unbekanntem Unicast/Broadcast-Flooding

3. **Ausgangsverarbeitung**:
   - VLAN-Header-Manipulation pro Port-Konfiguration
   - Paketkopierung für mehrere Ziele
   - TAP-Interface-Integration

4. **TAP-Interface-Verarbeitung**:
   - Router-Interfaces: Kernel-L3-Verarbeitung
   - Capture-Interfaces: Überwachung und Analyse

## Überwachung und Verwaltung

### Status-Befehle
```bash
# Virtual Switch-Konfiguration anzeigen
show vswitch_rib

# Virtual Switch-Links anzeigen
show vswitch-link

# Router-Interfaces anzeigen
show rib vswitch router-if

# Capture-Interfaces überprüfen
show rib vswitch capture-if
```

### Statistiken und Leistung
```bash
# Port-Level-Statistiken
show port statistics all

# Worker-Leistung
show worker statistics

# Thread-Informationen
show thread information
```

### Debug-Befehle
```bash
# Enhanced Repeater-Debug
debug sdplane enhanced-repeater

# VLAN-Switch-Debug (alternativer Worker-Typ)
debug sdplane vlan-switch

# RIB- und Weiterleitungs-Debug
debug sdplane rib
debug sdplane fdb-change
```

## Anwendungsfälle

### VLAN-Aggregation
- Konsolidierung mehrerer VLANs über eine einzige physische Verbindung
- Provider-Edge-Funktionalität
- Service-Multiplexing

### L3-Integration
- Router-on-a-Stick-Konfiguration
- Inter-VLAN-Routing über TAP-Interfaces
- Hybrid-L2/L3-Weiterleitung

### Netzwerküberwachung
- VLAN-spezifische Paket-Captures
- Traffic-Analyse und Debugging
- Service-Überwachung

### Service-Provider-Netzwerke
- VLAN-Translation für Kundenisolierung
- Multi-Tenant-Netzwerke
- Traffic-Engineering

## Leistungsüberlegungen

### Skalierung
- **Worker-Zuweisungen**: Dedizierte lcores für optimale Leistung
- **Queue-Konfiguration**: Ausgewogene Queue-Zuweisung über Cores
- **Speicherpools**: Angemessene Dimensionierung für Paketrate und Pufferbedarf

### VLAN-Verarbeitungs-Overhead
- **Header-Manipulation**: Minimaler Overhead für VLAN-Operationen
- **Paket-Kopierung**: Selektive Kopierung nur wenn nötig
- **Burst-Optimierung**: Paketverarbeitung in Bursts für Effizienz

### TAP-Interface-Leistung
- **Ring-Buffer-Dimensionierung**: Angemessene Ring-Größen konfigurieren
- **Selektive Weiterleitung**: Nur relevante Pakete an TAP-Interfaces senden
- **Kernel-Integration**: Kernel-Verarbeitungs-Overhead berücksichtigen

## Fehlerbehebung

### Häufige Probleme
- **VLAN-Abweichungen**: Sicherstellen, dass Tag-Konfigurationen mit Netzwerk-Setup übereinstimmen
- **TAP-Interface-Erstellung**: Angemessene Berechtigungen und Kernel-Unterstützung überprüfen
- **Leistungsprobleme**: Queue-Zuweisungen und Worker-Verteilung überprüfen

### Debug-Strategien
- **Debug-Logging aktivieren**: Debug-Befehle für detaillierten Paketfluss verwenden
- **Statistiken überwachen**: Port- und Worker-Statistiken überwachen
- **Paket-Capture**: Capture-Interfaces für Traffic-Analyse verwenden

## Verwandte Dokumentation

- [Enhanced Repeater-Befehle](enhanced-repeater.md) - Vollständige Befehlsreferenz
- [Worker-Management](worker-management.md) - Details zur Worker-Konfiguration
- [Port-Management](port-management.md) - DPDK-Port-Konfiguration
- [TAP-Interface-Management](tap-interface.md) - TAP-Interface-Details