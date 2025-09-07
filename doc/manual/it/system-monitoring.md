# Informazioni Sistema e Monitoraggio

**Language:** [English](../en/system-monitoring.md) | [日本語](../ja/system-monitoring.md) | [Français](../fr/system-monitoring.md) | [中文](../zh/system-monitoring.md) | [Deutsch](../de/system-monitoring.md) | **Italiano** | [한국어](../ko/system-monitoring.md) | [ไทย](../th/system-monitoring.md) | [Español](../es/system-monitoring.md)

Comandi che forniscono informazioni di sistema e funzionalità di monitoraggio per sdplane.

## Lista Comandi

### show_version - Visualizzazione Versione
```
show version
```

Visualizza le informazioni sulla versione di sdplane.

**Esempio d'uso:**
```bash
show version
```

### set_locale - Impostazione Locale
```
set locale (C|C.utf8|en_US.utf8|POSIX)
```

Imposta il locale del sistema.

**Locale Disponibili:**
- `C` - Locale C standard
- `C.utf8` - Locale C con supporto UTF-8
- `en_US.utf8` - Locale inglese UTF-8
- `POSIX` - Locale POSIX

**Esempio d'uso:**
```bash
# Imposta locale C con supporto UTF-8
set locale C.utf8

# Imposta locale inglese UTF-8
set locale en_US.utf8
```

### set_argv_list_1 - Impostazione argv-list
```
set argv-list <0-7> <WORD>
```

Configura la lista degli argomenti della riga di comando.

**Parametri:**
- `<0-7>` - Indice (0-7)
- `<WORD>` - Stringa da impostare

**Esempio d'uso:**
```bash
# Imposta argomento all'indice 0
set argv-list 0 "--verbose"

# Imposta argomento all'indice 1
set argv-list 1 "--config"
```

### **show argv-list**

Visualizza tutte le liste degli argomenti della riga di comando configurate.

**Esempio d'uso:**
```bash
# Visualizza tutti gli argv-list
show argv-list
```

---

### **show argv-list \<0-7\>**

Visualizza l'argv-list di un indice specifico.

**Esempio d'uso:**
```bash
# Visualizza argv-list di indice specifico
show argv-list 0

# Visualizza argv-list indice 3
show argv-list 3
```

### show_loop_count - Visualizzazione Contatore Loop
```
show loop-count (console|vty-shell|l2fwd) (pps|total)
```

Visualizza i contatori dei loop di ogni componente.

**Componenti:**
- `console` - Console
- `vty-shell` - Shell VTY
- `l2fwd` - Forwarding L2

**Tipi di Statistiche:**
- `pps` - Numero di loop per secondo
- `total` - Numero totale di loop

**Esempio d'uso:**
```bash
# Visualizza PPS della console
show loop-count console pps

# Visualizza numero totale di loop L2 forwarding
show loop-count l2fwd total
```

### show_rcu - Visualizzazione Informazioni RCU
```
show rcu
```

Visualizza le informazioni RCU (Read-Copy-Update).

**Esempio d'uso:**
```bash
show rcu
```

### show_fdb - Visualizzazione Informazioni FDB
```
show fdb
```

Visualizza le informazioni FDB (Forwarding Database).

**Esempio d'uso:**
```bash
show fdb
```

### show_vswitch - Visualizzazione Informazioni vswitch
```
show vswitch
```

Visualizza le informazioni dello switch virtuale.

**Esempio d'uso:**
```bash
show vswitch
```

### sleep_cmd - Comando Sleep
```
sleep <0-300>
```

Sospende per il numero di secondi specificato.

**Parametri:**
- `<0-300>` - Tempo di sospensione (secondi)

**Esempio d'uso:**
```bash
# Sospende per 5 secondi
sleep 5

# Sospende per 30 secondi
sleep 30
```

### show_mempool - Visualizzazione Informazioni Memory Pool
```
show mempool
```

Visualizza le informazioni del memory pool DPDK.

**Esempio d'uso:**
```bash
show mempool
```

## Descrizione degli Elementi di Monitoraggio

### Informazioni Versione
- Versione di sdplane
- Informazioni di build
- Versioni delle librerie dipendenti

### Contatori Loop
- Numero di loop di elaborazione di ogni componente
- Utilizzato per il monitoraggio delle prestazioni
- Utilizzato per il calcolo PPS (Packets Per Second)

### Informazioni RCU
- Stato del meccanismo Read-Copy-Update
- Situazione della sincronizzazione dei processi
- Stato della gestione memoria

### Informazioni FDB
- Stato della tabella indirizzi MAC
- Indirizzi MAC appresi
- Informazioni di aging

### Informazioni vswitch
- Configurazione dello switch virtuale
- Informazioni porte
- Configurazione VLAN

### Informazioni Memory Pool
- Memoria disponibile
- Memoria in uso
- Statistiche del memory pool

## Best Practice di Monitoraggio

### Monitoraggio Periodico
```bash
# Comandi di monitoraggio base
show version
show mempool
show vswitch
show rcu
```

### Monitoraggio Prestazioni
```bash
# Monitoraggio prestazioni tramite contatori loop
show loop-count console pps
show loop-count l2fwd pps
```

### Risoluzione Problemi
```bash
# Verifica stato sistema
show fdb
show vswitch
show mempool
```

## Posizione della Definizione

Questi comandi sono definiti nel seguente file:
- `sdplane/sdplane.c`

## Argomenti Correlati

- [Gestione Porte e Statistiche](port-management.md)
- [Gestione Worker e lcore](worker-lcore-thread-management.md)
- [Informazioni Thread](worker-lcore-thread-management.md)