# lthread-Management

**Sprache / Language:** [English](../en/lthread-management.md) | [日本語](../ja/lthread-management.md) | [Français](../fr/lthread-management.md) | [中文](../zh/lthread-management.md) | **Deutsch** | [Italiano](../it/lthread-management.md) | [한국어](../ko/lthread-management.md) | [ไทย](../th/lthread-management.md) | [Español](../es/lthread-management.md)

Befehle zur Verwaltung von lthread (Leichtgewicht-Threads).

## Befehlsübersicht

### set_worker_lthread_stat_collector - lthread-Statistik-Collector konfigurieren
```
set worker lthread stat-collector
```

Konfiguriert einen lthread-Worker zur Statistikinformationssammlung.

**Verwendungsbeispiel:**
```bash
set worker lthread stat-collector
```

### set_worker_lthread_rib_manager - lthread-RIB-Manager konfigurieren
```
set worker lthread rib-manager
```

Konfiguriert einen lthread-Worker zur RIB (Routing Information Base) Verwaltung.

**Verwendungsbeispiel:**
```bash
set worker lthread rib-manager
```

### set_worker_lthread_netlink_thread - lthread-Netlink-Thread konfigurieren
```
set worker lthread netlink-thread
```

Konfiguriert einen lthread-Worker zur Verarbeitung der Netlink-Kommunikation.

**Verwendungsbeispiel:**
```bash
set worker lthread netlink-thread
```

## lthread-Übersicht

### Was ist lthread?
lthread (lightweight thread) ist eine Leichtgewicht-Thread-Implementierung für kooperatives Multithreading.

### Hauptmerkmale
- **Leichtgewicht** - Geringer Speicher-Overhead
- **Hochgeschwindigkeit** - Schneller Kontext-Switch
- **Kooperativ** - Kontrolle durch explizites Yield
- **Skalierbarkeit** - Effiziente Verwaltung großer Thread-Mengen

### Unterschied zu herkömmlichen Threads
- **Präemptiv vs Kooperativ** - Explizite Kontrollabtretung
- **Kernel-Thread vs User-Thread** - Ausführung ohne Kernel-Beteiligung
- **Schwer vs Leicht** - Geringer Overhead bei Erstellung/Wechsel

## lthread-Worker-Typen

### Statistik-Collector (stat-collector)
Worker zur regelmäßigen Sammlung von Systemstatistiken.

**Funktionen:**
- Port-Statistik-Sammlung
- Thread-Statistik-Sammlung
- System-Statistik-Sammlung
- Statistikdaten-Aggregation

**Verwendungsbeispiel:**
```bash
# Statistik-Collector konfigurieren
set worker lthread stat-collector

# Statistikinformationen überprüfen
show port statistics
show thread counter
```

### RIB-Manager (rib-manager)
Worker zur Verwaltung der RIB (Routing Information Base).

**Funktionen:**
- Routing-Tabellen-Verwaltung
- Route-Hinzufügung/-Löschung/-Aktualisierung
- Route-Informationsverteilung
- Routing-Status-Überwachung

**Verwendungsbeispiel:**
```bash
# RIB-Manager konfigurieren
set worker lthread rib-manager

# RIB-Informationen überprüfen
show rib
```

### Netlink-Thread (netlink-thread)
Worker zur Verarbeitung der Netlink-Kommunikation.

**Funktionen:**
- Netlink-Kommunikation mit dem Kernel
- Netzwerkkonfigurations-Überwachung
- Interface-Status-Überwachung
- Routing-Informations-Empfang

**Verwendungsbeispiel:**
```bash
# Netlink-Thread konfigurieren
set worker lthread netlink-thread

# Netzwerkstatus überprüfen
show port
show vswitch
```

## lthread-Konfiguration

### Grundlegende Konfigurationsschritte
1. **Erforderliche lthread-Worker konfigurieren**
```bash
# Statistik-Collector konfigurieren
set worker lthread stat-collector

# RIB-Manager konfigurieren
set worker lthread rib-manager

# Netlink-Thread konfigurieren
set worker lthread netlink-thread
```

2. **Konfiguration überprüfen**
```bash
# Worker-Status überprüfen
show worker

# Thread-Informationen überprüfen
show thread
```

### Empfohlene Konfiguration
Für allgemeine Verwendung wird folgende Kombination empfohlen:
```bash
# Grundlegende lthread-Worker-Konfiguration
set worker lthread stat-collector
set worker lthread rib-manager
set worker lthread netlink-thread
```

## Verwendungsbeispiele

### Grundlegende Konfiguration
```bash
# lthread-Worker konfigurieren
set worker lthread stat-collector
set worker lthread rib-manager
set worker lthread netlink-thread

# Konfiguration überprüfen
show worker
show thread
```

### Statistik-Überwachungs-Konfiguration
```bash
# Statistik-Collector konfigurieren
set worker lthread stat-collector

# Statistikinformationen regelmäßig überprüfen
show port statistics
show thread counter
show mempool
```

### Routing-Management-Konfiguration
```bash
# RIB-Manager konfigurieren
set worker lthread rib-manager

# Routing-Informationen überprüfen
show rib
show vswitch
```

## Überwachung und Verwaltung

### lthread-Status überprüfen
```bash
# Gesamtstatus überprüfen
show worker
show thread

# Spezifische Statistikinformationen überprüfen
show thread counter
show loop-count console pps
```

### Leistungsüberwachung
```bash
# lthread-Leistung überprüfen
show thread counter

# Systemgesamtleistung überprüfen
show port statistics pps
show mempool
```

## Fehlerbehebung

### Wenn lthread nicht funktioniert
1. Worker-Konfiguration überprüfen
```bash
show worker
```

2. Thread-Status überprüfen
```bash
show thread
```

3. Systemstatus überprüfen
```bash
show rcu
show mempool
```

### Wenn Statistikinformationen nicht aktualisiert werden
1. Statistik-Collector-Status überprüfen
```bash
show worker
show thread
```

2. Statistikinformationen manuell überprüfen
```bash
show port statistics
show thread counter
```

### Wenn RIB nicht aktualisiert wird
1. RIB-Manager-Status überprüfen
```bash
show worker
show thread
```

2. RIB-Informationen überprüfen
```bash
show rib
```

3. Netlink-Thread-Status überprüfen
```bash
show worker
```

## Erweiterte Funktionen

### Kooperative Funktionsweise von lthread
Da lthread kooperativ funktioniert, sind folgende Punkte zu beachten:

- **Explizites Yield** - Bei langen Verarbeitungen ist explizites Yield erforderlich
- **Deadlock-Vermeidung** - Deadlock-Vermeidung durch kooperatives Design
- **Fairness** - Angemessene Ausführung aller Threads

### Leistungsoptimierung
- **Angemessene Worker-Platzierung** - Berücksichtigung der CPU-Core-Affinität
- **Speichereffizienz** - Konfiguration angemessener Speicherpool-Größen
- **Lastverteilung** - Lastverteilung auf mehrere Worker

## Definitionsort

Diese Befehle sind in den folgenden Dateien definiert:
- `sdplane/lthread_main.c`

## Verwandte Themen

- [Worker- und lcore-Management](worker-lcore-thread-management.md)
- [Thread-Informationen](worker-lcore-thread-management.md)
- [Systeminformationen und Überwachung](system-monitoring.md)
- [RIB und Routing](routing.md)