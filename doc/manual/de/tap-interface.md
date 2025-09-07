# TAP-Interface

**Sprache / Language:** [English](../en/tap-interface.md) | [日本語](../ja/tap-interface.md) | [Français](../fr/tap-interface.md) | [中文](../zh/tap-interface.md) | **Deutsch** | [Italiano](../it/tap-interface.md) | [한국어](../ko/tap-interface.md) | [ไทย](../th/tap-interface.md) | [Español](../es/tap-interface.md)

Befehle zur Verwaltung von TAP-Interfaces.

## Befehlsübersicht

### set_tap_capture_ifname - TAP-Capture-Interface-Name konfigurieren
```
set tap capture ifname <WORD>
```

Konfiguriert den Interface-Namen für die TAP-Capture-Funktion.

**Parameter:**
- `<WORD>` - Interface-Name

**Verwendungsbeispiel:**
```bash
# tap0-Interface konfigurieren
set tap capture ifname tap0

# tap1-Interface konfigurieren
set tap capture ifname tap1
```

### set_tap_capture_persistent - TAP-Capture-Persistierung aktivieren
```
set tap capture persistent
```

Aktiviert die Persistierung von TAP-Capture.

**Verwendungsbeispiel:**
```bash
# Persistierung aktivieren
set tap capture persistent
```

### no_tap_capture_persistent - TAP-Capture-Persistierung deaktivieren
```
no tap capture persistent
```

Deaktiviert die Persistierung von TAP-Capture.

**Verwendungsbeispiel:**
```bash
# Persistierung deaktivieren
no tap capture persistent
```

### unset_tap_capture_persistent - TAP-Capture-Persistierung-Konfiguration entfernen
```
unset tap capture persistent
```

Entfernt die TAP-Capture-Persistierung-Konfiguration.

**Verwendungsbeispiel:**
```bash
# Persistierung-Konfiguration entfernen
unset tap capture persistent
```

## Übersicht der TAP-Interfaces

### Was ist ein TAP-Interface?
Ein TAP (Network TAP) Interface ist ein virtuelles Netzwerk-Interface zur Überwachung und zum Testen von Netzwerkverkehr.

### Hauptfunktionen
- **Paket-Capture** - Erfassen von Netzwerkverkehr
- **Paket-Injection** - Einspeisung von Test-Paketen
- **Bridge-Funktion** - Bridge zwischen verschiedenen Netzwerken
- **Überwachungsfunktion** - Verkehrsüberwachung und -analyse

### Verwendung in sdplane
- **Debug** - Debugging von Paketflüssen
- **Test** - Testen von Netzwerkfunktionen
- **Überwachung** - Verkehrsüberwachung
- **Entwicklung** - Entwicklung und Test neuer Funktionen

## TAP-Interface-Konfiguration

### Grundlegende Konfigurationsschritte
1. **TAP-Interface erstellen**
```bash
# TAP-Interface auf Systemebene erstellen
sudo ip tuntap add tap0 mode tap
sudo ip link set tap0 up
```

2. **Konfiguration in sdplane**
```bash
# TAP-Capture-Interface-Namen konfigurieren
set tap capture ifname tap0

# Persistierung aktivieren
set tap capture persistent
```

3. **TAP-Handler-Worker-Konfiguration**
```bash
# TAP-Handler-Worker konfigurieren
set worker lcore 2 tap-handler
start worker lcore 2
```

### Konfigurationsbeispiele

#### Grundlegende TAP-Konfiguration
```bash
# TAP-Interface-Konfiguration
set tap capture ifname tap0
set tap capture persistent

# Worker-Konfiguration
set worker lcore 2 tap-handler
start worker lcore 2

# Konfiguration überprüfen
show worker
```

#### Mehrere TAP-Interface-Konfiguration
```bash
# Mehrere TAP-Interfaces konfigurieren
set tap capture ifname tap0
set tap capture ifname tap1

# Persistierung aktivieren
set tap capture persistent
```

## Persistierung-Funktion

### Was ist Persistierung?
Wenn die Persistierung-Funktion aktiviert ist, bleibt die TAP-Interface-Konfiguration auch nach einem Systemneustart erhalten.

### Vorteile der Persistierung
- **Konfiguration beibehalten** - Konfiguration bleibt nach Neustart wirksam
- **Automatische Wiederherstellung** - Automatische Wiederherstellung nach Systemfehlern
- **Betriebseffizienz** - Reduzierung manueller Konfiguration

### Persistierung-Konfiguration
```bash
# Persistierung aktivieren
set tap capture persistent

# Persistierung deaktivieren
no tap capture persistent

# Persistierung-Konfiguration entfernen
unset tap capture persistent
```

## Verwendungsbeispiele

### Debug-Verwendung
```bash
# Debug-TAP-Interface-Konfiguration
set tap capture ifname debug-tap
set tap capture persistent

# TAP-Handler-Worker konfigurieren
set worker lcore 3 tap-handler
start worker lcore 3

# Paket-Capture starten
tcpdump -i debug-tap
```

### Test-Verwendung
```bash
# Test-TAP-Interface-Konfiguration
set tap capture ifname test-tap
set tap capture persistent

# Test-Paket-Injection vorbereiten
set worker lcore 4 tap-handler
start worker lcore 4
```

## Überwachung und Verwaltung

### TAP-Interface-Status überprüfen
```bash
# Worker-Status überprüfen
show worker

# Thread-Informationen überprüfen
show thread

# Systemebenen-Überprüfung
ip link show tap0
```

### Verkehrsüberwachung
```bash
# Überwachung mit tcpdump
tcpdump -i tap0

# Überwachung mit Wireshark
wireshark -i tap0
```

## Fehlerbehebung

### Wenn TAP-Interface nicht erstellt wird
1. Systemebenen-Überprüfung
```bash
# TAP-Interface-Existenz überprüfen
ip link show tap0

# Berechtigung überprüfen
sudo ip tuntap add tap0 mode tap
```

2. sdplane-Überprüfung
```bash
# Konfiguration überprüfen
show worker

# Worker-Status überprüfen
show thread
```

### Wenn Pakete nicht erfasst werden
1. Interface-Status überprüfen
```bash
ip link show tap0
```

2. Worker-Status überprüfen
```bash
show worker
```

3. TAP-Handler neu starten
```bash
restart worker lcore 2
```

### Wenn Persistierung nicht funktioniert
1. Persistierung-Konfiguration überprüfen
```bash
# Aktuelle Konfiguration überprüfen (mit show-Befehlen überprüfen)
show worker
```

2. Systemkonfiguration überprüfen
```bash
# Systemebenen-Konfiguration überprüfen
systemctl status sdplane
```

## Erweiterte Funktionen

### Integration mit VLAN
TAP-Interfaces können in Zusammenarbeit mit VLAN-Funktionen verwendet werden:
```bash
# Integration mit VLAN-Switch-Worker
set worker lcore 5 vlan-switch
start worker lcore 5
```

### Bridge-Funktion
Verwendung mehrerer TAP-Interfaces als Bridge:
```bash
# Mehrere TAP-Interface-Konfigurationen
set tap capture ifname tap0
set tap capture ifname tap1
```

## Definitionsort

Diese Befehle sind in den folgenden Dateien definiert:
- `sdplane/tap_cmd.c`

## Verwandte Themen

- [Worker- und lcore-Management](worker-lcore-thread-management.md)
- [VTY- und Shell-Management](vty-shell.md)
- [Debug und Logging](debug-logging.md)