# DPDK-Anwendungsintegrationsleitfaden

**Sprache / Language:** [English](../dpdk-integration-guide.md) | [日本語](../ja/dpdk-integration-guide.md) | [Français](../fr/dpdk-integration-guide.md) | [中文](../zh/dpdk-integration-guide.md) | **Deutsch** | [Italiano](../it/dpdk-integration-guide.md) | [한국어](../ko/dpdk-integration-guide.md) | [ไทย](../th/dpdk-integration-guide.md) | [Español](../es/dpdk-integration-guide.md)

Dieser Leitfaden erklärt, wie bestehende DPDK-Anwendungen und deren pthread-basierte Worker-Threads in das sdplane-Framework unter Verwendung des DPDK-Dock-Ansatzes integriert werden.

## Übersicht

sdplane bietet ein Integrationsframework für die Ausführung mehrerer DPDK-Anwendungen innerhalb eines einzigen Prozessraums unter Verwendung des **DPDK-Dock-Ansatzes**. Anstatt dass jede DPDK-Anwendung ihre eigene EAL-Umgebung initialisiert, können Anwendungen als Worker-Module innerhalb des kooperativen Threading-Modells von sdplane integriert werden.

Der DPDK-Dock-Ansatz ermöglicht es mehreren DPDK-Anwendungen, Ressourcen effizient zu teilen und zu koexistieren durch:
- Zentralisierte DPDK EAL-Initialisierung
- Bereitstellung gemeinsamer Speicherpool-Verwaltung
- Vereinheitlichte Port- und Queue-Konfiguration
- Kooperatives Multitasking zwischen verschiedenen DPDK-Workloads

## Wichtige Integrationspunkte

### Worker-Thread-Konvertierung
- Konvertierung traditioneller DPDK-pthread-Worker zu sdplane-lcore-Workern
- Ersetzen von `pthread_create()` durch sdplanes `set worker lcore <id> <worker-type>`
- Integration mit sdplanes lcore-basiertem Threading-Modell

### Initialisierungsintegration
- Entfernung anwendungsspezifischer `rte_eal_init()`-Aufrufe
- Verwendung von sdplanes zentralisierter EAL-Initialisierung
- Registrierung anwendungsspezifischer CLI-Befehle über sdplanes Befehlsframework

### Speicher- und Port-Verwaltung
- Nutzung von sdplanes gemeinsamer Speicherpool-Verwaltung
- Verwendung von sdplanes Port-Konfigurations- und Queue-Zuweisungssystem
- Integration mit sdplanes RIB (Routing Information Base) für Port-Status

## Integrationsschritte

### 1. Worker-Funktionen identifizieren
Identifizieren Sie die Haupt-Paketverarbeitungsschleifen in der DPDK-Anwendung. Diese sind typischerweise Funktionen, die:
- In Endlosschleifen laufen, die Pakete verarbeiten
- `rte_eth_rx_burst()` und `rte_eth_tx_burst()` verwenden
- Paketweiterleitung oder -verarbeitungslogik behandeln

### 2. Worker-Module erstellen
Implementieren Sie Worker-Funktionen entsprechend sdplanes Worker-Interface:

```c
static __thread uint64_t loop_counter = 0;

int
my_worker_function(__rte_unused void *dummy)
{
    unsigned lcore_id = rte_lcore_id();
    int thread_id;
    
    // Loop-Counter für Überwachung registrieren
    thread_id = thread_lookup_by_lcore(my_worker_function, lcore_id);
    thread_register_loop_counter(thread_id, &loop_counter);
    
    while (!force_quit && !force_stop[lcore_id]) {
        // Paketverarbeitungslogik hier
        
        // Loop-Counter für Überwachung inkrementieren
        loop_counter++;
    }
    
    return 0;
}
```

### Worker-Loop-Counter-Überwachung

Die `loop_counter`-Variable ermöglicht die Überwachung der Worker-Leistung von der sdplane-Shell aus:

- **Thread-lokale Variable**: Jeder Worker hält seinen eigenen Loop-Counter
- **Registrierung**: Counter wird mit Worker-Name und lcore-ID im Überwachungssystem von sdplane registriert
- **Inkrementierung**: Counter wird bei jeder Iteration der Hauptschleife erhöht
- **Überwachung**: Counter-Werte können über sdplane CLI angezeigt werden, um Worker-Aktivität zu überprüfen

**CLI-Überwachungsbefehle:**
```bash
# Thread-Counter-Informationen einschließlich Loop-Counter anzeigen
show thread counter

# Allgemeine Thread-Informationen anzeigen
show thread

# Worker-Konfiguration und -Status anzeigen
show worker
```

Dies ermöglicht es Administratoren zu bestätigen, dass Worker aktiv verarbeiten und potenzielle Leistungsprobleme oder Worker-Stillstände durch Beobachtung der Loop-Counter-Inkremente zu erkennen.

### 3. RIB-Informationszugriff mit RCU

Für den Zugriff auf Port-Informationen und -Konfigurationen innerhalb von DPDK-Paketverarbeitungs-Workern bietet sdplane RIB (Routing Information Base) Zugriff über RCU (Read-Copy-Update) für thread-sichere Operationen.

#### RIB-Zugriffsmuster

```c
#if HAVE_LIBURCU_QSBR
#include <urcu/urcu-qsbr.h>
#endif /*HAVE_LIBURCU_QSBR*/

static __thread struct rib *rib = NULL;

int
my_worker_function(__rte_unused void *dummy)
{
    unsigned lcore_id = rte_lcore_id();
    int thread_id;
    
    // Loop-Counter für Überwachung registrieren
    thread_id = thread_lookup_by_lcore(my_worker_function, lcore_id);
    thread_register_loop_counter(thread_id, &loop_counter);
    
#if HAVE_LIBURCU_QSBR
    urcu_qsbr_register_thread();
#endif /*HAVE_LIBURCU_QSBR*/

    while (!force_quit && !force_stop[lcore_id]) {
#if HAVE_LIBURCU_QSBR
        urcu_qsbr_read_lock();
        rib = (struct rib *) rcu_dereference(rcu_global_ptr_rib);
#endif /*HAVE_LIBURCU_QSBR*/

        // Paketverarbeitungslogik hier
        // Zugriff auf Port-Informationen über rib->rib_info->port[portid]
        
#if HAVE_LIBURCU_QSBR
        urcu_qsbr_read_unlock();
        urcu_qsbr_quiescent_state();
#endif /*HAVE_LIBURCU_QSBR*/

        loop_counter++;
    }

#if HAVE_LIBURCU_QSBR
    urcu_qsbr_unregister_thread();
#endif /*HAVE_LIBURCU_QSBR*/
    
    return 0;
}
```

#### Zugriff auf Port-Informationen

Nach Erhalt der RIB können Sie auf portspezifische Informationen zugreifen:

```c
// Port-Link-Status überprüfen
if (!rib->rib_info->port[portid].link.link_status) {
    // Port ist down, Verarbeitung überspringen
    continue;
}

// Überprüfen, ob Port gestoppt ist
if (unlikely(rib->rib_info->port[portid].is_stopped)) {
    // Port ist administrativ gestoppt
    continue;
}

// Zugriff auf Port-Konfiguration
struct port_config *port_config = &rib->rib_info->port[portid];

// lcore-Queue-Konfiguration abrufen
struct lcore_qconf *lcore_qconf = &rib->rib_info->lcore_qconf[lcore_id];
for (i = 0; i < lcore_qconf->nrxq; i++) {
    portid = lcore_qconf->rx_queue_list[i].port_id;
    queueid = lcore_qconf->rx_queue_list[i].queue_id;
    // Pakete von diesem Port/Queue verarbeiten
}
```

#### RCU-Sicherheitsrichtlinien

- **Thread-Registrierung**: Immer Thread mit `urcu_qsbr_register_thread()` registrieren
- **Read-Lock**: Read-Lock vor Zugriff auf RIB-Daten akquirieren
- **Dereferenzierung**: `rcu_dereference()` für sicheren Zugriff auf RCU-geschützte Zeiger verwenden
- **Quiescent State**: `urcu_qsbr_quiescent_state()` aufrufen, um sichere Punkte zu kennzeichnen
- **Thread-Cleanup**: Thread mit `urcu_qsbr_unregister_thread()` deregistrieren

#### RIB-Datenstrukturen

Wichtige über RIB verfügbare Informationen:
- **Port-Informationen**: Link-Status, Konfiguration, Statistiken
- **Queue-Konfigurationen**: lcore-zu-Port/Queue-Zuweisungen
- **VLAN-Konfiguration**: Virtual Switch und VLAN-Konfigurationen (für erweiterte Funktionen)
- **Interface-Konfiguration**: TAP-Interface und Routing-Informationen

### 4. CLI-Befehle hinzufügen
Anwendungsspezifische Befehle im CLI-System von sdplane registrieren:

```c
CLI_COMMAND2(my_app_command,
             "my-app command <WORD>",
             "my application\n",
             "command help\n")
{
    // Befehlsimplementierung
    return 0;
}

// In der Initialisierungsfunktion
INSTALL_COMMAND2(cmdset, my_app_command);
```

### 4. Build-System konfigurieren
Build-Konfiguration aktualisieren, um das Modul einzuschließen:

```makefile
# In sdplane/Makefile.am hinzufügen
sdplane_SOURCES += my_worker.c my_worker.h
```

### 5. Integrations-Tests
Funktionalität mit sdplane-Konfiguration verifizieren:

```bash
# Innerhalb der sdplane-Konfiguration
set worker lcore 1 my-worker-type
set thread 1 port 0 queue 0

# Worker starten
start worker lcore all
```

## Integrierte Anwendungen

Die folgenden DPDK-Anwendungen wurden erfolgreich in sdplane mit dem DPDK-Dock-Ansatz integriert:

### L2FWD-Integration (`module/l2fwd/`)
**Status**: ✅ Erfolgreich integriert
- Anpassung der ursprünglichen DPDK l2fwd-Anwendung für sdplane
- Demonstriert grundlegende Paketweiterleitung zwischen Ports
- **Referenz**: Patch-Dateien für ursprünglichen DPDK l2fwd-Quellcode verfügbar
- Demonstriert pthread-zu-lcore-Worker-Konvertierung
- Hauptdateien: `module/l2fwd/l2fwd.c`, Integrationspatches

### PKTGEN-Integration (`module/pktgen/`)
**Status**: ✅ Erfolgreich integriert
- Vollständige DPDK PKTGEN-Anwendungsintegration
- Beispiel für komplexe Multi-Thread-Anwendung
- Externe Bibliotheksintegration mit sdplane CLI
- Demonstriert erweiterte Integrationstechniken
- Hauptdateien: `module/pktgen/app/`, `sdplane/pktgen_cmd.c`

### L3FWD-Integration
**Status**: ⚠️ Teilweise integriert (nicht vollständig funktionsfähig)
- Integration wurde versucht, benötigt aber weitere Arbeit
- Nicht als Referenz für neue Integrationen empfohlen
- Verwenden Sie L2FWD und PKTGEN als primäre Beispiele

## Empfohlene Integrationsreferenzen

Für Entwickler, die neue DPDK-Anwendungen integrieren, wird empfohlen, folgende als primäre Referenzen zu verwenden:

### 1. L2FWD-Integration (Empfohlen)
- **Standort**: `module/l2fwd/`
- **Patch-Dateien**: Für ursprünglichen DPDK l2fwd-Quellcode verfügbar
- **Abdeckung**: Verständnis der grundlegenden pthread-zu-lcore-Konvertierung
- **Integrationsmuster**: Einfacher Paketweiterleitung-Worker

### 2. PKTGEN-Integration (Erweiterte Referenz)
- **Standort**: `module/pktgen/` und `sdplane/pktgen_cmd.c`
- **Abdeckung**: Komplexe Anwendungsintegration mit CLI-Befehlen
- **Integrationsmuster**: Multi-Komponenten-Anwendung mit externen Abhängigkeiten

### Integrationspatches

Für die L2FWD-Integration sind Patch-Dateien verfügbar, die die genauen Modifikationen zeigen, die erforderlich sind, um die ursprüngliche DPDK l2fwd-Anwendung für die sdplane-Integration anzupassen. Diese Patches demonstrieren:
- EAL-Initialisierungsentfernung
- Threading-Modellanpassung
- CLI-Befehlsintegration
- Ressourcenverwaltungsänderungen

Entwickler sollten diese Patch-Dateien für einen systematischen Ansatz zur DPDK-Anwendungsintegration konsultieren.

## Beispiele für benutzerdefinierte Worker

### L2-Repeater (`sdplane/l2_repeater.c`)
Benutzerdefinierter sdplane-Worker (nicht auf bestehender DPDK-App basierend):
- Einfacher Paketweiterleitung-Worker
- Broadcast-Weiterleitung an alle aktiven Ports
- Integration mit sdplanes Port-Management

### Enhanced Repeater (`sdplane/enhanced_repeater.c`)
Benutzerdefinierter sdplane-Worker mit erweiterten Funktionen:
- VLAN-bewusstes Switching
- TAP-Interface-Integration
- RIB-Integration für Port-Status

### VLAN-Switch (`sdplane/vlan_switch.c`)
Benutzerdefinierte Layer-2-Switching-Implementierung:
- MAC-Lernen und -Weiterleitung
- VLAN-Verarbeitung
- Multi-Port-Paketverarbeitung

## Best Practices

### Leistungsüberlegungen
- Burst-Verarbeitung für optimale Leistung verwenden
- Paketkopieren wo möglich minimieren
- DPDKs Zero-Copy-Mechanismen nutzen
- NUMA-Topologie in Worker-Zuweisungen berücksichtigen

### Fehlerbehandlung
- Rückgabewerte von DPDK-Funktionen überprüfen
- Ordnungsgemäße Shutdown-Behandlung implementieren
- Angemessene Logging-Level verwenden
- Ressourcen-Cleanup ordnungsgemäß behandeln

### Threading-Modell
- sdplanes lcore-basiertes Threading verstehen
- Effiziente Paketverarbeitungsschleifen entwerfen
- Angemessene Synchronisationsmechanismen verwenden
- Thread-Affinität und CPU-Isolierung berücksichtigen

## Debugging und Überwachung

### Debug-Logging
Debug-Logging für Worker aktivieren:

```bash
debug sdplane my-worker-type
```

### Statistiksammlung
Mit sdplanes Statistik-Framework integrieren:

```c
// Port-Statistiken aktualisieren
port_statistics[portid].rx += nb_rx;
port_statistics[portid].tx += nb_tx;
```

### CLI-Überwachung
Status-Befehle für Überwachung bereitstellen:

```bash
show my-worker status
show my-worker statistics
```

## Gemeinsame Integrationsmuster

### Paketverarbeitungspipeline
```c
// Typische Paketverarbeitung in sdplane-Worker
while (!force_quit && !force_stop[lcore_id]) {
    // 1. Pakete empfangen
    nb_rx = rte_eth_rx_burst(portid, queueid, pkts_burst, MAX_PKT_BURST);
    
    // 2. Pakete verarbeiten
    for (i = 0; i < nb_rx; i++) {
        // Verarbeitungslogik
        process_packet(pkts_burst[i]);
    }
    
    // 3. Pakete senden
    rte_eth_tx_burst(dst_port, queueid, pkts_burst, nb_rx);
}
```

### Konfigurationsintegration
```c
// Bei sdplanes Konfigurationssystem registrieren
struct worker_config my_worker_config = {
    .name = "my-worker",
    .worker_func = my_worker_function,
    .init_func = my_worker_init,
    .cleanup_func = my_worker_cleanup
};
```

## Fehlerbehebung

### Häufige Probleme
- **Worker startet nicht**: lcore-Zuweisungen und Konfiguration überprüfen
- **Paket-Drops**: Queue-Konfiguration und Puffergrößen überprüfen
- **Leistungsprobleme**: CPU-Affinität und NUMA-Konfiguration überprüfen
- **CLI-Befehle funktionieren nicht**: Ordnungsgemäße Befehlsregistrierung überprüfen

### Debug-Techniken
- sdplanes Debug-Logging-System verwenden
- Worker-Statistiken und Counter überwachen
- Port-Link-Status und Konfiguration überprüfen
- Speicherpool-Zuweisungen verifizieren

Für detaillierte Implementierungsbeispiele siehe die bestehenden Worker-Module in der Codebasis und ihre entsprechenden CLI-Befehlsdefinitionen.