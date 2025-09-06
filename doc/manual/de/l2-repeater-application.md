# L2-Repeater-Anwendung

**Sprache / Language:** [English](../l2-repeater-application.md) | [日本語](../ja/l2-repeater-application.md) | [Français](../fr/l2-repeater-application.md) | [中文](../zh/l2-repeater-application.md) | **Deutsch**

Die L2-Repeater-Anwendung bietet einfache Layer-2-Paketweiterleitung zwischen DPDK-Ports mit grundlegender Port-zu-Port-Repeater-Funktionalität.

## Übersicht

Der L2-Repeater ist eine intuitive Layer-2-Weiterleitungsanwendung mit folgenden Funktionen:
- Paketweiterleitung zwischen gekoppelten DPDK-Ports (einfaches Port-zu-Port-Repeating)
- Grundlegende Paket-Repeater-Funktion ohne MAC-Adresslernfunktion
- Optionale MAC-Adress-Update-Funktion (Änderung der Quell-MAC-Adresse)
- Hochleistungs-Betrieb durch DPDK Zero-Copy-Paketverarbeitung

## Hauptfunktionen

### Layer-2-Weiterleitung
- **Port-zu-Port-Repeating**: Einfache Paketweiterleitung zwischen vorkonfigurierten Port-Paaren
- **Kein MAC-Lernen**: Direktes Paket-Repeating ohne Aufbau einer Weiterleitungstabelle
- **Transparente Weiterleitung**: Weiterleitung aller Pakete unabhängig von der Ziel-MAC
- **Port-Paarung**: Feste Port-zu-Port-Weiterleitungskonfiguration

### Leistungsmerkmale
- **Zero-Copy-Verarbeitung**: Nutzung der effizienten DPDK-Paketverarbeitung
- **Burst-Verarbeitung**: Burst-Verarbeitung von Paketen für optimalen Durchsatz
- **Niedrige Latenz**: Minimaler Verarbeitungsoverhead für schnelle Weiterleitung
- **Multi-Core-Unterstützung**: Ausführung auf dedizierten lcores für Skalierung

## Konfiguration

### Grundlegende Einrichtung
Der L2-Repeater wird über das Haupt-sdplane-Konfigurationssystem konfiguriert:

```bash
# Worker-Typ auf L2-Repeater setzen
set worker lcore 1 l2-repeater

# Port- und Queue-Konfiguration
set thread 1 port 0 queue 0  
set thread 1 port 1 queue 0

# Promiscuous-Modus für das Lernen aktivieren
set port all promiscuous enable
```

### Beispiel-Konfigurationsdatei
Für eine vollständige Konfigurationsbeispiel siehe [`example-config/sdplane_l2_repeater.conf`](../../example-config/sdplane_l2_repeater.conf):

```bash
# Device-Bindung
set device 02:00.0 driver vfio-pci bind
set device 03:00.0 driver vfio-pci bind

# DPDK-Initialisierung
set rte_eal argv -c 0x7
rte_eal_init

# Hintergrund-Worker (müssen vor Queue-Konfiguration gestartet werden)
set worker lthread stat-collector
set worker lthread rib-manager
set worker lthread netlink-thread

set mempool

# Queue-Konfiguration (muss nach rib-manager-Start ausgeführt werden)
set thread 1 port 0 queue 0
set thread 1 port 1 queue 0

# Port-Konfiguration
stop port all
set port all dev-configure 1 4
set port all nrxdesc 1024
set port all ntxdesc 1024

# Worker-Einrichtung
set worker lcore 1 l2-repeater
set worker lcore 2 tap-handler
set port all promiscuous enable
start port all

# Auf Port-Start warten
sleep 5

# Worker starten
start worker lcore all
```

## Betrieb

### Broadcast-Weiterleitung
Der L2-Repeater leitet Pakete an alle anderen aktiven Ports weiter:
- **Broadcast-Verhalten**: Weiterleitung aller auf einem Port empfangenen Pakete an alle anderen aktiven Ports
- **Split-Horizon**: Keine Rücksendung von Paketen an den empfangenden Port
- **Keine Filterung**: Weiterleitung aller Pakettypen (Unicast, Broadcast, Multicast)

### Weiterleitungsverhalten
- **Alle-Verkehr-Weiterleitung**: Repeating aller Pakete unabhängig von der Ziel-MAC
- **Alle-Ports-Broadcast**: Paketweiterleitung an alle aktiven Ports außer dem Eingangsport
- **Transparent**: Keine Änderung des Paketinhalts (außer MAC-Update ist aktiviert)
- **Multi-Port-Replikation**: Erstellung von Paketkopien für jeden Zielport

### MAC-Adress-Update
Wenn aktiviert, kann der L2-Repeater MAC-Adressen in Paketen ändern:
- **Quell-MAC-Update**: Änderung der Quell-MAC zur MAC des Ausgabeports
- **Transparentes Bridging**: Beibehaltung der ursprünglichen MAC-Adressen (Standard)

## Leistungsoptimierung

### Puffer-Konfiguration
```bash
# Deskriptor-Anzahl für Workload optimieren
set port all nrxdesc 2048  # Erhöhung für hohe Paketrate
set port all ntxdesc 2048  # Erhöhung für Pufferung
```

### Worker-Zuordnung
```bash
# Dedizierte lcore für L2-Weiterleitung
set worker lcore 1 l2-repeater  # Zuordnung zu dediziertem Core
set worker lcore 2 tap-handler  # TAP-Verarbeitung trennen
```

### Speicherpool-Dimensionierung
Der Speicherpool sollte angemessen für erwarteten Traffic dimensioniert werden:
- Berücksichtigung von Paketrate und Pufferanforderungen
- Berücksichtigung von Burst-Größe und temporärer Paketspeicherung

## Überwachung und Debugging

### Port-Statistiken
```bash
# Weiterleitungsstatistiken anzeigen
show port statistics all

# Spezifische Port-Überwachung
show port statistics 0
show port statistics 1
```

### Debug-Befehle
```bash
# L2-Repeater-Debug aktivieren
debug sdplane l2-repeater

# VLAN-Switch-Debug (alternativer Worker-Typ)
debug sdplane vlan-switch

# Allgemeines sdplane-Debug
debug sdplane fdb-change
debug sdplane rib
```

## Anwendungsfälle

### Hub-artiger Repeater
- Replikation von Traffic zu allen angeschlossenen Ports
- Transparente Layer-2-Wiederholung
- Grundlegende Hub-Funktionalität ohne Lernfähigkeit

### Port-Mirroring/Repeating
- Traffic-Mirroring zwischen Ports
- Netzwerküberwachung und -analyse
- Einfache Paketreplikation

### Leistungstests
- Messung der Weiterleitungsleistung
- Baseline für L2-Weiterleitungs-Benchmarks
- Validierung der DPDK-Port-Konfiguration

## Einschränkungen

- **Keine VLAN-Verarbeitung**: Einfaches L2-Repeating ohne VLAN-Erkennung
- **Kein MAC-Lernen**: Alle-Ports-Broadcast-Weiterleitung ohne Adresslernen
- **Keine STP-Unterstützung**: Keine Implementierung des Spanning Tree Protocol
- **Keine Filterung**: Weiterleitung aller Pakete unabhängig vom Ziel

## Verwandte Anwendungen

- **Enhanced Repeater**: Erweiterte Version mit VLAN-Unterstützung und TAP-Interfaces
- **L3-Weiterleitung**: Layer-3-Routing-Funktionalität
- **VLAN-Switch**: VLAN-bewusste Switching-Funktionalität

Für erweiterte Layer-2-Funktionen einschließlich VLAN-Unterstützung siehe die Dokumentation des [Enhanced Repeater](enhanced-repeater.md).