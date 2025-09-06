# Gestione Worker e lcore

**Lingua / Language:** [English](../worker-management.md) | [日本語](../ja/worker-management.md) | [Français](../fr/worker-management.md) | [中文](../zh/worker-management.md) | [Deutsch](../de/worker-management.md) | **Italiano** | [한국어](../ko/worker-management.md) | [ไทย](../th/worker-management.md) | [Español](../es/worker-management.md)

Comandi per gestire i thread worker DPDK e gli lcore.

## Lista Comandi

### set_worker - Impostazione Tipo Worker
```
set worker lcore <0-16> (|none|l2fwd|l3fwd|l3fwd-lpm|tap-handler|l2-repeater|enhanced-repeater|vlan-switch|pktgen|linkflap-generator)
```

Imposta il tipo di worker per l'lcore specificato.

**Tipi di Worker:**
- `none` - Nessun worker
- `l2fwd` - Forwarding Layer 2
- `l3fwd` - Forwarding Layer 3
- `l3fwd-lpm` - Forwarding Layer 3 (LPM)
- `tap-handler` - Gestore interfaccia TAP
- `l2-repeater` - Ripetitore Layer 2
- `enhanced-repeater` - Ripetitore avanzato con switching VLAN e interfacce TAP
- `vlan-switch` - Switch VLAN
- `pktgen` - Generatore di pacchetti
- `linkflap-generator` - Generatore di flap link

**Esempio d'uso:**
```bash
# Imposta worker L2 forwarding su lcore 1
set worker lcore 1 l2fwd

# Imposta worker ripetitore avanzato su lcore 1
set worker lcore 1 enhanced-repeater

# Imposta nessun worker su lcore 2
set worker lcore 2 none

# Imposta worker L3 forwarding (LPM) su lcore 3
set worker lcore 3 l3fwd-lpm
```

### reset_worker - Reset Worker
```
reset worker lcore <0-16>
```

Resetta il worker dell'lcore specificato.

**Esempio d'uso:**
```bash
# Resetta il worker dell'lcore 2
reset worker lcore 2
```

### start_worker - Avvio Worker
```
start worker lcore <0-16>
```

Avvia il worker dell'lcore specificato.

**Esempio d'uso:**
```bash
# Avvia il worker dell'lcore 1
start worker lcore 1
```

### restart_worker - Riavvio Worker
```
restart worker lcore <0-16>
```

Riavvia il worker dell'lcore specificato.

**Esempio d'uso:**
```bash
# Riavvia il worker dell'lcore 4
restart worker lcore 4
```

### start_worker_all - Avvio Worker (con opzione all)
```
start worker lcore (<0-16>|all)
```

Avvia il worker dell'lcore specificato o di tutti gli lcore.

**Esempio d'uso:**
```bash
# Avvia il worker dell'lcore 1
start worker lcore 1

# Avvia tutti i worker
start worker lcore all
```

### stop_worker - Arresto Worker
```
stop worker lcore (<0-16>|all)
```

Arresta il worker dell'lcore specificato o di tutti gli lcore.

**Esempio d'uso:**
```bash
# Arresta il worker dell'lcore 1
stop worker lcore 1

# Arresta tutti i worker
stop worker lcore all
```

### reset_worker_all - Reset Worker (con opzione all)
```
reset worker lcore (<0-16>|all)
```

Resetta il worker dell'lcore specificato o di tutti gli lcore.

**Esempio d'uso:**
```bash
# Resetta il worker dell'lcore 2
reset worker lcore 2

# Resetta tutti i worker
reset worker lcore all
```

### restart_worker_all - Riavvio Worker (con opzione all)
```
restart worker lcore (<0-16>|all)
```

Riavvia il worker dell'lcore specificato o di tutti gli lcore.

**Esempio d'uso:**
```bash
# Riavvia il worker dell'lcore 3
restart worker lcore 3

# Riavvia tutti i worker
restart worker lcore all
```

### show_worker - Visualizzazione Informazioni Worker
```
show worker
```

Visualizza lo stato e la configurazione attuali dei worker.

**Esempio d'uso:**
```bash
show worker
```

### set_mempool - Impostazione Memory Pool
```
set mempool
```

Configura il memory pool DPDK.

**Esempio d'uso:**
```bash
set mempool
```

### set_rte_eal_argv - Impostazione Argomenti Riga di Comando RTE EAL
```
set rte_eal argv <WORD> <WORD> <WORD> <WORD> <WORD> <WORD>
```

Imposta gli argomenti della riga di comando utilizzati per l'inizializzazione di RTE EAL (Environment Abstraction Layer).

**Esempio d'uso:**
```bash
# Imposta argomenti EAL
set rte_eal argv -c 0x1 -n 4 --socket-mem 1024,1024 --huge-dir /mnt/huge
```

### rte_eal_init - Inizializzazione RTE EAL
```
rte_eal_init
```

Inizializza RTE EAL (Environment Abstraction Layer).

**Esempio d'uso:**
```bash
rte_eal_init
```

## Descrizione dei Tipi di Worker

### Forwarding L2 (l2fwd)
Worker che esegue il trasferimento di pacchetti a livello Layer 2. Inoltra i pacchetti basandosi sull'indirizzo MAC.

### Forwarding L3 (l3fwd)
Worker che esegue il trasferimento di pacchetti a livello Layer 3. Esegue il routing basandosi sull'indirizzo IP.

### Forwarding L3 LPM (l3fwd-lpm)
Worker di forwarding Layer 3 che utilizza Longest Prefix Matching (LPM).

### Gestore TAP (tap-handler)
Worker che esegue il trasferimento di pacchetti tra interfacce TAP e porte DPDK.

### Ripetitore L2 (l2-repeater)
Worker che esegue la duplicazione/ripetizione di pacchetti a livello Layer 2.


### Switch VLAN (vlan-switch)
Worker di switching che fornisce funzionalità VLAN (Virtual LAN).

### Generatore di Pacchetti (pktgen)
Worker che genera pacchetti per test.

### Generatore di Flap Link (linkflap-generator)
Worker per testare i cambiamenti di stato del link di rete.

## Posizione della Definizione

Questi comandi sono definiti nel seguente file:
- `sdplane/dpdk_lcore_cmd.c`

## Argomenti Correlati

- [Gestione Porte e Statistiche](port-management.md)
- [Informazioni Thread](thread-information.md)
- [Gestione lthread](lthread-management.md)