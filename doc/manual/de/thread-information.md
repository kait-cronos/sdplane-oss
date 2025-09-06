# Thread-Informationen

**Sprache / Language:** [English](../thread-information.md) | [日本語](../ja/thread-information.md) | [Français](../fr/thread-information.md) | [中文](../zh/thread-information.md) | **Deutsch**

Befehle zur Thread-Information und Überwachung.

## Befehlsübersicht

### show_thread_cmd - Thread-Informationen anzeigen
```
show thread
```

Zeigt den aktuellen Thread-Status und Informationen an.

**Verwendungsbeispiel:**
```bash
show thread
```

Dieser Befehl zeigt folgende Informationen an:
- Thread-ID
- Thread-Status
- Laufende Aufgaben
- CPU-Auslastung
- Speicherverbrauch

### show_thread_counter - Thread-Counter anzeigen
```
show thread counter
```

Zeigt Thread-Counter-Informationen an.

**Verwendungsbeispiel:**
```bash
show thread counter
```

Dieser Befehl zeigt folgende Informationen an:
- Anzahl verarbeiteter Pakete
- Ausführungsanzahl
- Fehleranzahl
- Verarbeitungszeit-Statistiken

## Übersicht des Thread-Systems

### sdplane-Thread-Architektur
In sdplane werden folgende Thread-Typen verwendet:

#### 1. Worker-Threads
- **L2-Forwarding** - Layer 2-Paketweiterleitung
- **L3-Forwarding** - Layer 3-Paketweiterleitung
- **TAP-Handler** - TAP-Interface-Verarbeitung
- **PKTGEN** - Paketgenerierung

#### 2. Management-Threads
- **RIB-Manager** - Routing-Informationsverwaltung
- **Statistik-Collector** - Statistikinformationssammlung
- **Netlink-Thread** - Netlink-Kommunikationsverarbeitung

#### 3. System-Threads
- **VTY-Server** - VTY-Verbindungsverarbeitung
- **Konsole** - Konsolen-Ein/Ausgabe

### Beziehung zu lthread
sdplane verwendet ein kooperatives Multithreading-Modell:

- **lthread** - Leichtgewicht-Thread-Implementierung
- **Kooperative Planung** - Kontrolle durch explizites Yield
- **Hohe Effizienz** - Reduzierter Kontext-Switch-Overhead

## Interpretation der Thread-Informationen

### Grundlegende Anzeigeelemente
- **Thread ID** - Thread-Identifikator
- **Name** - Thread-Name
- **State** - Thread-Status
- **lcore** - Laufender CPU-Core
- **Type** - Thread-Typ

### Thread-Status
- **Running** - Laufend
- **Ready** - Ausführungsbereit
- **Blocked** - Blockiert
- **Terminated** - Beendet

### Counter-Informationen
- **Packets** - Anzahl verarbeiteter Pakete
- **Loops** - Anzahl Loop-Ausführungen
- **Errors** - Fehleranzahl
- **CPU Time** - CPU-Verwendungszeit

## Verwendungsbeispiele

### Grundlegende Überwachung
```bash
# Thread-Informationen anzeigen
show thread

# Thread-Counter anzeigen
show thread counter
```

### Interpretation von Ausgabebeispielen
```bash
# show thread Ausgabebeispiel
Thread ID: 1
Name: l2fwd-worker
State: Running
lcore: 1
Type: L2FWD

Thread ID: 2
Name: rib-manager
State: Running
lcore: 2
Type: RIB_MANAGER
```

```bash
# show thread counter Ausgabebeispiel
Thread ID: 1
Packets: 1000000
Loops: 5000000
Errors: 0
CPU Time: 123.45s
```

## Überwachung und Fehlerbehebung

### Regelmäßige Überwachung
```bash
# Regelmäßige Überwachungsbefehle
show thread
show thread counter
```

### Leistungsanalyse
```bash
# Leistungsbezogene Informationen
show thread counter
show loop-count l2fwd pps
show worker
```

### Fehlerbehebung

#### Wenn Thread nicht reagiert
1. Thread-Status überprüfen
```bash
show thread
```

2. Worker-Status überprüfen
```bash
show worker
```

3. Bei Bedarf neu starten
```bash
restart worker lcore 1
```

#### Bei Leistungsabfall
1. Counter-Informationen überprüfen
```bash
show thread counter
```

2. Loop-Counter überprüfen
```bash
show loop-count l2fwd pps
```

3. Fehleranzahl überprüfen
```bash
show thread counter
```

#### Bei hohem Speicherverbrauch
1. Speicherpool-Informationen überprüfen
```bash
show mempool
```

2. Thread-Informationen überprüfen
```bash
show thread
```

## Thread-Optimierung

### CPU-Affinitäts-Konfiguration
- Angemessene Worker-Platzierung auf lcores
- Berücksichtigung von NUMA-Knoten
- Gleichmäßige CPU-Auslastung

### Verbesserung der Speichereffizienz
- Angemessene Speicherpool-Größe
- Vermeidung von Speicherlecks
- Verbesserung der Cache-Effizienz

## Erweiterte Funktionen

### lthread-Management
```bash
# lthread-Worker-Konfiguration
set worker lthread stat-collector
set worker lthread rib-manager
set worker lthread netlink-thread
```

### Nutzung von Statistikinformationen
- Leistungsüberwachung
- Kapazitätsplanung
- Anomalieerkennung

## Definitionsort

Diese Befehle sind in den folgenden Dateien definiert:
- `sdplane/thread_info.c`

## Verwandte Themen

- [Worker- und lcore-Management](worker-management.md)
- [lthread-Management](lthread-management.md)
- [Systeminformationen und Überwachung](system-monitoring.md)