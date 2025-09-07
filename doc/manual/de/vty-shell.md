# VTY- und Shell-Management

**Sprache / Language:** [English](../en/vty-shell.md) | [日本語](../ja/vty-shell.md) | [Français](../fr/vty-shell.md) | [中文](../zh/vty-shell.md) | **Deutsch** | [Italiano](../it/vty-shell.md) | [한국어](../ko/vty-shell.md) | [ไทย](../th/vty-shell.md) | [Español](../es/vty-shell.md)

Befehle zur Verwaltung von VTY (Virtual Terminal) und Shell.

## Befehlsübersicht

### clear_cmd - Bildschirm löschen
```
clear
```

Löscht den VTY-Bildschirm.

**Verwendungsbeispiel:**
```bash
clear
```

### vty_exit_cmd - VTY beenden
```
(exit|quit|logout)
```

Beendet die VTY-Sitzung. Mehrere Aliase sind verfügbar.

**Verwendungsbeispiel:**
```bash
exit
# oder
quit
# oder
logout
```

### shutdown_cmd - System herunterfahren
```
shutdown
```

Fährt das sdplane-System herunter.

**Verwendungsbeispiel:**
```bash
shutdown
```

**Achtung:** Dieser Befehl stoppt das gesamte System. Stellen Sie vor der Ausführung sicher, dass Konfigurationen gespeichert und aktive Sitzungen beendet wurden.

### exit_cmd - Konsole beenden
```
(exit|quit)
```

Beendet die Konsolen-Shell.

**Verwendungsbeispiel:**
```bash
exit
# oder
quit
```

## Unterschied zwischen VTY und Konsole

### VTY-Shell
- Remote-Shell über Telnet-Zugriff
- Mehrere Sitzungen gleichzeitig verwendbar
- Zugriff über Netzwerk möglich

### Konsolen-Shell
- Zugriff über lokale Konsole
- Direkter Systemzugriff
- Hauptsächlich für lokale Verwaltung

## Verbindungsmethoden

### Verbindung zu VTY
```bash
telnet localhost 9882
```

### Verbindung zur Konsole
```bash
# sdplane direkt ausführen
sudo ./sdplane/sdplane
```

## Sitzungsmanagement

### Sitzungsstatus prüfen
Der Status von VTY-Sitzungen kann mit folgendem Befehl geprüft werden:
```bash
show worker
```

### Sitzung beenden
- Normale Beendigung mit `exit`, `quit`, `logout` Befehlen
- Bei abnormaler Beendigung das gesamte System mit `shutdown` Befehl stoppen

## Sicherheitsüberlegungen

### Zugriffskontrolle
- VTY erlaubt standardmäßig nur Zugriff von localhost (127.0.0.1)
- Es wird empfohlen, den Zugriff auf Port 9882 über Firewall-Einstellungen zu beschränken

### Sitzungsüberwachung
- Nicht benötigte Sitzungen ordnungsgemäß beenden
- Lang andauernde inaktive Sitzungen regelmäßig überprüfen

## Fehlerbehebung

### Wenn VTY-Verbindung nicht möglich ist
1. Prüfen Sie, ob sdplane normal gestartet ist
2. Prüfen Sie, ob Port 9882 verfügbar ist
3. Überprüfen Sie die Firewall-Einstellungen

### Wenn Sitzung nicht reagiert
1. Status mit `show worker` aus einer anderen VTY-Sitzung prüfen
2. Bei Bedarf System mit `shutdown` Befehl neu starten

### Wenn Befehle nicht erkannt werden
1. Prüfen Sie, ob Sie in der korrekten Shell (VTY oder Konsole) sind
2. Zeigen Sie mit `?` die Hilfe an, um verfügbare Befehle zu prüfen

## Definitionsort

Diese Befehle sind in den folgenden Dateien definiert:
- `sdplane/vty_shell.c` - VTY-bezogene Befehle
- `sdplane/console_shell.c` - Konsolen-bezogene Befehle

## Verwandte Themen

- [Debug und Logging](debug-logging.md)
- [Systeminformationen und Überwachung](system-monitoring.md)