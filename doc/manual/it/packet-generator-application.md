# Applicazione Generatore di Pacchetti (PKTGEN)

**Lingua / Language:** [English](../packet-generator-application.md) | [日本語](../ja/packet-generator-application.md) | [Français](../fr/packet-generator-application.md) | [中文](../zh/packet-generator-application.md) | [Deutsch](../de/packet-generator-application.md) | **Italiano** | [한국어](../ko/packet-generator-application.md) | [ไทย](../th/packet-generator-application.md) | [Español](../es/packet-generator-application.md)

L'applicazione Generatore di Pacchetti (PKTGEN) fornisce funzionalità di generazione pacchetti ad alte prestazioni e test del traffico utilizzando il framework di elaborazione pacchetti ottimizzato DPDK.

## Panoramica

PKTGEN è uno strumento sofisticato di generazione traffico integrato in sdplane-oss che realizza:
- **Generazione Pacchetti ad Alta Velocità**: Velocità di trasmissione pacchetti multi-gigabit
- **Pattern Traffico Flessibili**: Dimensioni pacchetti, velocità e pattern personalizzabili
- **Supporto Multi-Porta**: Generazione traffico indipendente su porte multiple
- **Funzionalità Avanzate**: Test di range, limitazione velocità, shaping traffico
- **Test Prestazioni**: Misurazione throughput e latenza di rete

## Architettura

### Componenti Principali
- **Engine TX**: Trasmissione pacchetti ad alte prestazioni usando DPDK
- **Engine RX**: Ricezione pacchetti e raccolta statistiche
- **Framework L2P**: Mappatura lcore-to-port per prestazioni ottimali
- **Gestione Configurazione**: Configurazione dinamica parametri traffico
- **Engine Statistiche**: Metriche prestazioni e reporting comprensivi

### Modello Worker
PKTGEN opera utilizzando thread worker dedicati (lcore):
- **Worker TX**: Core dedicati trasmissione pacchetti
- **Worker RX**: Core dedicati ricezione pacchetti
- **Worker Misti**: TX/RX combinati su singolo core
- **Thread Controllo**: Gestione e raccolta statistiche

## Caratteristiche Principali

### Generazione Traffico
- **Velocità Pacchetti**: Generazione traffico line rate fino ai limiti interfaccia
- **Dimensioni Pacchetti**: Configurabile da 64 byte a jumbo frame
- **Pattern Traffico**: Uniforme, burst, pattern personalizzati
- **Multi-Stream**: Stream traffico multipli per porta

### Funzionalità Avanzate
- **Limitazione Velocità**: Controllo preciso velocità traffico
- **Test Range**: Sweep dimensioni pacchetti e velocità
- **Pattern Carico**: Traffico costante, ramp-up, burst
- **Supporto Protocolli**: Ethernet, IPv4, IPv6, TCP, UDP

## Configurazione

### Configurazione Base
PKTGEN richiede inizializzazione specifica e configurazione worker:

```bash
# Binding dispositivi per PKTGEN
set device 01:00.0 driver vfio-pci bind
set device 01:00.1 driver vfio-pci bind

# Inizializzazione PKTGEN con argv-list
set argv-list 2 ./usr/local/bin/pktgen -l 0-7 -n 4 --proc-type auto --log-level 7 --file-prefix pg -- -v -T -P -l pktgen.log -m [4:5].0 -m [6:7].1 -f themes/black-yellow.theme

# Inizializza PKTGEN
pktgen init argv-list 2
```

### Assegnazione Worker
Assegnazione core dedicati per prestazioni ottimali:

```bash
# Assegna worker PKTGEN a lcore specifici
set worker lcore 4 pktgen  # Porta 0 TX/RX
set worker lcore 5 pktgen  # Porta 0 TX/RX
set worker lcore 6 pktgen  # Porta 1 TX/RX
set worker lcore 7 pktgen  # Porta 1 TX/RX
```

## Comandi CLI

### Controllo PKTGEN

#### Inizializzazione
```bash
# Inizializza PKTGEN con configurazione argv-list
pktgen init argv-list <0-7>
```

#### Avvio/Arresto Generazione Traffico
```bash
# Avvia generazione traffico su porta specifica
pktgen do start port <0-7>
pktgen do start port all

# Arresta generazione traffico
pktgen do stop port <0-7>
pktgen do stop port all
```

#### Configurazione Traffico

##### Impostazione Numero Pacchetti
```bash
# Imposta numero pacchetti da trasmettere
pktgen do set port <0-7> count <0-4000000000>
pktgen do set port all count <0-4000000000>
```

##### Impostazione Dimensione Pacchetti
```bash
# Imposta dimensione pacchetti in byte
pktgen do set port <0-7> size <0-9999>
pktgen do set port all size <0-9999>
```

##### Impostazione Velocità Trasmissione
```bash
# Imposta velocità trasmissione in percentuale
pktgen do set port <0-7> rate <0-100>
pktgen do set port all rate <0-100>
```

## Casi d'Uso

### Test Prestazioni Rete
- **Test Throughput**: Misurazione larghezza banda massima
- **Test Latenza**: Analisi latenza end-to-end
- **Test Carico**: Generazione traffico sostenuto
- **Stress Test**: Verifica packet rate massimi

### Validazione Apparati
- **Test Switch**: Verifica prestazioni forwarding
- **Test Router**: Prestazioni forwarding L3
- **Test Interfacce**: Validazione porte e cavi
- **Test Protocolli**: Comportamento protocolli specifici

## Monitoraggio

### Comandi Stato
```bash
# Visualizza stato e configurazione PKTGEN
show pktgen

# Visualizza statistiche porte
show port statistics all
```

## Posizione della Definizione

Questi comandi sono definiti nei seguenti file:
- `sdplane/pktgen_cmd.c`
- `module/pktgen/`

## Argomenti Correlati

- [Generazione Pacchetti](packet-generation.md) - Riferimento comandi
- [Gestione Worker](worker-lcore-thread-management.md) - Configurazione worker
- [Gestione Porte](port-management.md) - Configurazione porte DPDK