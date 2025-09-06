# RIB und Routing

**Sprache / Language:** [English](../routing.md) | [日本語](../ja/routing.md) | [Français](../fr/routing.md) | [中文](../zh/routing.md) | **Deutsch**

Befehle zur Verwaltung von RIB (Routing Information Base) und Systemressourceninformationen.

## Befehlsübersicht

### show_rib - RIB-Informationen anzeigen
```
show rib
```

Zeigt RIB (Routing Information Base) Informationen an.

**Verwendungsbeispiel:**
```bash
show rib
```

Dieser Befehl zeigt folgende Informationen an:
- RIB-Version und Speicherzeiger
- Virtual Switch-Konfiguration und VLAN-Zuweisungen
- DPDK-Port-Status und Queue-Konfiguration
- lcore-zu-Port-Queue-Zuweisungen
- Neighbor-Tabellen für L2/L3-Forwarding

## RIB-Übersicht

### Was ist RIB?
RIB (Routing Information Base) ist eine zentrale Datenbank zur Speicherung von Systemressourcen und Netzwerkinformationen. In sdplane werden folgende Informationen verwaltet:

- **Virtual Switch-Konfiguration** - VLAN-Switching und Port-Zuweisungen
- **DPDK-Port-Informationen** - Link-Status, Queue-Konfiguration, Feature-Informationen
- **lcore-Queue-Konfiguration** - Paketverarbeitungszuweisungen pro CPU-Core
- **Neighbor-Tabellen** - L2/L3-Forwarding-Datenbankeinträge

### RIB-Struktur
RIB besteht aus zwei Hauptstrukturen:

```c
struct rib {
    struct rib_info *rib_info;  // Zeiger auf tatsächliche Daten
};

struct rib_info {
    uint32_t ver;                                    // Versionsnummer
    uint8_t vswitch_size;                           // Anzahl Virtual Switches
    uint8_t port_size;                              // Anzahl DPDK-Ports
    uint8_t lcore_size;                             // Anzahl lcores
    struct vswitch_conf vswitch[MAX_VSWITCH];       // Virtual Switch-Konfiguration
    struct vswitch_link vswitch_link[MAX_VSWITCH_LINK]; // VLAN-Port-Links
    struct port_conf port[MAX_ETH_PORTS];           // DPDK-Port-Konfiguration
    struct lcore_qconf lcore_qconf[RTE_MAX_LCORE];  // lcore-Queue-Zuweisungen
    struct neigh_table neigh_tables[NEIGH_NR_TABLES]; // Neighbor/Forwarding-Tabellen
};
```

## Interpretation der RIB-Informationen

### Grundlegende Anzeigeelemente
- **Destination** - Ziel-Netzwerkadresse
- **Netmask** - Netzmaske
- **Gateway** - Gateway (Next Hop)
- **Interface** - Ausgabeschnittstelle
- **Metric** - Route-Metrik
- **Status** - Route-Status

### Route-Status
- **Active** - Aktive Route
- **Inactive** - Inaktive Route
- **Pending** - Konfigurierte Route
- **Invalid** - Ungültige Route

## Verwendungsbeispiele

### Grundlegende Verwendung
```bash
# RIB-Informationen anzeigen
show rib
```

### Interpretation der Ausgabe
```
rib information version: 21 (0x55555dd42010)
vswitches: 
dpdk ports: 
  dpdk_port[0]: 
    link: speed=1000Mbps duplex=full autoneg=on status=up
    nb_rxd=1024 nb_txd=1024
    queues: nrxq=1 ntxq=4
  dpdk_port[1]: 
    link: speed=0Mbps duplex=half autoneg=on status=down
    nb_rxd=1024 nb_txd=1024
    queues: nrxq=1 ntxq=4
  dpdk_port[2]: 
    link: speed=0Mbps duplex=half autoneg=off status=down
    nb_rxd=1024 nb_txd=1024
    queues: nrxq=1 ntxq=4
lcores: 
  lcore[0]: nrxq=0
  lcore[1]: nrxq=1
    rxq[0]: dpdk_port[0], queue_id=0
  lcore[2]: nrxq=1
    rxq[0]: dpdk_port[1], queue_id=0
  lcore[3]: nrxq=1
    rxq[0]: dpdk_port[2], queue_id=0
  lcore[4]: nrxq=0
  lcore[5]: nrxq=0
  lcore[6]: nrxq=0
  lcore[7]: nrxq=0
```

Dieses Beispiel zeigt:
- RIB-Version 21 repräsentiert den aktuellen Systemstatus
- DPDK-Port 0 ist aktiv (up) mit 1Gbps Link-Geschwindigkeit
- DPDK-Ports 1, 2 sind inaktiv (down) ohne Link
- lcores 1, 2, 3 sind für die Paketverarbeitung von Port 0, 1, 2 zuständig
- Jeder Port verwendet 1 RX-Queue und 4 TX-Queues
- RX/TX-Descriptor-Rings sind mit 1024 Einträgen konfiguriert

## RIB-Verwaltung

### Automatische Updates
RIB wird automatisch zu folgenden Zeitpunkten aktualisiert:
- Interface-Statusänderungen
- Netzwerkkonfigurationsänderungen
- Informationsempfang von Routing-Protokollen

### Manuelle Updates
Um RIB-Informationen manuell zu prüfen:
```bash
show rib
```

## Fehlerbehebung

### Wenn Routing nicht korrekt funktioniert
1. RIB-Informationen prüfen
```bash
show rib
```

2. Interface-Status prüfen
```bash
show port
```

3. Worker-Status prüfen
```bash
show worker
```

### Wenn Routen nicht in RIB angezeigt werden
- Netzwerkkonfiguration prüfen
- Interface-Status prüfen
- RIB-Manager-Betrieb prüfen

## Erweiterte Funktionen

### RIB-Manager
Der RIB-Manager läuft als unabhängiger Thread und bietet folgende Funktionen:
- Automatische Aktualisierung von Routing-Informationen
- Route-Gültigkeitsprüfung
- Netzwerkstatusüberwachung

### Verwandte Worker
- **rib-manager** - Worker zur RIB-Verwaltung
- **l3fwd** - Verwendet RIB für Layer 3 Forwarding
- **l3fwd-lpm** - Kooperation zwischen LPM-Tabelle und RIB

## Definitionsort

Dieser Befehl ist in der folgenden Datei definiert:
- `sdplane/rib.c`

## Verwandte Themen

- [Worker- und lcore-Management](worker-management.md)
- [lthread-Management](lthread-management.md)
- [Systeminformationen und Überwachung](system-monitoring.md)