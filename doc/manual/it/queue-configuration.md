# Configurazione Code

**Lingua / Language:** [English](../queue-configuration.md) | [日本語](../ja/queue-configuration.md) | [Français](../fr/queue-configuration.md) | [中文](../zh/queue-configuration.md) | [Deutsch](../de/queue-configuration.md) | **Italiano** | [한국어](../ko/queue-configuration.md) | [ไทย](../th/queue-configuration.md) | [Español](../es/queue-configuration.md)

Comandi per configurare e gestire le code DPDK.

## Lista Comandi

### update_port_status - Aggiornamento Stato Porta
```
update port status
```

Aggiorna lo stato di tutte le porte.

**Esempio d'uso:**
```bash
update port status
```

Questo comando esegue:
- Verifica stato link di ogni porta
- Aggiorna configurazione code
- Aggiorna informazioni statistiche porta

### set_thread_lcore_port_queue - Configurazione Code Thread
```
set thread <0-128> port <0-128> queue <0-128>
```

Assegna porta e coda al thread specificato.

**Parametri:**
- `<0-128>` (thread) - Numero thread
- `<0-128>` (port) - Numero porta
- `<0-128>` (queue) - Numero coda

**Esempio d'uso:**
```bash
# Assegna coda 0 della porta 0 al thread 0
set thread 0 port 0 queue 0

# Assegna coda 1 della porta 1 al thread 1
set thread 1 port 1 queue 1

# Assegna coda 1 della porta 0 al thread 2
set thread 2 port 0 queue 1
```

### show_thread_qconf - Visualizzazione Configurazione Code Thread
```
show thread qconf
```

Visualizza la configurazione attuale delle code thread.

**Esempio d'uso:**
```bash
show thread qconf
```

## Panoramica Sistema Code

### Concetto Code DPDK
In DPDK, è possibile configurare multiple code di trasmissione e ricezione per ogni porta:

- **Coda di Ricezione (RX Queue)** - Riceve pacchetti in arrivo
- **Coda di Trasmissione (TX Queue)** - Trasmette pacchetti in uscita
- **Multi-coda** - Elaborazione parallela di multiple code

### Importanza della Configurazione Code
Una configurazione appropriata delle code realizza:
- **Miglioramento Prestazioni** - Accelerazione tramite elaborazione parallela
- **Bilanciamento Carico** - Distribuzione elaborazione tra multiple worker
- **Efficienza CPU** - Utilizzo efficace dei core CPU

## Metodo di Configurazione Code

### Procedura di Configurazione Base
1. **Aggiornamento Stato Porta**
```bash
update port status
```

2. **Configurazione Code Thread**
```bash
set thread 0 port 0 queue 0
set thread 1 port 0 queue 1
```

3. **Verifica Configurazione**
```bash
show thread qconf
```

### Pattern di Configurazione Raccomandati

#### Porta Singola, Coda Singola
```bash
set thread 0 port 0 queue 0
```

#### Porta Singola, Code Multiple
```bash
set thread 0 port 0 queue 0
set thread 1 port 0 queue 1
set thread 2 port 0 queue 2
```

#### Porte Multiple, Code Multiple
```bash
set thread 0 port 0 queue 0
set thread 1 port 1 queue 0
set thread 2 port 2 queue 0
```

## Esempi di Configurazione

### Configurazione Ad Alte Prestazioni (4 Core, 4 Porte)
```bash
# Aggiornamento stato porta
update port status

# Assegna porta diversa a ogni core
set thread 0 port 0 queue 0
set thread 1 port 1 queue 0
set thread 2 port 2 queue 0
set thread 3 port 3 queue 0

# Verifica configurazione
show thread qconf
```

### Configurazione Bilanciamento Carico (2 Core, 1 Porta)
```bash
# Aggiornamento stato porta
update port status

# Configura code multiple su una porta
set thread 0 port 0 queue 0
set thread 1 port 0 queue 1

# Verifica configurazione
show thread qconf
```

## Tuning Prestazioni

### Determinazione Numero Code
- **Numero Core CPU** - Configura in base al numero di core CPU disponibili
- **Numero Porte** - Considera il numero di porte fisiche
- **Caratteristiche Traffico** - Pattern di traffico previsto

### Punti di Ottimizzazione
1. **Affinità CPU** - Posizionamento appropriato di core CPU e code
2. **Posizionamento Memoria** - Posizionamento memoria considerando nodi NUMA
3. **Gestione Interrupt** - Gestione efficiente degli interrupt

## Risoluzione Problemi

### Quando la Configurazione Code Non Viene Applicata
1. Aggiorna stato porta
```bash
update port status
```

2. Verifica stato worker
```bash
show worker
```

3. Verifica stato porta
```bash
show port
```

### Quando le Prestazioni Non Migliorano
1. Verifica configurazione code
```bash
show thread qconf
```

2. Verifica carico thread
```bash
show thread counter
```

3. Verifica statistiche porta
```bash
show port statistics
```

## Posizione della Definizione

Questi comandi sono definiti nel seguente file:
- `sdplane/queue_config.c`

## Argomenti Correlati

- [Gestione Porte e Statistiche](port-management.md)
- [Gestione Worker e lcore](worker-management.md)
- [Informazioni Thread](thread-information.md)