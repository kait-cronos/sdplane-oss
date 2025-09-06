# Debug und Logging

**Sprache / Language:** [English](../debug-logging.md) | [日本語](../ja/debug-logging.md) | [Français](../fr/debug-logging.md) | [中文](../zh/debug-logging.md) | **Deutsch** | [Italiano](../it/debug-logging.md) | [한국어](../ko/debug-logging.md) | [ไทย](../th/debug-logging.md) | [Español](../es/debug-logging.md)

Befehle zur Steuerung der Debug- und Logging-Funktionen von sdplane.

## Befehlsübersicht

### debug_sdplane - sdplane-Debug-Konfiguration
```
debug sdplane [Kategorie] [Level]
```

Konfiguriert das Debug-Logging für sdplane. Dieser Befehl wird dynamisch generiert, daher werden verfügbare Kategorien und Level zur Laufzeit bestimmt.

**Verwendungsbeispiel:**
```bash
# Debug-Konfiguration aktivieren
debug sdplane

# Debug für spezifische Kategorie aktivieren
debug sdplane [category] [level]
```

**Hinweis:** Spezifische Kategorien und Level können mit dem `show debugging sdplane` Befehl eingesehen werden.

### show_debug_sdplane - sdplane-Debug-Informationen anzeigen
```
show debugging sdplane
```

Zeigt die aktuelle sdplane-Debug-Konfiguration an.

**Verwendungsbeispiel:**
```bash
show debugging sdplane
```

Dieser Befehl zeigt folgende Informationen an:
- Aktuell aktivierte Debug-Kategorien
- Debug-Level für jede Kategorie
- Verfügbare Debug-Optionen

## Übersicht des Debug-Systems

Das Debug-System von sdplane hat folgende Eigenschaften:

### Kategoriebasiertes Debug
- Debug-Kategorien sind nach verschiedenen Funktionsmodulen unterteilt
- Debug-Logs können nur für benötigte Funktionen aktiviert werden

### Levelbasierte Steuerung
- Debug-Nachrichten sind nach Wichtigkeit in Level unterteilt
- Durch Setzen des angemessenen Levels können nur benötigte Informationen angezeigt werden

### Dynamische Konfiguration
- Debug-Konfiguration kann während des Systembetriebs geändert werden
- Debug-Level können ohne Neustart angepasst werden

## Verwendung

### 1. Aktuelle Debug-Konfiguration prüfen
```bash
show debugging sdplane
```

### 2. Debug-Kategorien prüfen
Verwenden Sie den `show debugging sdplane` Befehl, um verfügbare Kategorien zu prüfen.

### 3. Debug-Konfiguration ändern
```bash
# Debug für spezifische Kategorie aktivieren
debug sdplane [category] [level]
```

### 4. Debug-Logs prüfen
Debug-Logs werden auf die Standardausgabe oder in Log-Dateien ausgegeben.

## Fehlerbehebung

### Wenn Debug-Logs nicht ausgegeben werden
1. Prüfen Sie, ob die Debug-Kategorie korrekt konfiguriert ist
2. Prüfen Sie, ob das Debug-Level angemessen konfiguriert ist
3. Prüfen Sie, ob das Log-Ausgabeziel korrekt konfiguriert ist

### Auswirkungen auf die Leistung
- Die Aktivierung von Debug-Logs kann die Leistung beeinträchtigen
- In Produktionsumgebungen wird empfohlen, nur das Minimum an notwendigem Debug zu aktivieren

## Definitionsort

Diese Befehle sind in den folgenden Dateien definiert:
- `sdplane/debug_sdplane.c`

## Verwandte Themen

- [Systeminformationen und Überwachung](system-monitoring.md)
- [VTY- und Shell-Management](vty-shell.md)