# Paket-Generator (PKTGEN) Anwendung

**Language:** [English](../en/packet-generator-application.md) | [日本語](../ja/packet-generator-application.md) | [Français](../fr/packet-generator-application.md) | [中文](../zh/packet-generator-application.md) | **Deutsch** | [Italiano](../it/packet-generator-application.md) | [한국어](../ko/packet-generator-application.md) | [ไทย](../th/packet-generator-application.md) | [Español](../es/packet-generator-application.md)

Die Paket-Generator (PKTGEN) Anwendung bietet hochleistungsfähige Paketgenerierung und Traffic-Test-Funktionen unter Verwendung des DPDK-optimierten Paketverarbeitungs-Frameworks.

## Übersicht

PKTGEN ist ein ausgeklügeltes Traffic-Generierungs-Tool, das in sdplane-oss integriert ist und folgende Funktionen bietet:
- **Hochgeschwindigkeits-Paketgenerierung**: Multi-Gigabit-Paketübertragungsraten
- **Flexible Traffic-Muster**: Anpassbare Paketgrößen, -raten und -muster
- **Multi-Port-Unterstützung**: Unabhängige Traffic-Generierung auf mehreren Ports
- **Erweiterte Funktionen**: Bereichstests, Ratenbegrenzung, Traffic-Shaping
- **Leistungstests**: Netzwerk-Durchsatz- und Latenzmessungen

## Architektur

### Kernkomponenten
- **TX-Engine**: Hochleistungs-Paketübertragung mit DPDK
- **RX-Engine**: Paketempfang und Statistiksammlung
- **L2P-Framework**: Lcore-zu-Port-Mapping für optimale Leistung
- **Konfigurationsmanagement**: Dynamische Traffic-Parameter-Konfiguration
- **Statistik-Engine**: Umfassende Leistungsmetriken und Berichterstattung

### Worker-Modell
PKTGEN arbeitet mit dedizierten Worker-Threads (lcore):
- **TX-Worker**: Dedizierte Cores für Paketübertragung
- **RX-Worker**: Dedizierte Cores für Paketempfang
- **Gemischte Worker**: Kombinierte TX/RX auf einem einzelnen Core
- **Kontroll-Threads**: Management und Statistiksammlung

## Hauptfunktionen

### Traffic-Generierung
- **Paketrate**: Leitungsraten-Traffic-Generierung bis zu Interface-Limits
- **Paketgröße**: Konfigurierbar von 64 Bytes bis Jumbo-Frames
- **Traffic-Muster**: Gleichmäßige, Burst- und benutzerdefinierte Muster
- **Multi-Streams**: Mehrere Traffic-Streams pro Port

### Erweiterte Funktionen
- **Ratenbegrenzung**: Präzise Traffic-Ratenkontrolle
- **Bereichstests**: Paketgröße-Sweeps und Raten-Sweeps
- **Lastmuster**: Konstante, Ramp-up, Burst-Traffic
- **Protokollunterstützung**: Ethernet, IPv4, IPv6, TCP, UDP

### Leistungsüberwachung
- **Echtzeit-Statistiken**: TX/RX-Raten, Paketzahlen, Fehlerzahlen
- **Latenzmessungen**: Ende-zu-Ende-Paket-Latenztests
- **Durchsatzanalyse**: Bandbreitennutzung und -effizienz
- **Fehlererkennung**: Paketverlust und Korruptionserkennung

## Konfiguration

### Grundlegende Einrichtung
PKTGEN erfordert spezifische Initialisierung und Worker-Konfiguration:

```bash
# Gerätebindung für PKTGEN
set device 01:00.0 driver vfio-pci bind
set device 01:00.1 driver vfio-pci bind

# PKTGEN-Initialisierung über argv-list
set argv-list 2 ./usr/local/bin/pktgen -l 0-7 -n 4 --proc-type auto --log-level 7 --file-prefix pg -- -v -T -P -l pktgen.log -m [4:5].0 -m [6:7].1 -f themes/black-yellow.theme

# PKTGEN-Initialisierung
pktgen init argv-list 2
```

### Worker-Zuweisungen
Dedizierte Core-Zuweisungen für optimale Leistung:

```bash
# PKTGEN-Worker spezifischen lcores zuweisen
set worker lcore 4 pktgen  # Port 0 TX/RX
set worker lcore 5 pktgen  # Port 0 TX/RX
set worker lcore 6 pktgen  # Port 1 TX/RX
set worker lcore 7 pktgen  # Port 1 TX/RX
```

### Port-Konfiguration
DPDK-Ports für PKTGEN-Verwendung konfigurieren:

```bash
# Port-Einrichtung
stop port all
set port all dev-configure 1 4
set port all nrxdesc 1024
set port all ntxdesc 1024
set port all promiscuous enable
start port all
```

## Konfigurationsbeispiele

### Vollständige PKTGEN-Einrichtung
Siehe [`example-config/sdplane-pktgen.conf`](../../example-config/sdplane-pktgen.conf):

```bash
# Log-Konfiguration
log file /var/log/sdplane.log
log stdout

# Gerätebindung
set device 01:00.0 driver vfio-pci bind
set device 01:00.1 driver vfio-pci bind
set device 03:00.0 driver uio_pci_generic bind

# PKTGEN-Initialisierung
set argv-list 2 ./usr/local/bin/pktgen -l 0-7 -n 4 --proc-type auto --log-level 7 --file-prefix pg -- -v -T -P -l pktgen.log -m [4:5].0 -m [6:7].1 -f themes/black-yellow.theme
pktgen init argv-list 2

# Speicherpool-Einrichtung
set mempool

# Hintergrund-Worker
set worker lthread stat-collector
set worker lthread rib-manager
set worker lthread netlink-thread

# Port-Konfiguration
stop port all
set port all dev-configure 1 4
set port all nrxdesc 1024
set port all ntxdesc 1024

# Worker-Zuweisungen
set worker lcore 2 vlan-switch
set worker lcore 3 tap-handler
set worker lcore 4 pktgen
set worker lcore 5 pktgen
set worker lcore 6 pktgen
set worker lcore 7 pktgen

# Promiscuous-Modus aktivieren und starten
set port all promiscuous enable
start port all

# Alle Worker starten
start worker lcore all
```

## PKTGEN-Befehlszeilenargumente

### Core-Parameter
- `-l 0-7`: CPU-Core-Liste (Cores 0-7)
- `-n 4`: Anzahl der Speicherkanäle
- `--proc-type auto`: Prozesstyp (primär/sekundär)
- `--log-level 7`: Debug-Log-Level
- `--file-prefix pg`: Shared-Memory-Präfix

### PKTGEN-spezifische Optionen
- `-v`: Verbose-Ausgabe
- `-T`: Farbige Terminal-Ausgabe aktivieren
- `-P`: Promiscuous-Modus aktivieren
- `-l pktgen.log`: Log-Datei-Standort
- `-m [4:5].0`: lcores 4,5 zu Port 0 mappen
- `-m [6:7].1`: lcores 6,7 zu Port 1 mappen
- `-f themes/black-yellow.theme`: Farbthema

## lcore-zu-Port-Mapping

### Mapping-Syntax
Cores zu Ports mit dem Format `[lcore_list].port` zuweisen:
- `[4:5].0`: lcores 4 und 5 verarbeiten Port 0
- `[6:7].1`: lcores 6 und 7 verarbeiten Port 1
- `[4].0`: Einzelner lcore 4 verarbeitet Port 0
- `[4-7].0`: lcores 4 bis 7 verarbeiten Port 0

### Leistungsoptimierung
- **Dedizierte Cores**: Separate Cores für TX und RX zuweisen
- **NUMA-Bewusstsein**: Cores lokal zu Netzwerk-Interfaces verwenden
- **Konfliktvermeidung**: Überlappungen mit System-Workern vermeiden
- **Lastverteilung**: Traffic über mehrere Cores verteilen

## Betrieb und Überwachung

### Status-Befehle
```bash
# PKTGEN-Status und Konfiguration anzeigen
show pktgen

# Worker-Zuweisungen und Leistung anzeigen
show worker statistics

# Port-Level-Statistiken
show port statistics all
```

### Laufzeit-Management
PKTGEN bietet umfangreiche Laufzeit-Konfiguration über interaktive CLI:
- **Traffic-Parameter**: Paketgröße, Rate, Muster
- **Start/Stop-Kontrolle**: Port-spezifische Traffic-Kontrolle
- **Statistiken**: Echtzeit-Leistungsüberwachung
- **Bereichstests**: Automatisierte Parameter-Sweeps

### Leistungsüberwachung
```bash
# Echtzeit-Statistiken überwachen
# (Verfügbar über PKTGEN-interaktive Schnittstelle)

# Hauptmetriken:
# - TX/RX-Pakete pro Sekunde
# - Bandbreitennutzung
# - Paketverlustrate
# - Latenzmessungen
```

## CLI-Befehle

### PKTGEN-Kontrollbefehle

#### Initialisierung
```bash
# PKTGEN mit argv-list-Konfiguration initialisieren
pktgen init argv-list <0-7>
```

#### Traffic-Generierung starten/stoppen
```bash
# Traffic-Generierung auf spezifischem Port starten
pktgen do start port <0-7>
pktgen do start port all

# Traffic-Generierung stoppen
pktgen do stop port <0-7>
pktgen do stop port all
```

#### Traffic-Konfigurationsbefehle

##### Paketzahl konfigurieren
```bash
# Anzahl der zu sendenden Pakete konfigurieren
pktgen do set port <0-7> count <0-4000000000>
pktgen do set port all count <0-4000000000>
```

##### Paketgröße konfigurieren
```bash
# Paketgröße in Bytes konfigurieren
pktgen do set port <0-7> size <0-9999>
pktgen do set port all size <0-9999>
```

##### Übertragungsrate konfigurieren
```bash
# Übertragungsrate in Prozent konfigurieren
pktgen do set port <0-7> rate <0-100>
pktgen do set port all rate <0-100>
```

##### TCP/UDP-Portnummern konfigurieren
```bash
# TCP-Quell- und Zielports konfigurieren
pktgen do set port <0-7> tcp src <0-65535> dst <0-65535>
pktgen do set port all tcp src <0-65535> dst <0-65535>

# UDP-Quell- und Zielports konfigurieren
pktgen do set port <0-7> udp src <0-65535> dst <0-65535>
pktgen do set port all udp src <0-65535> dst <0-65535>
```

##### TTL-Wert konfigurieren
```bash
# IP Time-to-Live-Wert konfigurieren
pktgen do set port <0-7> ttl <0-255>
pktgen do set port all ttl <0-255>
```

##### MAC-Adressen konfigurieren
```bash
# Quell-MAC-Adresse konfigurieren
pktgen do set port <0-7> mac source <MAC>
pktgen do set port all mac source <MAC>

# Ziel-MAC-Adresse konfigurieren
pktgen do set port <0-7> mac destination <MAC>
pktgen do set port all mac destination <MAC>
```

##### IPv4-Adressen konfigurieren
```bash
# Quell-IPv4-Adresse konfigurieren
pktgen do set port <0-7> ipv4 source <IPv4>
pktgen do set port all ipv4 source <IPv4>

# Ziel-IPv4-Adresse konfigurieren
pktgen do set port <0-7> ipv4 destination <IPv4>
pktgen do set port all ipv4 destination <IPv4>
```

#### Status- und Überwachungsbefehle
```bash
# PKTGEN-Status und Konfiguration anzeigen
show pktgen

# Port-Statistiken anzeigen
show port statistics all
show port statistics <0-7>
```

## Anwendungsfälle

### Netzwerk-Leistungstests
- **Durchsatztests**: Maximale Bandbreitenmessung
- **Latenztests**: Ende-zu-Ende-Latenzanalyse
- **Lasttests**: Nachhaltige Traffic-Generierung
- **Stresstests**: Maximale Paketrate-Verifikation

### Gerätevalidierung
- **Switch-Tests**: Weiterleitungsleistungs-Verifikation
- **Router-Tests**: L3-Weiterleitungsleistung
- **Interface-Tests**: Port- und Kabelverifikation
- **Protokolltests**: Spezifisches Protokollverhalten

### Netzwerkentwicklung
- **Protokollentwicklung**: Test neuer Netzwerkprotokolle
- **Anwendungstests**: Realistische Traffic-Muster-Generierung
- **Leistungstuning**: Netzwerkkonfigurations-Optimierung
- **Benchmarking**: Standardisierte Leistungsvergleiche

## Leistungstuning

### Core-Zuweisungen
```bash
# Core-Verwendung für höchste Leistung optimieren
# TX- und RX-Cores trennen wenn möglich
# NUMA-lokale Cores zu Netzwerk-Interfaces verwenden
set worker lcore 4 pktgen  # Port 0 TX-Core
set worker lcore 5 pktgen  # Port 0 RX-Core
```

### Speicherkonfiguration
```bash
# Deskriptor-Ringe für Traffic-Muster optimieren
set port all nrxdesc 2048  # Für hohe Raten erhöhen
set port all ntxdesc 2048  # Für Bursts erhöhen
```

### System-Tuning
- **CPU-Isolierung**: PKTGEN-Cores vom OS-Scheduler isolieren
- **Interrupt-Affinität**: Interrupts an Nicht-PKTGEN-Cores binden
- **Speicherzuweisung**: Hugepages für optimale Leistung verwenden
- **NIC-Tuning**: Netzwerk-Interface-Konfigurationen optimieren

## Fehlerbehebung

### Häufige Probleme
- **Niedrige Leistung**: Core-Zuweisungen und NUMA-Topologie überprüfen
- **Paketverluste**: Puffergrößen und Systemressourcen überprüfen
- **Initialisierungsfehler**: Gerätebindung und Berechtigungen überprüfen
- **Ratenbegrenzung**: Interface-Fähigkeiten und Konfiguration überprüfen

### Debug-Strategien
- **Verbose-Logging aktivieren**: Höhere Log-Level für detaillierte Ausgabe verwenden
- **Statistiken überprüfen**: TX/RX-Zähler auf Anomalien überwachen
- **Mapping verifizieren**: Korrekte lcore-zu-Port-Zuweisungen sicherstellen
- **System überwachen**: CPU-, Speicher- und Interrupt-Verwendung während Tests überprüfen

#### Debug-Befehle
```bash
# PKTGEN-Debug-Logging aktivieren
debug sdplane pktgen

# Allgemeines sdplane-Debug
debug sdplane rib
debug sdplane fdb-change
```

### Leistungsverifikation
```bash
# Überprüfen, ob PKTGEN erwartete Raten erreicht
show pktgen
show port statistics all

# Auf Fehler oder Drops überprüfen
# Systemressourcen während Tests überwachen
```

## Erweiterte Funktionen

### Bereichstests
PKTGEN unterstützt automatisierte Tests über Parameterbereiche:
- **Paketgrößen-Sweeps**: Tests von 64 bis 1518 Bytes
- **Raten-Sweeps**: Tests von 1% bis 100% Leitungsrate
- **Automatische Berichte**: Generierung umfassender Testberichte

### Traffic-Muster
- **Konstante Rate**: Stabile Traffic-Generierung
- **Burst-Muster**: Traffic-Bursts mit Leerlaufperioden
- **Ramp-Muster**: Graduell steigende/fallende Raten
- **Benutzerdefinierte Muster**: Benutzerdefinierte Traffic-Profile

## Verwandte Dokumentation

- [Paketgenerierung-Befehle](packet-generation.md) - Befehlsreferenz
- [Worker-Management](worker-lcore-thread-management.md) - Worker-Konfiguration
- [Port-Management](port-management.md) - DPDK-Port-Setup
- [Leistungstuning-Leitfaden](#) - System-Optimierungstipps

## Externe Ressourcen

- [DPDK Pktgen-Dokumentation](http://pktgen-dpdk.readthedocs.io/) - Offizielle PKTGEN-Dokumentation
- [DPDK-Leistungsleitfaden](https://doc.dpdk.org/guides/prog_guide/) - DPDK-Optimierungsleitfaden