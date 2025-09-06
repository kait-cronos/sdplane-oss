# Gestione lthread

**Lingua / Language:** [English](../lthread-management.md) | [日本語](../ja/lthread-management.md) | [Français](../fr/lthread-management.md) | [中文](../zh/lthread-management.md) | [Deutsch](../de/lthread-management.md) | **Italiano** | [한국어](../ko/lthread-management.md) | [ไทย](../th/lthread-management.md) | [Español](../es/lthread-management.md)

Comandi per gestire lthread (thread leggeri).

## Lista Comandi

### set_worker_lthread_stat_collector - Configurazione Collettore Statistiche lthread
```
set worker lthread stat-collector
```

Configura un worker lthread che raccoglie informazioni statistiche.

**Esempio d'uso:**
```bash
set worker lthread stat-collector
```

### set_worker_lthread_rib_manager - Configurazione Manager RIB lthread
```
set worker lthread rib-manager
```

Configura un worker lthread che gestisce RIB (Routing Information Base).

**Esempio d'uso:**
```bash
set worker lthread rib-manager
```

### set_worker_lthread_netlink_thread - Configurazione Thread netlink lthread
```
set worker lthread netlink-thread
```

Configura un worker lthread che elabora la comunicazione Netlink.

**Esempio d'uso:**
```bash
set worker lthread netlink-thread
```

## Panoramica lthread

### Cos'è lthread
lthread (lightweight thread) è un'implementazione di thread leggeri che realizza il multithreading cooperativo.

### Caratteristiche Principali
- **Leggerezza** - Basso overhead di memoria
- **Velocità** - Context switch rapido
- **Cooperatività** - Controllo tramite yield esplicito
- **Scalabilità** - Gestione efficiente di grandi quantità di thread

### Differenze con i Thread Tradizionali
- **Preemptive vs Cooperativo** - Cessione esplicita del controllo
- **Thread Kernel vs Thread Utente** - Esecuzione senza passare dal kernel
- **Pesante vs Leggero** - Overhead ridotto per creazione e switching

## Tipi di Worker lthread

### Collettore Statistiche (stat-collector)
Worker che raccoglie periodicamente informazioni statistiche del sistema.

**Funzionalità:**
- Raccolta statistiche porte
- Raccolta statistiche thread
- Raccolta statistiche sistema
- Aggregazione dati statistici

**Esempio d'uso:**
```bash
# Configura collettore statistiche
set worker lthread stat-collector

# Verifica informazioni statistiche
show port statistics
show thread counter
```

### Manager RIB (rib-manager)
Worker che gestisce RIB (Routing Information Base).

**Funzionalità:**
- Gestione tabella routing
- Aggiunta/rimozione/aggiornamento route
- Distribuzione informazioni route
- Monitoraggio stato routing

**Esempio d'uso:**
```bash
# Configura manager RIB
set worker lthread rib-manager

# Verifica informazioni RIB
show rib
```

### Thread Netlink (netlink-thread)
Worker che elabora la comunicazione Netlink.

**Funzionalità:**
- Comunicazione Netlink con kernel
- Monitoraggio configurazioni di rete
- Monitoraggio stato interfacce
- Ricezione informazioni routing

**Esempio d'uso:**
```bash
# Configura thread Netlink
set worker lthread netlink-thread

# Verifica stato rete
show port
show vswitch
```

## Configurazione lthread

### Procedura di Configurazione Base
1. **Configurazione Worker lthread Necessari**
```bash
# Configura collettore statistiche
set worker lthread stat-collector

# Configura manager RIB
set worker lthread rib-manager

# Configura thread Netlink
set worker lthread netlink-thread
```

2. **Verifica Configurazione**
```bash
# Verifica stato worker
show worker

# Verifica informazioni thread
show thread
```

### Configurazione Raccomandata
Per utilizzi generali, si raccomanda la seguente combinazione:
```bash
# Configurazione worker lthread base
set worker lthread stat-collector
set worker lthread rib-manager
set worker lthread netlink-thread
```

## Esempi d'Uso

### Configurazione Base
```bash
# Configura worker lthread
set worker lthread stat-collector
set worker lthread rib-manager
set worker lthread netlink-thread

# Verifica configurazione
show worker
show thread
```

### Configurazione Monitoraggio Statistiche
```bash
# Configura collettore statistiche
set worker lthread stat-collector

# Verifica periodicamente informazioni statistiche
show port statistics
show thread counter
show mempool
```

### Configurazione Gestione Routing
```bash
# Configura manager RIB
set worker lthread rib-manager

# Verifica informazioni routing
show rib
show vswitch
```

## Monitoraggio e Gestione

### Verifica Stato lthread
```bash
# Verifica stato generale
show worker
show thread

# Verifica informazioni statistiche specifiche
show thread counter
show loop-count console pps
```

### Monitoraggio Prestazioni
```bash
# Verifica prestazioni lthread
show thread counter

# Verifica prestazioni sistema completo
show port statistics pps
show mempool
```

## Risoluzione Problemi

### Quando lthread Non Funziona
1. Verifica configurazione worker
```bash
show worker
```

2. Verifica stato thread
```bash
show thread
```

3. Verifica stato sistema
```bash
show rcu
show mempool
```

### Quando le Informazioni Statistiche Non Si Aggiornano
1. Verifica stato collettore statistiche
```bash
show worker
show thread
```

2. Verifica manualmente informazioni statistiche
```bash
show port statistics
show thread counter
```

### Quando RIB Non Si Aggiorna
1. Verifica stato manager RIB
```bash
show worker
show thread
```

2. Verifica informazioni RIB
```bash
show rib
```

3. Verifica stato thread Netlink
```bash
show worker
```

## Funzionalità Avanzate

### Funzionamento Cooperativo lthread
Gli lthread funzionano in modo cooperativo, quindi è necessario prestare attenzione ai seguenti punti:

- **Yield Esplicito** - Per elaborazioni lunghe è necessario yield esplicito
- **Prevenzione Deadlock** - Il design cooperativo previene i deadlock
- **Equità** - Tutti i thread vengono eseguiti appropriatamente

### Ottimizzazione Prestazioni
- **Posizionamento Worker Appropriato** - Considerare l'affinità con i core CPU
- **Efficienza Memoria** - Configurazione appropriata delle dimensioni del memory pool
- **Bilanciamento Carico** - Distribuzione del carico tra worker multipli

## Posizione della Definizione

Questi comandi sono definiti nel seguente file:
- `sdplane/lthread_main.c`

## Argomenti Correlati

- [Gestione Worker e lcore](worker-lcore-thread-management.md)
- [Informazioni Thread](worker-lcore-thread-management.md)
- [Informazioni Sistema e Monitoraggio](system-monitoring.md)
- [RIB e Routing](routing.md)