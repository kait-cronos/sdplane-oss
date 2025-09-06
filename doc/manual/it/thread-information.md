# Informazioni Thread

**Lingua / Language:** [English](../worker-lcore-thread-management.md) | [日本語](../ja/worker-lcore-thread-management.md) | [Français](../fr/worker-lcore-thread-management.md) | [中文](../zh/worker-lcore-thread-management.md) | [Deutsch](../de/worker-lcore-thread-management.md) | **Italiano** | [한국어](../ko/worker-lcore-thread-management.md) | [ไทย](../th/worker-lcore-thread-management.md) | [Español](../es/worker-lcore-thread-management.md)

Comandi per informazioni e monitoraggio dei thread.

## Lista Comandi

### show_thread_cmd - Visualizzazione Informazioni Thread
```
show thread
```

Visualizza lo stato e le informazioni attuali dei thread.

**Esempio d'uso:**
```bash
show thread
```

Questo comando visualizza le seguenti informazioni:
- ID thread
- Stato del thread
- Task in esecuzione
- Utilizzo CPU
- Utilizzo memoria

### show_thread_counter - Visualizzazione Contatori Thread
```
show thread counter
```

Visualizza le informazioni sui contatori dei thread.

**Esempio d'uso:**
```bash
show thread counter
```

Questo comando visualizza le seguenti informazioni:
- Numero di pacchetti elaborati
- Numero di esecuzioni
- Numero di errori
- Statistiche tempo di elaborazione

## Panoramica Sistema Thread

### Architettura Thread sdplane
In sdplane vengono utilizzati i seguenti tipi di thread:

#### 1. Thread Worker
- **Forwarding L2** - Trasferimento pacchetti Layer 2
- **Forwarding L3** - Trasferimento pacchetti Layer 3
- **Gestore TAP** - Elaborazione interfacce TAP
- **PKTGEN** - Generazione pacchetti

#### 2. Thread di Gestione
- **Manager RIB** - Gestione informazioni routing
- **Collettore Statistiche** - Raccolta informazioni statistiche
- **Thread Netlink** - Elaborazione comunicazioni Netlink

#### 3. Thread di Sistema
- **Server VTY** - Elaborazione connessioni VTY
- **Console** - Input/output console

### Relazione con lthread
sdplane utilizza un modello di multithreading cooperativo (cooperative threading):

- **lthread** - Implementazione thread leggeri
- **Scheduling Cooperativo** - Controllo tramite yield esplicito
- **Alta Efficienza** - Riduzione overhead context switch

## Come Leggere le Informazioni Thread

### Elementi di Visualizzazione Base
- **Thread ID** - Identificatore del thread
- **Name** - Nome del thread
- **State** - Stato del thread
- **lcore** - Core CPU in esecuzione
- **Type** - Tipo di thread

### Stati Thread
- **Running** - In esecuzione
- **Ready** - Eseguibile
- **Blocked** - Bloccato
- **Terminated** - Terminato

### Informazioni Contatori
- **Packets** - Numero pacchetti elaborati
- **Loops** - Numero esecuzioni loop
- **Errors** - Numero errori
- **CPU Time** - Tempo utilizzo CPU

## Esempi d'Uso

### Monitoraggio Base
```bash
# Visualizzazione informazioni thread
show thread

# Visualizzazione contatori thread
show thread counter
```

### Interpretazione Output di Esempio
```bash
# Esempio output show thread
Thread ID: 1
Name: l2fwd-worker
State: Running
lcore: 1
Type: L2FWD

Thread ID: 2
Name: rib-manager
State: Running
lcore: 2
Type: RIB_MANAGER
```

```bash
# Esempio output show thread counter
Thread ID: 1
Packets: 1000000
Loops: 5000000
Errors: 0
CPU Time: 123.45s
```

## Monitoraggio e Risoluzione Problemi

### Monitoraggio Periodico
```bash
# Comandi di monitoraggio periodico
show thread
show thread counter
```

### Analisi Prestazioni
```bash
# Informazioni relative alle prestazioni
show thread counter
show loop-count l2fwd pps
show worker
```

### Risoluzione Problemi

#### Quando i Thread Non Rispondono
1. Verifica stato thread
```bash
show thread
```

2. Verifica stato worker
```bash
show worker
```

3. Riavvia se necessario
```bash
restart worker lcore 1
```

#### Quando le Prestazioni Sono Degradate
1. Verifica informazioni contatori
```bash
show thread counter
```

2. Verifica contatori loop
```bash
show loop-count l2fwd pps
```

3. Verifica numero errori
```bash
show thread counter
```

#### Quando l'Utilizzo Memoria È Elevato
1. Verifica informazioni memory pool
```bash
show mempool
```

2. Verifica informazioni thread
```bash
show thread
```

## Ottimizzazione Thread

### Configurazione Affinità CPU
- Posizionamento worker su lcore appropriati
- Posizionamento considerando nodi NUMA
- Equalizzazione utilizzo CPU

### Miglioramento Efficienza Memoria
- Dimensioni memory pool appropriate
- Prevenzione memory leak
- Miglioramento efficienza cache

## Funzionalità Avanzate

### Gestione lthread
```bash
# Configurazione worker lthread
set worker lthread stat-collector
set worker lthread rib-manager
set worker lthread netlink-thread
```

### Utilizzo Informazioni Statistiche
- Monitoraggio prestazioni
- Pianificazione capacità
- Rilevamento anomalie

## Posizione della Definizione

Questi comandi sono definiti nel seguente file:
- `sdplane/thread_info.c`

## Argomenti Correlati

- [Gestione Worker e lcore](worker-lcore-thread-management.md)
- [Gestione lthread](lthread-management.md)
- [Informazioni Sistema e Monitoraggio](system-monitoring.md)