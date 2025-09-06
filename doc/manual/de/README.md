# sdplane-oss Benutzerhandbuch

**Language:** [English](../README.md) | [日本語](../ja/README.md) | [Français](../fr/README.md) | [中文](../zh/README.md) | **Deutsch**

sdplane-oss ist ein hochleistungsfähiger DPDK-basierter Software-Router. Dieses Benutzerhandbuch beschreibt alle Befehle und Funktionen von sdplane.

## Inhaltsverzeichnis

1. [Port-Verwaltung & Statistiken](port-management.md) - DPDK-Port-Verwaltung und Statistiken
2. [Worker & lcore-Verwaltung](worker-management.md) - Worker-Threads und lcore-Verwaltung
3. [Debug & Logging](debug-logging.md) - Debug- und Logging-Funktionen
4. [VTY & Shell-Verwaltung](vty-shell.md) - VTY- und Shell-Verwaltung
5. [System-Information & Überwachung](system-monitoring.md) - Systeminformationen und Überwachung
6. [RIB & Routing](routing.md) - RIB- und Routing-Funktionen
7. [Warteschlangen-Konfiguration](queue-configuration.md) - Warteschlangen-Konfiguration und -verwaltung
8. [Paketgenerierung](packet-generation.md) - Paketgenerierung mit PKTGEN
9. [Thread-Informationen](thread-information.md) - Thread-Informationen und Überwachung
10. [TAP-Interface](tap-interface.md) - TAP-Interface-Verwaltung
11. [lthread-Verwaltung](lthread-management.md) - lthread-Verwaltung
12. [Geräteverwaltung](device-management.md) - Geräte- und Treiberverwaltung

## Grundlegende Verwendung

### Verbindungsmethode

Um eine Verbindung zu sdplane herzustellen:

```bash
# sdplane starten
sudo ./sdplane/sdplane

# Vom anderen Terminal aus mit CLI verbinden
telnet localhost 9882
```

### Hilfe anzeigen

Bei jedem Befehl können Sie `?` verwenden, um Hilfe anzuzeigen:

```
sdplane# ?
sdplane# show ?
sdplane# set ?
```

### Grundlegende Befehle

- `show version` - Versionsinformationen anzeigen
- `show port` - Port-Informationen anzeigen
- `show worker` - Worker-Informationen anzeigen
- `exit` - CLI beenden

## Befehlsklassifizierung

sdplane hat 79 definierte Befehle, die in die folgenden 13 Funktionskategorien unterteilt sind:

1. **Port-Verwaltung & Statistiken** (10 Befehle) - DPDK-Port-Steuerung und Statistiken
2. **Worker & lcore-Verwaltung** (6 Befehle) - Worker-Threads und lcore-Verwaltung
3. **Debug & Logging** (2 Befehle) - Debug- und Logging-Funktionen
4. **VTY & Shell-Verwaltung** (4 Befehle) - VTY- und Shell-Steuerung
5. **System-Information & Überwachung** (10 Befehle) - Systeminformationen und Überwachung
6. **RIB & Routing** (1 Befehl) - Routing-Informationen-Verwaltung
7. **Warteschlangen-Konfiguration** (3 Befehle) - Warteschlangen-Einstellung
8. **Paketgenerierung** (3 Befehle) - Paketgenerierung mit PKTGEN
9. **Thread-Informationen** (2 Befehle) - Thread-Überwachung
10. **TAP-Interface** (2 Befehle) - TAP-Interface-Verwaltung
11. **lthread-Verwaltung** (3 Befehle) - lthread-Verwaltung
12. **Geräteverwaltung** (2 Befehle) - Geräte- und Treiberverwaltung

Für detaillierte Verwendungshinweise siehe die Dokumentation jeder Kategorie.