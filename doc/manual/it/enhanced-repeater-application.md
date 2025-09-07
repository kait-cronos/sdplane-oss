# Applicazione Ripetitore Avanzato

**Lingua / Language:** [English](../en/enhanced-repeater-application.md) | [日本語](../ja/enhanced-repeater-application.md) | [Français](../fr/enhanced-repeater-application.md) | [中文](../zh/enhanced-repeater-application.md) | [Deutsch](../de/enhanced-repeater-application.md) | **Italiano** | [한국어](../ko/enhanced-repeater-application.md) | [ไทย](../th/enhanced-repeater-application.md) | [Español](../es/enhanced-repeater-application.md)

L'applicazione Ripetitore Avanzato fornisce switching Layer 2 avanzato con supporto VLAN, con integrazione di interfacce TAP per funzionalità di routing L3 e cattura pacchetti.

## Panoramica

Il Ripetitore Avanzato è un'applicazione sofisticata di forwarding Layer 2 che estende il forwarding L2 base con le seguenti caratteristiche:
- **Switching con Supporto VLAN**: Supporto completo IEEE 802.1Q VLAN (tagging/untagging)
- **Astrazione Switch Virtuale**: Switch virtuali multipli con tabelle di forwarding indipendenti
- **Integrazione Interfacce TAP**: Interfacce router e di cattura per integrazione kernel
- **Elaborazione VLAN Avanzata**: Funzionalità di traduzione, inserimento e rimozione VLAN

## Architettura

### Framework Switch Virtuale
Il Ripetitore Avanzato implementa un'architettura di switch virtuali:
- **VLAN Multiple**: Supporto per domini VLAN multipli (1-4094)
- **Aggregazione Porte**: Porte fisiche multiple per switch virtuale
- **Forwarding Isolato**: Domini di forwarding indipendenti per VLAN
- **Tagging Flessibile**: Modalità native, tagged e di traduzione per porta

### Integrazione Interfacce TAP
- **Interfacce Router**: Integrazione stack di rete kernel per elaborazione L3
- **Interfacce Cattura**: Funzionalità di monitoraggio e analisi pacchetti
- **Buffer Ring**: Trasferimento efficiente pacchetti tra data plane e kernel
- **Bidirezionale**: Elaborazione pacchetti in entrambe le direzioni

## Caratteristiche Principali

### Elaborazione VLAN
- **Tagging VLAN**: Aggiunta header 802.1Q a frame untagged
- **Untagging VLAN**: Rimozione header 802.1Q da frame tagged
- **Traduzione VLAN**: Modifica VLAN ID tra input e output
- **VLAN Nativa**: Gestione traffico untagged su porte trunk

### Switching Virtuale
- **Apprendimento**: Apprendimento automatico indirizzi MAC per VLAN
- **Flooding**: Gestione appropriata unicast sconosciuti e broadcast
- **Split Horizon**: Prevenzione loop all'interno degli switch virtuali
- **Multi-Dominio**: Tabelle di forwarding indipendenti per VLAN

### Elaborazione Pacchetti
- **Zero-Copy**: Elaborazione pacchetti DPDK efficiente con overhead minimo
- **Elaborazione Burst**: Ottimizzata per alti packet rate
- **Manipolazione Header**: Inserimento/rimozione efficiente header VLAN
- **Ottimizzazione Copia**: Copia selettiva pacchetti per interfacce TAP

## Configurazione

### Configurazione Switch Virtuale
Creazione di switch virtuali per diversi domini VLAN:

```bash
# Crea switch virtuali con VLAN ID
set vswitch 2031
set vswitch 2032
```

### Collegamento Porte a Switch Virtuali
Collegamento porte fisiche DPDK a switch virtuali:

```bash
# Collega porta 0 a switch virtuale 0 con tag VLAN 2031
set vswitch-link vswitch 0 port 0 tag 2031

# Collega porta 0 a switch virtuale 1 come nativo/untagged
set vswitch-link vswitch 1 port 0 tag 0

# Esempio traduzione VLAN
set vswitch-link vswitch 0 port 1 tag 2040
```

### Interfacce Router
Creazione interfacce router per connettività L3:

```bash
# Crea interfacce router per elaborazione L3
set vswitch 2031 router-if rif2031
set vswitch 2032 router-if rif2032
```

### Interfacce Cattura
Configurazione interfacce cattura per monitoraggio:

```bash
# Crea interfacce cattura per monitoraggio pacchetti
set vswitch 2031 capture-if cif2031
set vswitch 2032 capture-if cif2032
```

### Configurazione Worker
Assegnazione ripetitore avanzato a lcore:

```bash
# Configura worker ripetitore avanzato
set worker lcore 1 enhanced-repeater

# Configura assegnazione code
set thread 1 port 0 queue 0
```

## Esempi di Configurazione

### Configurazione Completa
Vedere [`example-config/sdplane_enhanced_repeater.conf`](../../example-config/sdplane_enhanced_repeater.conf):

```bash
# Configurazione dispositivi
set device 03:00.0 driver vfio-pci bind

# Inizializzazione DPDK
set rte_eal argv -c 0x7
rte_eal_init
set mempool

# Configurazione porte
stop port all
set port all dev-configure 1 4
set port all nrxdesc 1024
set port all ntxdesc 1024

# Configurazione worker
set worker lthread stat-collector
set worker lthread rib-manager
set worker lcore 1 enhanced-repeater
set worker lcore 2 l3-tap-handler

# Abilita modalità promiscua
set port all promiscuous enable
start port all

# Configurazione code
set thread 1 port 0 queue 0

# Configurazione switch virtuali
set vswitch 2031
set vswitch 2032

# Configurazione link
set vswitch-link vswitch 0 port 0 tag 2031
set vswitch-link vswitch 1 port 0 tag 2032

# Creazione interfacce
set vswitch 2031 router-if rif2031
set vswitch 2032 router-if rif2032
set vswitch 2031 capture-if cif2031
set vswitch 2032 capture-if cif2032

# Avvio worker
start worker lcore all
```

## Dettagli Elaborazione VLAN

### Modalità Tagging

#### Modalità Tagged
- **Input**: Accetta pacchetti con tag VLAN specifico
- **Output**: Mantiene o traduce tag VLAN
- **Utilizzo**: Porte trunk, traduzione VLAN

#### Modalità Nativa (tag 0)
- **Input**: Accetta pacchetti untagged
- **Output**: Rimuove header VLAN
- **Utilizzo**: Porte access, dispositivi non-VLAN aware

#### Modalità Traduzione
- **Funzionalità**: Modifica VLAN ID tra porte
- **Configurazione**: Tag diversi su link input/output
- **Utilizzo**: Mappatura VLAN, reti service provider

### Flusso Pacchetti

1. **Elaborazione Input**:
   - Ricezione pacchetti su porte DPDK
   - Determinazione VLAN basata su configurazione tag o nativa
   - Ricerca switch virtuale destinazione

2. **Ricerca Switch Virtuale**:
   - Apprendimento e ricerca indirizzi MAC
   - Determinazione porta output all'interno dominio VLAN
   - Gestione flooding unicast sconosciuti/broadcast

3. **Elaborazione Output**:
   - Manipolazione header VLAN per configurazione porta
   - Copia pacchetti per destinazioni multiple
   - Integrazione interfacce TAP

4. **Elaborazione Interfacce TAP**:
   - Interfacce Router: Elaborazione L3 kernel
   - Interfacce Cattura: Monitoraggio e analisi

## Monitoraggio e Gestione

### Comandi Stato
```bash
# Visualizza configurazione switch virtuali
show vswitch_rib

# Visualizza link switch virtuali
show vswitch-link

# Visualizza interfacce router
show rib vswitch router-if

# Verifica interfacce cattura
show rib vswitch capture-if
```

### Statistiche e Prestazioni
```bash
# Statistiche livello porta
show port statistics all

# Prestazioni worker
show worker statistics

# Informazioni thread
show thread information
```

### Comandi Debug
```bash
# Debug ripetitore avanzato
debug sdplane enhanced-repeater

# Debug switch VLAN (tipo worker alternativo)
debug sdplane vlan-switch

# Debug RIB e forwarding
debug sdplane rib
debug sdplane fdb-change
```

## Casi d'Uso

### Aggregazione VLAN
- Consolidamento VLAN multiple su singolo link fisico
- Funzionalità provider edge
- Multiplexing servizi

### Integrazione L3
- Configurazioni router-on-a-stick
- Routing inter-VLAN tramite interfacce TAP
- Forwarding ibrido L2/L3

### Monitoraggio Rete
- Cattura pacchetti per VLAN
- Analisi traffico e debug
- Monitoraggio servizi

### Reti Service Provider
- Traduzione VLAN per isolamento clienti
- Networking multi-tenant
- Traffic engineering

## Considerazioni Prestazioni

### Scalabilità
- **Assegnazione Worker**: Dedicare lcore per prestazioni ottimali
- **Configurazione Code**: Bilanciare assegnazione code tra core
- **Memory Pool**: Dimensionamento appropriato per packet rate e necessità buffering

### Overhead Elaborazione VLAN
- **Manipolazione Header**: Overhead minimo per operazioni VLAN
- **Copia Pacchetti**: Copia selettiva solo quando necessario
- **Ottimizzazione Burst**: Elaborazione pacchetti in unità burst per efficienza

### Prestazioni Interfacce TAP
- **Dimensionamento Ring Buffer**: Configurazione dimensioni ring appropriate
- **Forwarding Selettivo**: Invio solo pacchetti rilevanti a interfacce TAP
- **Integrazione Kernel**: Considerazione overhead elaborazione kernel

## Risoluzione Problemi

### Problemi Comuni
- **Mismatch VLAN**: Verificare che configurazioni tag corrispondano a setup rete
- **Creazione Interfacce TAP**: Verificare permessi appropriati e supporto kernel
- **Problemi Prestazioni**: Verificare assegnazione code e distribuzione worker

### Strategie Debug
- **Abilitazione Log Debug**: Utilizzare comandi debug per flusso pacchetti dettagliato
- **Monitoraggio Statistiche**: Monitorare statistiche porte e worker
- **Cattura Pacchetti**: Utilizzare interfacce cattura per analisi traffico

## Documentazione Correlata

- [Comandi Ripetitore Avanzato](enhanced-repeater.md) - Riferimento comandi completo
- [Gestione Worker](worker-lcore-thread-management.md) - Dettagli configurazione worker
- [Gestione Porte](port-management.md) - Configurazione porte DPDK
- [Gestione Interfacce TAP](tap-interface.md) - Dettagli interfacce TAP