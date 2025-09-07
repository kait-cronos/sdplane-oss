# Applicazione Ripetitore L2

**Lingua / Language:** [English](../en/l2-repeater-application.md) | [日本語](../ja/l2-repeater-application.md) | [Français](../fr/l2-repeater-application.md) | [中文](../zh/l2-repeater-application.md) | [Deutsch](../de/l2-repeater-application.md) | **Italiano** | [한국어](../ko/l2-repeater-application.md) | [ไทย](../th/l2-repeater-application.md) | [Español](../es/l2-repeater-application.md)

L'applicazione Ripetitore L2 fornisce un trasferimento pacchetti Layer 2 semplice tra porte DPDK con funzionalità di ripetizione base tra porte.

## Panoramica

Il Ripetitore L2 è un'applicazione intuitiva di forwarding Layer 2 con le seguenti caratteristiche:
- Trasferimento pacchetti tra porte DPDK abbinate (semplice ripetizione tra porte)
- Funzionalità di ripetizione pacchetti base senza apprendimento indirizzi MAC
- Funzionalità opzionale di aggiornamento indirizzi MAC (modifica indirizzo MAC sorgente)
- Funzionamento ad alte prestazioni con elaborazione pacchetti zero-copy di DPDK

## Caratteristiche Principali

### Forwarding Layer 2
- **Ripetizione tra Porte**: Trasferimento pacchetti semplice tra coppie di porte preconfigurate
- **Nessun Apprendimento MAC**: Ripetizione pacchetti diretta senza costruire tabelle di forwarding
- **Forwarding Trasparente**: Inoltra tutti i pacchetti indipendentemente dal MAC di destinazione
- **Accoppiamento Porte**: Configurazione di trasferimento fissa porta-a-porta

### Caratteristiche Prestazioni
- **Elaborazione Zero-Copy**: Utilizzo dell'elaborazione pacchetti efficiente di DPDK
- **Elaborazione Burst**: Elaborazione pacchetti a burst per throughput ottimale
- **Bassa Latenza**: Overhead di elaborazione minimo per forwarding ad alta velocità
- **Supporto Multi-Core**: Esecuzione su lcore dedicati per scalabilità

## Configurazione

### Configurazione Base
Il Ripetitore L2 viene configurato attraverso il sistema di configurazione principale sdplane:

```bash
# Imposta tipo worker a ripetitore L2
set worker lcore 1 l2-repeater

# Configurazione porte e code
set thread 1 port 0 queue 0  
set thread 1 port 1 queue 0

# Abilita modalità promiscua per l'apprendimento
set port all promiscuous enable
```

### Esempio File di Configurazione
Per un esempio di configurazione completo, vedere [`example-config/sdplane_l2_repeater.conf`](../../example-config/sdplane_l2_repeater.conf):

```bash
# Binding dispositivi
set device 02:00.0 driver vfio-pci bind
set device 03:00.0 driver vfio-pci bind

# Inizializzazione DPDK
set rte_eal argv -c 0x7
rte_eal_init

# Worker background (devono essere avviati prima della configurazione code)
set worker lthread stat-collector
set worker lthread rib-manager
set worker lthread netlink-thread

set mempool

# Configurazione code (deve essere eseguita dopo l'avvio di rib-manager)
set thread 1 port 0 queue 0
set thread 1 port 1 queue 0

# Configurazione porte
stop port all
set port all dev-configure 1 4
set port all nrxdesc 1024
set port all ntxdesc 1024

# Configurazione worker
set worker lcore 1 l2-repeater
set worker lcore 2 tap-handler
set port all promiscuous enable
start port all

# Aspetta che le porte si avviino
sleep 5

# Avvia worker
start worker lcore all
```

## Funzionamento

### Forwarding Broadcast
Il Ripetitore L2 inoltra i pacchetti a tutte le altre porte attive:
- **Comportamento Broadcast**: Inoltra tutti i pacchetti ricevuti su una porta a tutte le altre porte attive
- **Split Horizon**: Non reinvia pacchetti alla porta di ricezione
- **Nessun Filtraggio**: Inoltra tutti i tipi di pacchetti (unicast, broadcast, multicast)

### Comportamento Forwarding
- **Forwarding Tutto il Traffico**: Ripete tutti i pacchetti indipendentemente dal MAC di destinazione
- **Broadcast Tutte le Porte**: Inoltra pacchetti a tutte le porte attive eccetto quella di input
- **Trasparente**: Nessuna modifica del contenuto pacchetto (a meno che l'aggiornamento MAC non sia abilitato)
- **Duplicazione Multi-Porta**: Crea copie del pacchetto per ogni porta di destinazione

### Aggiornamento Indirizzi MAC
Quando abilitato, il Ripetitore L2 può modificare gli indirizzi MAC dei pacchetti:
- **Aggiornamento MAC Sorgente**: Cambia il MAC sorgente con il MAC della porta di output
- **Bridging Trasparente**: Mantiene gli indirizzi MAC originali (predefinito)

## Tuning Prestazioni

### Configurazione Buffer
```bash
# Ottimizza il numero di descrittori per il carico di lavoro
set port all nrxdesc 2048  # Aumenta per alti packet rate
set port all ntxdesc 2048  # Aumenta per buffering
```

### Assegnazione Worker
```bash
# Dedica lcore specifici per forwarding L2
set worker lcore 1 l2-repeater  # Assegna a core dedicato
set worker lcore 2 tap-handler  # Separa elaborazione TAP
```

### Dimensionamento Memory Pool
I memory pool devono essere dimensionati appropriatamente per il traffico previsto:
- Considera packet rate e requisiti di buffering
- Considera burst size e storage temporaneo pacchetti

## Monitoraggio e Debug

### Statistiche Porte
```bash
# Visualizza statistiche forwarding
show port statistics all

# Monitora porte specifiche
show port statistics 0
show port statistics 1
```

### Comandi Debug
```bash
# Abilita debug ripetitore L2
debug sdplane l2-repeater

# Debug VLAN switch (tipo worker alternativo)
debug sdplane vlan-switch

# Debug sdplane generale
debug sdplane fdb-change
debug sdplane rib
```

## Casi d'Uso

### Ripetitore Tipo Hub
- Duplica traffico a tutte le porte connesse
- Ripetizione Layer 2 trasparente
- Funzionalità hub base senza apprendimento

### Mirroring/Ripetizione Porte
- Mirroring traffico tra porte
- Monitoraggio e analisi rete
- Duplicazione pacchetti semplice

### Test Prestazioni
- Misura prestazioni forwarding
- Baseline per benchmark forwarding L2
- Validazione configurazioni porte DPDK

## Limitazioni

- **Nessuna Elaborazione VLAN**: Ripetizione L2 semplice senza riconoscimento VLAN
- **Nessun Apprendimento MAC**: Forwarding broadcast tutte le porte senza apprendimento indirizzi
- **Nessun Supporto STP**: Nessuna implementazione Spanning Tree Protocol
- **Nessun Filtraggio**: Inoltra tutti i pacchetti indipendentemente dalla destinazione

## Applicazioni Correlate

- **Ripetitore Avanzato**: Versione avanzata con supporto VLAN e interfacce TAP
- **Forwarding L3**: Funzionalità routing Layer 3
- **Switch VLAN**: Funzionalità switching con supporto VLAN

Per funzionalità Layer 2 più avanzate incluso il supporto VLAN, vedere la documentazione del [Ripetitore Avanzato](enhanced-repeater.md).