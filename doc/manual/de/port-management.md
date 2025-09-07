# Port-Management und Statistiken

**Sprache / Language:** [English](../en/port-management.md) | [日本語](../ja/port-management.md) | [Français](../fr/port-management.md) | [中文](../zh/port-management.md) | **Deutsch** | [Italiano](../it/port-management.md) | [한국어](../ko/port-management.md) | [ไทย](../th/port-management.md) | [Español](../es/port-management.md)

Befehle zur Verwaltung von DPDK-Ports und Statistikinformationen.

## Befehlsübersicht

### **start port**

Startet DPDK-Ports (Standardverhalten).

**Verwendungsbeispiel:**
```bash
# Port starten (Standard)
start port
```

---

### **start port \<0-16\>**

Startet einen spezifischen DPDK-Port.

**Verwendungsbeispiel:**
```bash
# Port 0 starten
start port 0

# Port 1 starten
start port 1
```

---

### **start port all**

Startet alle DPDK-Ports.

**Verwendungsbeispiel:**
```bash
# Alle Ports starten
start port all
```

---

### **stop port**

Stoppt DPDK-Ports (Standardverhalten).

**Verwendungsbeispiel:**
```bash
# Port stoppen (Standard)
stop port
```

---

### **stop port \<0-16\>**

Stoppt einen spezifischen DPDK-Port.

**Verwendungsbeispiel:**
```bash
# Port 0 stoppen
stop port 0

# Port 1 stoppen
stop port 1
```

---

### **stop port all**

Stoppt alle DPDK-Ports.

**Verwendungsbeispiel:**
```bash
# Alle Ports stoppen
stop port all
```

---

### **reset port**

Setzt DPDK-Ports zurück (Standardverhalten).

**Verwendungsbeispiel:**
```bash
# Port zurücksetzen (Standard)
reset port
```

---

### **reset port \<0-16\>**

Setzt einen spezifischen DPDK-Port zurück.

**Verwendungsbeispiel:**
```bash
# Port 0 zurücksetzen
reset port 0

# Port 1 zurücksetzen
reset port 1
```

---

### **reset port all**

Setzt alle DPDK-Ports zurück.

**Verwendungsbeispiel:**
```bash
# Alle Ports zurücksetzen
reset port all
```

---

### **show port**

Zeigt die Grundinformationen aller Ports an (Standardverhalten).

**Verwendungsbeispiel:**
```bash
# Informationen aller Ports anzeigen
show port
```

---

### **show port \<0-16\>**

Zeigt die Grundinformationen eines spezifischen Ports an.

**Verwendungsbeispiel:**
```bash
# Informationen von Port 0 anzeigen
show port 0

# Informationen von Port 1 anzeigen
show port 1
```

---

### **show port all**

Zeigt explizit die Informationen aller Ports an.

**Verwendungsbeispiel:**
```bash
# Informationen aller Ports explizit anzeigen
show port all
```

---

### **show port statistics**

Zeigt alle Port-Statistikinformationen an.

**Verwendungsbeispiel:**
```bash
# Alle Statistikinformationen anzeigen
show port statistics
```

---

### **show port statistics pps**

Zeigt Pakete/Sekunde-Statistiken an.

**Verwendungsbeispiel:**
```bash
# Nur PPS-Statistiken anzeigen
show port statistics pps
```

---

### **show port statistics total**

Zeigt die Gesamtzahl der Pakete an.

**Verwendungsbeispiel:**
```bash
# Gesamtzahl der Pakete anzeigen
show port statistics total
```

---

### **show port statistics bps**

Zeigt Bits/Sekunde-Statistiken an.

**Verwendungsbeispiel:**
```bash
# Bits/Sekunde anzeigen
show port statistics bps
```

---

### **show port statistics Bps**

Zeigt Bytes/Sekunde-Statistiken an.

**Verwendungsbeispiel:**
```bash
# Bytes/Sekunde anzeigen
show port statistics Bps
```

---

### **show port statistics total-bytes**

Zeigt die Gesamtzahl der Bytes an.

**Verwendungsbeispiel:**
```bash
# Gesamtzahl der Bytes anzeigen
show port statistics total-bytes
```

---

### **show port \<0-16\> promiscuous**

Zeigt den Status des Promiskuitätsmodus für den angegebenen Port an.

**Verwendungsbeispiel:**
```bash
# Promiskuitätsmodus von Port 0 anzeigen
show port 0 promiscuous

# Promiskuitätsmodus von Port 1 anzeigen
show port 1 promiscuous
```

---

### **show port all promiscuous**

Zeigt den Status des Promiskuitätsmodus für alle Ports an.

**Verwendungsbeispiel:**
```bash
# Promiskuitätsmodus aller Ports anzeigen
show port all promiscuous
```

---

### **show port \<0-16\> flowcontrol**

Zeigt die Flusskontrolleinstellungen für den angegebenen Port an.

**Verwendungsbeispiel:**
```bash
# Flusskontrolleinstellungen von Port 0 anzeigen
show port 0 flowcontrol

# Flusskontrolleinstellungen von Port 1 anzeigen
show port 1 flowcontrol
```

---

### **show port all flowcontrol**

Zeigt die Flusskontrolleinstellungen für alle Ports an.

**Verwendungsbeispiel:**
```bash
# Flusskontrolleinstellungen aller Ports anzeigen
show port all flowcontrol
```

---

### **set port \<0-16\> promiscuous enable**

Aktiviert den Promiskuitätsmodus für den angegebenen Port.

**Verwendungsbeispiel:**
```bash
# Promiskuitätsmodus für Port 0 aktivieren
set port 0 promiscuous enable

# Promiskuitätsmodus für Port 1 aktivieren
set port 1 promiscuous enable
```

---

### **set port \<0-16\> promiscuous disable**

Deaktiviert den Promiskuitätsmodus für den angegebenen Port.

**Verwendungsbeispiel:**
```bash
# Promiskuitätsmodus für Port 0 deaktivieren
set port 0 promiscuous disable

# Promiskuitätsmodus für Port 1 deaktivieren
set port 1 promiscuous disable
```

---

### **set port all promiscuous enable**

Aktiviert den Promiskuitätsmodus für alle Ports.

**Verwendungsbeispiel:**
```bash
# Promiskuitätsmodus für alle Ports aktivieren
set port all promiscuous enable
```

---

### **set port all promiscuous disable**

Deaktiviert den Promiskuitätsmodus für alle Ports.

**Verwendungsbeispiel:**
```bash
# Promiskuitätsmodus für alle Ports deaktivieren
set port all promiscuous disable
```

---

### **set port \<0-16\> flowcontrol (rx|tx|autoneg|send-xon|fwd-mac-ctrl) (on|off)**

Ändert die Flusskontrolleinstellungen für den angegebenen Port.

**Optionen:**
- `rx` - Empfangsflusskontrolle
- `tx` - Sendeflusskontrolle
- `autoneg` - Automatische Verhandlung
- `send-xon` - XON-Sendung
- `fwd-mac-ctrl` - MAC-Kontrollrahmen-Weiterleitung

**Verwendungsbeispiel:**
```bash
# Empfangsflusskontrolle für Port 0 aktivieren
set port 0 flowcontrol rx on

# Automatische Verhandlung für Port 1 deaktivieren
set port 1 flowcontrol autoneg off
```

---

### **set port all flowcontrol (rx|tx|autoneg|send-xon|fwd-mac-ctrl) (on|off)**

Ändert die Flusskontrolleinstellungen für alle Ports.

**Optionen:**
- `rx` - Empfangsflusskontrolle
- `tx` - Sendeflusskontrolle
- `autoneg` - Automatische Verhandlung
- `send-xon` - XON-Sendung
- `fwd-mac-ctrl` - MAC-Kontrollrahmen-Weiterleitung

**Verwendungsbeispiel:**
```bash
# Automatische Verhandlung für alle Ports deaktivieren
set port all flowcontrol autoneg off

# Sendeflusskontrolle für alle Ports aktivieren
set port all flowcontrol tx on
```

---

### **set port \<0-16\> dev-configure \<0-64\> \<0-64\>**


Konfiguriert das DPDK-Gerät für den angegebenen Port.

**Parameter:**
- 1. Argument: Anzahl der Empfangsqueues (0-64)
- 2. Argument: Anzahl der Sendequeues (0-64)

**Verwendungsbeispiel:**
```bash
# Port 0 mit 4 Empfangs- und 4 Sendequeues konfigurieren
set port 0 dev-configure 4 4

# Port 1 mit 2 Empfangs- und 2 Sendequeues konfigurieren
set port 1 dev-configure 2 2
```

---

### **set port all dev-configure \<0-64\> \<0-64\>**

Konfiguriert das DPDK-Gerät für alle Ports.

**Parameter:**
- 1. Argument: Anzahl der Empfangsqueues (0-64)
- 2. Argument: Anzahl der Sendequeues (0-64)

**Verwendungsbeispiel:**
```bash
# Alle Ports mit 1 Empfangs- und 1 Sendequeue konfigurieren
set port all dev-configure 1 1
```

---

### **set port \<0-16\> nrxdesc \<0-16384\>**


Legt die Anzahl der Empfangsdeskriptoren für den angegebenen Port fest.

**Verwendungsbeispiel:**
```bash
# Anzahl der Empfangsdeskriptoren für Port 0 auf 1024 setzen
set port 0 nrxdesc 1024

# Anzahl der Empfangsdeskriptoren für Port 1 auf 512 setzen
set port 1 nrxdesc 512
```

---

### **set port all nrxdesc \<0-16384\>**

Legt die Anzahl der Empfangsdeskriptoren für alle Ports fest.

**Verwendungsbeispiel:**
```bash
# Anzahl der Empfangsdeskriptoren für alle Ports auf 512 setzen
set port all nrxdesc 512
```

---

### **set port \<0-16\> ntxdesc \<0-16384\>**


Legt die Anzahl der Sendedeskriptoren für den angegebenen Port fest.

**Verwendungsbeispiel:**
```bash
# Anzahl der Sendedeskriptoren für Port 0 auf 1024 setzen
set port 0 ntxdesc 1024

# Anzahl der Sendedeskriptoren für Port 1 auf 512 setzen
set port 1 ntxdesc 512
```

---

### **set port all ntxdesc \<0-16384\>**

Legt die Anzahl der Sendedeskriptoren für alle Ports fest.

**Verwendungsbeispiel:**
```bash
# Anzahl der Sendedeskriptoren für alle Ports auf 512 setzen
set port all ntxdesc 512
```

---

### **set port \<0-16\> link up**


Aktiviert die Verbindung für den angegebenen Port.

**Verwendungsbeispiel:**
```bash
# Verbindung für Port 0 aktivieren
set port 0 link up

# Verbindung für Port 1 aktivieren
set port 1 link up
```

---

### **set port \<0-16\> link down**

Deaktiviert die Verbindung für den angegebenen Port.

**Verwendungsbeispiel:**
```bash
# Verbindung für Port 0 deaktivieren
set port 0 link down

# Verbindung für Port 1 deaktivieren
set port 1 link down
```

---

### **set port all link up**

Aktiviert die Verbindung für alle Ports.

**Verwendungsbeispiel:**
```bash
# Verbindung für alle Ports aktivieren
set port all link up
```

---

### **set port all link down**

Deaktiviert die Verbindung für alle Ports.

**Verwendungsbeispiel:**
```bash
# Verbindung für alle Ports deaktivieren
set port all link down
```

## Definitionsort

Diese Befehle sind in den folgenden Dateien definiert:
- `sdplane/dpdk_port_cmd.c`

## Verwandte Themen

- [Worker- und lcore-Management](worker-lcore-thread-management.md)
- [Systeminformationen und Überwachung](system-monitoring.md)
- [Queue-Konfiguration](queue-configuration.md)