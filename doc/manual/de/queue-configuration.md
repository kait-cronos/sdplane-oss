# Queue-Konfiguration

**Sprache / Language:** [English](../queue-configuration.md) | [日本語](../ja/queue-configuration.md) | [Français](../fr/queue-configuration.md) | [中文](../zh/queue-configuration.md) | **Deutsch** | [Italiano](../it/queue-configuration.md) | [한국어](../ko/queue-configuration.md) | [ไทย](../th/queue-configuration.md) | [Español](../es/queue-configuration.md)

Befehle zur Konfiguration und Verwaltung von DPDK-Queues.

## Befehlsübersicht

### update_port_status - Port-Status aktualisieren
```
update port status
```

Aktualisiert den Status aller Ports.

**Verwendungsbeispiel:**
```bash
update port status
```

Dieser Befehl führt folgende Aktionen aus:
- Überprüft den Link-Status jedes Ports
- Aktualisiert Queue-Konfigurationen
- Aktualisiert Port-Statistiken

### set_thread_lcore_port_queue - Thread-Queue-Konfiguration
```
set thread <0-128> port <0-128> queue <0-128>
```

Weist dem angegebenen Thread einen Port und eine Queue zu.

**Parameter:**
- `<0-128>` (thread) - Thread-Nummer
- `<0-128>` (port) - Port-Nummer
- `<0-128>` (queue) - Queue-Nummer

**Verwendungsbeispiel:**
```bash
# Thread 0 Port 0 Queue 0 zuweisen
set thread 0 port 0 queue 0

# Thread 1 Port 1 Queue 1 zuweisen
set thread 1 port 1 queue 1

# Thread 2 Port 0 Queue 1 zuweisen
set thread 2 port 0 queue 1
```

### show_thread_qconf - Thread-Queue-Konfiguration anzeigen
```
show thread qconf
```

Zeigt die aktuelle Thread-Queue-Konfiguration an.

**Verwendungsbeispiel:**
```bash
show thread qconf
```

## Übersicht des Queue-Systems

### DPDK-Queue-Konzepte
In DPDK können für jeden Port mehrere Sende- und Empfangsqueues konfiguriert werden:

- **Empfangsqueue (RX Queue)** - Empfängt eingehende Pakete
- **Sendequeue (TX Queue)** - Sendet ausgehende Pakete
- **Multi-Queue** - Parallele Verarbeitung mehrerer Queues

### Bedeutung der Queue-Konfiguration
Durch ordnungsgemäße Queue-Konfiguration wird folgendes realisiert:
- **Leistungssteigerung** - Beschleunigung durch parallele Verarbeitung
- **Lastverteilung** - Verteilung der Verarbeitung auf mehrere Worker
- **CPU-Effizienz** - Effektive Nutzung der CPU-Cores

## Queue-Konfigurationsmethoden

### Grundlegende Konfigurationsschritte
1. **Port-Status aktualisieren**
```bash
update port status
```

2. **Thread-Queue-Konfiguration**
```bash
set thread 0 port 0 queue 0
set thread 1 port 0 queue 1
```

3. **Konfiguration überprüfen**
```bash
show thread qconf
```

### Empfohlene Konfigurationsmuster

#### Einzelner Port, einzelne Queue
```bash
set thread 0 port 0 queue 0
```

#### Einzelner Port, mehrere Queues
```bash
set thread 0 port 0 queue 0
set thread 1 port 0 queue 1
set thread 2 port 0 queue 2
```

#### Mehrere Ports, mehrere Queues
```bash
set thread 0 port 0 queue 0
set thread 1 port 1 queue 0
set thread 2 port 2 queue 0
```

## Konfigurationsbeispiele

### Hochleistungskonfiguration (4 Cores, 4 Ports)
```bash
# Port-Status aktualisieren
update port status

# Jedem Core einen anderen Port zuweisen
set thread 0 port 0 queue 0
set thread 1 port 1 queue 0
set thread 2 port 2 queue 0
set thread 3 port 3 queue 0

# Konfiguration überprüfen
show thread qconf
```

### Lastverteilungskonfiguration (2 Cores, 1 Port)
```bash
# Port-Status aktualisieren
update port status

# Mehrere Queues für einen Port konfigurieren
set thread 0 port 0 queue 0
set thread 1 port 0 queue 1

# Konfiguration überprüfen
show thread qconf
```

## Performance-Tuning

### Bestimmung der Queue-Anzahl
- **CPU-Core-Anzahl** - Konfiguration entsprechend der verfügbaren CPU-Cores
- **Port-Anzahl** - Berücksichtigung der physischen Port-Anzahl
- **Traffic-Charakteristiken** - Berücksichtigung erwarteter Traffic-Muster

### Optimierungspunkte
1. **CPU-Affinität** - Angemessene Platzierung von CPU-Cores und Queues
2. **Speicherplatzierung** - Speicherplatzierung unter Berücksichtigung von NUMA-Knoten
3. **Interrupt-Verarbeitung** - Effiziente Interrupt-Verarbeitung

## Fehlerbehebung

### Wenn Queue-Konfiguration nicht übernommen wird
1. Port-Status aktualisieren
```bash
update port status
```

2. Worker-Status überprüfen
```bash
show worker
```

3. Port-Status überprüfen
```bash
show port
```

### Wenn Leistung nicht verbessert wird
1. Queue-Konfiguration überprüfen
```bash
show thread qconf
```

2. Thread-Last überprüfen
```bash
show thread counter
```

3. Port-Statistiken überprüfen
```bash
show port statistics
```

## Definitionsort

Diese Befehle sind in den folgenden Dateien definiert:
- `sdplane/queue_config.c`

## Verwandte Themen

- [Port-Management und Statistiken](port-management.md)
- [Worker- und lcore-Management](worker-lcore-thread-management.md)
- [Thread-Informationen](worker-lcore-thread-management.md)