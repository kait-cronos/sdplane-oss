# Worker- und lcore-Management

**Sprache / Language:** [English](../worker-management.md) | [日本語](../ja/worker-management.md) | [Français](../fr/worker-management.md) | [中文](../zh/worker-management.md) | **Deutsch**

Befehle zur Verwaltung von DPDK-Worker-Threads und lcores.

## Befehlsübersicht

### set_worker - Worker-Typ-Konfiguration
```
set worker lcore <0-16> (|none|l2fwd|l3fwd|l3fwd-lpm|tap-handler|l2-repeater|enhanced-repeater|vlan-switch|pktgen|linkflap-generator)
```

Konfiguriert den Worker-Typ für den angegebenen lcore.

**Worker-Typen:**
- `none` - Kein Worker
- `l2fwd` - Layer 2 Forwarding
- `l3fwd` - Layer 3 Forwarding
- `l3fwd-lpm` - Layer 3 Forwarding (LPM)
- `tap-handler` - TAP-Interface-Handler
- `l2-repeater` - Layer 2 Repeater
- `enhanced-repeater` - Erweiterter Repeater mit VLAN-Switching und TAP-Interface
- `vlan-switch` - VLAN-Switch
- `pktgen` - Paket-Generator
- `linkflap-generator` - Link-Flap-Generator

**Verwendungsbeispiel:**
```bash
# L2-Forwarding-Worker für lcore 1 konfigurieren
set worker lcore 1 l2fwd

# Erweiterten Repeater-Worker für lcore 1 konfigurieren
set worker lcore 1 enhanced-repeater

# Kein Worker für lcore 2 konfigurieren
set worker lcore 2 none

# L3-Forwarding(LPM)-Worker für lcore 3 konfigurieren
set worker lcore 3 l3fwd-lpm
```

### reset_worker - Worker-Reset
```
reset worker lcore <0-16>
```

Setzt den Worker für den angegebenen lcore zurück.

**Verwendungsbeispiel:**
```bash
# Worker für lcore 2 zurücksetzen
reset worker lcore 2
```

### start_worker - Worker starten
```
start worker lcore <0-16>
```

Startet den Worker für den angegebenen lcore.

**Verwendungsbeispiel:**
```bash
# Worker für lcore 1 starten
start worker lcore 1
```

### restart_worker - Worker neustarten
```
restart worker lcore <0-16>
```

Startet den Worker für den angegebenen lcore neu.

**Verwendungsbeispiel:**
```bash
# Worker für lcore 4 neustarten
restart worker lcore 4
```

### start_worker_all - Worker starten (mit all-Option)
```
start worker lcore (<0-16>|all)
```

Startet den Worker für den angegebenen lcore oder alle lcores.

**Verwendungsbeispiel:**
```bash
# Worker für lcore 1 starten
start worker lcore 1

# Alle Worker starten
start worker lcore all
```

### stop_worker - Worker stoppen
```
stop worker lcore (<0-16>|all)
```

Stoppt den Worker für den angegebenen lcore oder alle lcores.

**Verwendungsbeispiel:**
```bash
# Worker für lcore 1 stoppen
stop worker lcore 1

# Alle Worker stoppen
stop worker lcore all
```

### reset_worker_all - Worker zurücksetzen (mit all-Option)
```
reset worker lcore (<0-16>|all)
```

Setzt den Worker für den angegebenen lcore oder alle lcores zurück.

**Verwendungsbeispiel:**
```bash
# Worker für lcore 2 zurücksetzen
reset worker lcore 2

# Alle Worker zurücksetzen
reset worker lcore all
```

### restart_worker_all - Worker neustarten (mit all-Option)
```
restart worker lcore (<0-16>|all)
```

Startet den Worker für den angegebenen lcore oder alle lcores neu.

**Verwendungsbeispiel:**
```bash
# Worker für lcore 3 neustarten
restart worker lcore 3

# Alle Worker neustarten
restart worker lcore all
```

### show_worker - Worker-Informationen anzeigen
```
show worker
```

Zeigt den aktuellen Zustand und die Konfiguration der Worker an.

**Verwendungsbeispiel:**
```bash
show worker
```

### set_mempool - Speicherpool konfigurieren
```
set mempool
```

Konfiguriert den DPDK-Speicherpool.

**Verwendungsbeispiel:**
```bash
set mempool
```

### set_rte_eal_argv - RTE EAL-Befehlszeilenargumente konfigurieren
```
set rte_eal argv <WORD> <WORD> <WORD> <WORD> <WORD> <WORD>
```

Konfiguriert die Befehlszeilenargumente für die RTE EAL (Environment Abstraction Layer) Initialisierung.

**Verwendungsbeispiel:**
```bash
# EAL-Argumente konfigurieren
set rte_eal argv -c 0x1 -n 4 --socket-mem 1024,1024 --huge-dir /mnt/huge
```

### rte_eal_init - RTE EAL-Initialisierung
```
rte_eal_init
```

Initialisiert den RTE EAL (Environment Abstraction Layer).

**Verwendungsbeispiel:**
```bash
rte_eal_init
```

## Beschreibung der Worker-Typen

### L2-Forwarding (l2fwd)
Worker für Paketweiterleitung auf Layer 2-Ebene. Leitet Pakete basierend auf MAC-Adressen weiter.

### L3-Forwarding (l3fwd)
Worker für Paketweiterleitung auf Layer 3-Ebene. Führt Routing basierend auf IP-Adressen durch.

### L3-Forwarding LPM (l3fwd-lpm)
Layer 3-Forwarding-Worker, der Longest Prefix Matching (LPM) verwendet.

### TAP-Handler (tap-handler)
Worker für Paketweiterleitung zwischen TAP-Interfaces und DPDK-Ports.

### L2-Repeater (l2-repeater)
Worker für Paketduplizierung und -weiterleitung auf Layer 2-Ebene.


### VLAN-Switch (vlan-switch)
Switching-Worker, der VLAN (Virtual LAN) Funktionalität bereitstellt.

### Paket-Generator (pktgen)
Worker für die Generierung von Test-Paketen.

### Link-Flap-Generator (linkflap-generator)
Worker zum Testen von Netzwerklink-Statusänderungen.

## Definitionsort

Diese Befehle sind in den folgenden Dateien definiert:
- `sdplane/dpdk_lcore_cmd.c`

## Verwandte Themen

- [Port-Management und Statistiken](port-management.md)
- [Thread-Informationen](thread-information.md)
- [lthread-Management](lthread-management.md)