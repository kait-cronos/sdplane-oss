# Paketgenerierung

**Sprache / Language:** [English](../packet-generation.md) | [日本語](../ja/packet-generation.md) | [Français](../fr/packet-generation.md) | [中文](../zh/packet-generation.md) | **Deutsch** | [Italiano](../it/packet-generation.md) | [한국어](../ko/packet-generation.md) | [ไทย](../th/packet-generation.md) | [Español](../es/packet-generation.md)

Befehle für die Paketgenerierungsfunktionen mit PKTGEN (Packet Generator).

## Befehlsübersicht

### show_pktgen - PKTGEN-Informationen anzeigen
```
show pktgen
```

Zeigt den aktuellen Status und die Konfiguration des PKTGEN (Packet Generator) an.

**Verwendungsbeispiel:**
```bash
show pktgen
```

Dieser Befehl zeigt folgende Informationen an:
- PKTGEN-Initialisierungsstatus
- Aktuelle Konfigurationsparameter
- Laufende Aufgaben
- Statistikinformationen

### pktgen_init - PKTGEN-Initialisierung
```
pktgen init argv-list <0-7>
```

Initialisiert PKTGEN mit der angegebenen argv-list.

**Parameter:**
- `<0-7>` - Index der zu verwendenden argv-list

**Verwendungsbeispiel:**
```bash
# PKTGEN mit argv-list 0 initialisieren
pktgen init argv-list 0

# PKTGEN mit argv-list 2 initialisieren
pktgen init argv-list 2
```

### pktgen_do_start - PKTGEN starten
```
pktgen do start (<0-7>|all)
```

Startet die Paketgenerierung auf dem angegebenen Port.

**Ziel:**
- `<0-7>` - Spezifische Port-Nummer
- `all` - Alle Ports

**Verwendungsbeispiel:**
```bash
# Paketgenerierung auf Port 0 starten
pktgen do start 0

# Paketgenerierung auf allen Ports starten
pktgen do start all
```

### pktgen_do_stop - PKTGEN stoppen
```
pktgen do stop (<0-7>|all)
```

Stoppt die Paketgenerierung auf dem angegebenen Port.

**Ziel:**
- `<0-7>` - Spezifische Port-Nummer
- `all` - Alle Ports

**Verwendungsbeispiel:**
```bash
# Paketgenerierung auf Port 1 stoppen
pktgen do stop 1

# Paketgenerierung auf allen Ports stoppen
pktgen do stop all
```

## PKTGEN-Übersicht

### Was ist PKTGEN?
PKTGEN (Packet Generator) ist ein Paketgenerierungstool für Netzwerktests. Es bietet folgende Funktionen:

- **Hochgeschwindigkeits-Paketgenerierung** - Hochleistungs-Paketgenerierung
- **Vielfältige Paketformate** - Unterstützung verschiedener Protokolle
- **Flexible Konfiguration** - Detaillierte Paketkonfiguration möglich
- **Statistikfunktionen** - Bereitstellung detaillierter Statistikinformationen

### Hauptanwendungen
- **Netzwerk-Leistungstests** - Messung von Durchsatz und Latenz
- **Lasttests** - Testen der Lastresistenz des Systems
- **Funktionstests** - Verifizierung von Netzwerkfunktionen
- **Benchmarks** - Leistungsvergleichstests

## PKTGEN-Konfiguration

### Grundlegende Konfigurationsschritte
1. **argv-list-Konfiguration**
```bash
# Parameter für PKTGEN konfigurieren
set argv-list 0 "-c 0x3 -n 4"
set argv-list 1 "--socket-mem 1024"
set argv-list 2 "--huge-dir /mnt/huge"
```

2. **PKTGEN-Initialisierung**
```bash
pktgen init argv-list 0
```

3. **Worker-Konfiguration**
```bash
set worker lcore 1 pktgen
start worker lcore 1
```

4. **Paketgenerierung starten**
```bash
pktgen do start 0
```

### Konfigurationsparameter
Beispiele für in argv-list konfigurierbare Parameter:

- **-c** - CPU-Maske
- **-n** - Anzahl Speicherkanäle
- **--socket-mem** - Socket-Speichergröße
- **--huge-dir** - Hugepage-Verzeichnis
- **--file-prefix** - Dateipräfix

## Verwendungsbeispiele

### Grundlegende Paketgenerierung
```bash
# Konfiguration
set argv-list 0 "-c 0x3 -n 4 --socket-mem 1024"

# Initialisierung
pktgen init argv-list 0

# Worker-Konfiguration
set worker lcore 1 pktgen
start worker lcore 1

# Paketgenerierung starten
pktgen do start 0

# Status überprüfen
show pktgen

# Paketgenerierung stoppen
pktgen do stop 0
```

### Generierung auf mehreren Ports
```bash
# Auf mehreren Ports starten
pktgen do start all

# Status überprüfen
show pktgen

# Auf mehreren Ports stoppen
pktgen do stop all
```

## Überwachung und Statistiken

### Statistikinformationen überprüfen
```bash
# PKTGEN-Statistiken anzeigen
show pktgen

# Port-Statistiken anzeigen
show port statistics

# Worker-Statistiken anzeigen
show worker
```

### Leistungsüberwachung
```bash
# PPS (Packets Per Second) überprüfen
show port statistics pps

# Gesamtpaketanzahl überprüfen
show port statistics total

# Bytes/Sekunde überprüfen
show port statistics Bps
```

## Fehlerbehebung

### Wenn PKTGEN nicht startet
1. Initialisierungsstatus überprüfen
```bash
show pktgen
```

2. Worker-Status überprüfen
```bash
show worker
```

3. Port-Status überprüfen
```bash
show port
```

### Wenn Paketgenerierung nicht stoppt
1. Explizit stoppen
```bash
pktgen do stop all
```

2. Worker neu starten
```bash
restart worker lcore 1
```

### Bei niedriger Leistung
1. CPU-Auslastung überprüfen
2. Speicherkonfiguration überprüfen
3. Port-Konfiguration überprüfen

## Erweiterte Funktionen

### Paketformat-Konfiguration
PKTGEN kann verschiedene Paketformate generieren:
- **Ethernet** - Grundlegende Ethernet-Frames
- **IP** - IPv4/IPv6-Pakete
- **UDP/TCP** - UDP/TCP-Pakete
- **VLAN** - VLAN-getaggte Pakete

### Lastkontrolle
- **Ratenkontrolle** - Kontrolle der Paketgenerierungsrate
- **Burst-Kontrolle** - Generierung von Burst-Paketen
- **Größenkontrolle** - Kontrolle der Paketgröße

## Definitionsort

Diese Befehle sind in den folgenden Dateien definiert:
- `sdplane/pktgen_cmd.c`

## Verwandte Themen

- [Worker- und lcore-Management](worker-management.md)
- [Port-Management und Statistiken](port-management.md)
- [Systeminformationen und Überwachung](system-monitoring.md)