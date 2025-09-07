# RIB e Routing

**Language:** [English](../en/routing.md) | [日本語](../ja/routing.md) | [Français](../fr/routing.md) | [中文](../zh/routing.md) | [Deutsch](../de/routing.md) | **Italiano** | [한국어](../ko/routing.md) | [ไทย](../th/routing.md) | [Español](../es/routing.md)

Comandi per gestire RIB (Routing Information Base) e informazioni delle risorse di sistema.

## Lista Comandi

### show_rib - Visualizzazione Informazioni RIB
```
show rib
```

Visualizza le informazioni della RIB (Routing Information Base).

**Esempio d'uso:**
```bash
show rib
```

Questo comando visualizza le seguenti informazioni:
- Versione RIB e puntatore memoria
- Configurazione switch virtuale e assegnazione VLAN
- Stato porte DPDK e configurazione code
- Assegnazione code lcore-to-port
- Tabelle neighbor per forwarding L2/L3

## Panoramica RIB

### Cos'è RIB
RIB (Routing Information Base) è un database centrale che memorizza le risorse di sistema e le informazioni di rete. In sdplane, gestisce le seguenti informazioni:

- **Configurazione Switch Virtuale** - Switching VLAN e assegnazione porte
- **Informazioni Porte DPDK** - Stato link, configurazione code, informazioni funzionalità
- **Configurazione Code lcore** - Assegnazione elaborazione pacchetti per core CPU
- **Tabelle Neighbor** - Entry database forwarding L2/L3

### Struttura RIB
RIB è composta da due strutture principali:

```c
struct rib {
    struct rib_info *rib_info;  // Puntatore ai dati effettivi
};

struct rib_info {
    uint32_t ver;                                    // Numero versione
    uint8_t vswitch_size;                           // Numero switch virtuali
    uint8_t port_size;                              // Numero porte DPDK
    uint8_t lcore_size;                             // Numero lcore
    struct vswitch_conf vswitch[MAX_VSWITCH];       // Configurazione switch virtuali
    struct vswitch_link vswitch_link[MAX_VSWITCH_LINK]; // Link porte VLAN
    struct port_conf port[MAX_ETH_PORTS];           // Configurazione porte DPDK
    struct lcore_qconf lcore_qconf[RTE_MAX_LCORE];  // Assegnazione code lcore
    struct neigh_table neigh_tables[NEIGH_NR_TABLES]; // Tabelle neighbor/forwarding
};
```

## Come Leggere le Informazioni RIB

### Elementi di Visualizzazione Base
- **Destination** - Indirizzo di rete destinazione
- **Netmask** - Maschera di rete
- **Gateway** - Gateway (next hop)
- **Interface** - Interfaccia di output
- **Metric** - Valore metrico del route
- **Status** - Stato del route

### Stati dei Route
- **Active** - Route attivo
- **Inactive** - Route inattivo
- **Pending** - Route in configurazione
- **Invalid** - Route non valido

## Esempi d'Uso

### Utilizzo Base
```bash
# Visualizzazione informazioni RIB
show rib
```

### Interpretazione Output di Esempio
```
rib information version: 21 (0x55555dd42010)
vswitches: 
dpdk ports: 
  dpdk_port[0]: 
    link: speed=1000Mbps duplex=full autoneg=on status=up
    nb_rxd=1024 nb_txd=1024
    queues: nrxq=1 ntxq=4
  dpdk_port[1]: 
    link: speed=0Mbps duplex=half autoneg=on status=down
    nb_rxd=1024 nb_txd=1024
    queues: nrxq=1 ntxq=4
  dpdk_port[2]: 
    link: speed=0Mbps duplex=half autoneg=off status=down
    nb_rxd=1024 nb_txd=1024
    queues: nrxq=1 ntxq=4
lcores: 
  lcore[0]: nrxq=0
  lcore[1]: nrxq=1
    rxq[0]: dpdk_port[0], queue_id=0
  lcore[2]: nrxq=1
    rxq[0]: dpdk_port[1], queue_id=0
  lcore[3]: nrxq=1
    rxq[0]: dpdk_port[2], queue_id=0
  lcore[4]: nrxq=0
  lcore[5]: nrxq=0
  lcore[6]: nrxq=0
  lcore[7]: nrxq=0
```

Questo esempio mostra:
- RIB versione 21 indica lo stato corrente del sistema
- Porta DPDK 0 attiva (up) con velocità link 1Gbps
- Porte DPDK 1, 2 inattive (down) senza link
- lcore 1, 2, 3 responsabili dell'elaborazione pacchetti dalle porte 0, 1, 2 rispettivamente
- Ogni porta utilizza 1 coda RX e 4 code TX
- Ring descrittori RX/TX configurati con 1024 entry

## Gestione RIB

### Aggiornamento Automatico
RIB viene aggiornata automaticamente nei seguenti momenti:
- Cambiamenti di stato interfaccia
- Modifiche configurazione di rete
- Ricezione informazioni da protocolli di routing

### Aggiornamento Manuale
Per verificare manualmente le informazioni RIB:
```bash
show rib
```

## Risoluzione Problemi

### Quando il Routing Non Funziona Correttamente
1. Verifica informazioni RIB
```bash
show rib
```

2. Verifica stato interfacce
```bash
show port
```

3. Verifica stato worker
```bash
show worker
```

### Quando i Route Non Appaiono in RIB
- Verifica configurazione di rete
- Verifica stato interfacce
- Verifica funzionamento RIB manager

## Funzionalità Avanzate

### RIB Manager
RIB manager funziona come thread indipendente e fornisce le seguenti funzionalità:
- Aggiornamento automatico informazioni routing
- Controllo validità route
- Monitoraggio stato di rete

### Worker Correlati
- **rib-manager** - Worker che gestisce RIB
- **l3fwd** - Utilizza RIB per forwarding Layer 3
- **l3fwd-lpm** - Coordinamento tabelle LPM e RIB

## Posizione della Definizione

Questo comando è definito nel seguente file:
- `sdplane/rib.c`

## Argomenti Correlati

- [Gestione Worker e lcore](worker-lcore-thread-management.md)
- [Gestione lthread](lthread-management.md)
- [Informazioni Sistema e Monitoraggio](system-monitoring.md)