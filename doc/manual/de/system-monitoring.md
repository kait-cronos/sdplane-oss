# Systeminformationen und Überwachung

**Sprache / Language:** [English](../system-monitoring.md) | [日本語](../ja/system-monitoring.md) | [Français](../fr/system-monitoring.md) | [中文](../zh/system-monitoring.md) | **Deutsch** | [Italiano](../it/system-monitoring.md) | [한국어](../ko/system-monitoring.md) | [ไทย](../th/system-monitoring.md) | [Español](../es/system-monitoring.md)

Befehle zur Bereitstellung von Systeminformationen und Überwachungsfunktionen für sdplane.

## Befehlsübersicht

### show_version - Versionsinformationen anzeigen
```
show version
```

Zeigt die Versionsinformationen von sdplane an.

**Verwendungsbeispiel:**
```bash
show version
```

### set_locale - Locale konfigurieren
```
set locale (C|C.utf8|en_US.utf8|POSIX)
```

Konfiguriert die System-Locale.

**Verfügbare Locales:**
- `C` - Standard C-Locale
- `C.utf8` - UTF-8-kompatible C-Locale
- `en_US.utf8` - Englische UTF-8-Locale
- `POSIX` - POSIX-Locale

**Verwendungsbeispiel:**
```bash
# UTF-8-kompatible C-Locale konfigurieren
set locale C.utf8

# Englische UTF-8-Locale konfigurieren
set locale en_US.utf8
```

### set_argv_list_1 - argv-list konfigurieren
```
set argv-list <0-7> <WORD>
```

Konfiguriert die Befehlszeilenargumentliste.

**Parameter:**
- `<0-7>` - Index (0-7)
- `<WORD>` - Zu konfigurierende Zeichenkette

**Verwendungsbeispiel:**
```bash
# Argument für Index 0 konfigurieren
set argv-list 0 "--verbose"

# Argument für Index 1 konfigurieren
set argv-list 1 "--config"
```

### **show argv-list**

Zeigt alle konfigurierten Befehlszeilenargumentlisten an.

**Verwendungsbeispiel:**
```bash
# Alle argv-lists anzeigen
show argv-list
```

---

### **show argv-list \<0-7\>**

Zeigt die argv-list für einen spezifischen Index an.

**Verwendungsbeispiel:**
```bash
# argv-list für spezifischen Index anzeigen
show argv-list 0

# argv-list Index 3 anzeigen
show argv-list 3
```

### show_loop_count - Loop-Counter anzeigen
```
show loop-count (console|vty-shell|l2fwd) (pps|total)
```

Zeigt die Loop-Counter für jede Komponente an.

**Komponenten:**
- `console` - Konsole
- `vty-shell` - VTY-Shell
- `l2fwd` - L2-Forwarding

**Statistiktypen:**
- `pps` - Loops pro Sekunde
- `total` - Gesamt-Loop-Anzahl

**Verwendungsbeispiel:**
```bash
# Konsolen-PPS anzeigen
show loop-count console pps

# Gesamt-Loop-Anzahl für L2-Forwarding anzeigen
show loop-count l2fwd total
```

### show_rcu - RCU-Informationen anzeigen
```
show rcu
```

Zeigt RCU (Read-Copy-Update) Informationen an.

**Verwendungsbeispiel:**
```bash
show rcu
```

### show_fdb - FDB-Informationen anzeigen
```
show fdb
```

Zeigt FDB (Forwarding Database) Informationen an.

**Verwendungsbeispiel:**
```bash
show fdb
```

### show_vswitch - vswitch-Informationen anzeigen
```
show vswitch
```

Zeigt Informationen über den virtuellen Switch an.

**Verwendungsbeispiel:**
```bash
show vswitch
```

### sleep_cmd - Sleep-Befehl
```
sleep <0-300>
```

Pausiert für die angegebene Anzahl von Sekunden.

**Parameter:**
- `<0-300>` - Sleep-Zeit (Sekunden)

**Verwendungsbeispiel:**
```bash
# 5 Sekunden pausieren
sleep 5

# 30 Sekunden pausieren
sleep 30
```

### show_mempool - Speicherpool-Informationen anzeigen
```
show mempool
```

Zeigt DPDK-Speicherpool-Informationen an.

**Verwendungsbeispiel:**
```bash
show mempool
```

## Beschreibung der Überwachungskomponenten

### Versionsinformationen
- sdplane-Version
- Build-Informationen
- Versionen der abhängigen Bibliotheken

### Loop-Counter
- Anzahl der Verarbeitungsschleifen für jede Komponente
- Zur Leistungsüberwachung verwendet
- Zur Berechnung von PPS (Packets Per Second) verwendet

### RCU-Informationen
- Status des Read-Copy-Update-Mechanismus
- Zustand der Synchronisationsverarbeitung
- Speicherverwaltungsstatus

### FDB-Informationen
- Status der MAC-Adresstabelle
- Gelernte MAC-Adressen
- Aging-Informationen

### vswitch-Informationen
- Konfiguration des virtuellen Switches
- Port-Informationen
- VLAN-Konfiguration

### Speicherpool-Informationen
- Verfügbarer Speicher
- Verwendeter Speicher
- Speicherpool-Statistiken

## Best Practices für die Überwachung

### Regelmäßige Überwachung
```bash
# Grundlegende Überwachungsbefehle
show version
show mempool
show vswitch
show rcu
```

### Leistungsüberwachung
```bash
# Leistungsüberwachung über Loop-Counter
show loop-count console pps
show loop-count l2fwd pps
```

### Fehlerbehebung
```bash
# Systemstatusüberprüfung
show fdb
show vswitch
show mempool
```

## Definitionsort

Diese Befehle sind in den folgenden Dateien definiert:
- `sdplane/sdplane.c`

## Verwandte Themen

- [Port-Management und Statistiken](port-management.md)
- [Worker- und lcore-Management](worker-management.md)
- [Thread-Informationen](thread-information.md)