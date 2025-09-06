# Guida Integrazione Applicazioni DPDK

**Lingua / Language:** [English](../dpdk-integration-guide.md) | [日本語](../ja/dpdk-integration-guide.md) | [Français](../fr/dpdk-integration-guide.md) | [中文](../zh/dpdk-integration-guide.md) | [Deutsch](../de/dpdk-integration-guide.md) | **Italiano** | [한국어](../ko/dpdk-integration-guide.md) | [ไทย](../th/dpdk-integration-guide.md) | [Español](../es/dpdk-integration-guide.md)

Questa guida spiega come integrare applicazioni DPDK esistenti e i loro thread worker basati su pthread nel framework sdplane utilizzando l'approccio DPDK-dock.

## Panoramica

sdplane fornisce un framework di integrazione per eseguire applicazioni DPDK multiple in un singolo spazio di processo utilizzando l'**approccio DPDK-dock**. Invece che ogni applicazione DPDK inizializzi il proprio ambiente EAL, le applicazioni possono essere integrate come moduli worker nel modello di threading cooperativo di sdplane.

L'approccio DPDK-dock consente a applicazioni DPDK multiple di condividere risorse efficientemente e coesistere attraverso:
- Inizializzazione EAL DPDK centralizzata
- Gestione memory pool condivisa
- Configurazione unificata porte e code
- Multitasking cooperativo tra carichi di lavoro DPDK diversi

## Punti Chiave di Integrazione

### Conversione Thread Worker
- Conversione worker pthread DPDK tradizionali a worker lcore sdplane
- Sostituzione `pthread_create()` con `set worker lcore <id> <worker-type>` di sdplane
- Integrazione con il modello di threading basato su lcore di sdplane

### Integrazione Inizializzazione
- Rimozione chiamate `rte_eal_init()` specifiche dell'applicazione
- Utilizzo inizializzazione EAL centralizzata di sdplane
- Registrazione comandi CLI specifici dell'applicazione tramite framework comandi sdplane

## Procedura di Integrazione

### 1. Identificazione Funzioni Worker
Identificazione dei loop principali di elaborazione pacchetti nell'applicazione DPDK. Queste sono tipicamente funzioni che:
- Operano in loop infiniti che elaborano pacchetti
- Utilizzano `rte_eth_rx_burst()` e `rte_eth_tx_burst()`
- Gestiscono logica di forwarding o elaborazione pacchetti

### 2. Creazione Moduli Worker
Implementazione funzioni worker seguendo l'interfaccia worker di sdplane:

```c
static __thread uint64_t loop_counter = 0;

int
my_worker_function(__rte_unused void *dummy)
{
    unsigned lcore_id = rte_lcore_id();
    int thread_id;
    
    // Registra contatore loop per monitoraggio
    thread_id = thread_lookup_by_lcore(my_worker_function, lcore_id);
    thread_register_loop_counter(thread_id, &loop_counter);
    
    while (!force_quit && !force_stop[lcore_id]) {
        // Logica elaborazione pacchetti qui
        
        // Incrementa contatore loop per monitoraggio
        loop_counter++;
    }
    
    return 0;
}
```

### 3. Accesso Informazioni RIB tramite RCU
Per accedere a informazioni e configurazioni porte nei worker di elaborazione pacchetti DPDK, sdplane fornisce accesso RIB (Routing Information Base) tramite RCU (Read-Copy-Update) per operazioni thread-safe.

```c
#if HAVE_LIBURCU_QSBR
#include <urcu/urcu-qsbr.h>
#endif /*HAVE_LIBURCU_QSBR*/

static __thread struct rib *rib = NULL;

int
my_worker_function(__rte_unused void *dummy)
{
    unsigned lcore_id = rte_lcore_id();
    
#if HAVE_LIBURCU_QSBR
    urcu_qsbr_register_thread();
#endif /*HAVE_LIBURCU_QSBR*/

    while (!force_quit && !force_stop[lcore_id]) {
#if HAVE_LIBURCU_QSBR
        urcu_qsbr_read_lock();
        rib = (struct rib *) rcu_dereference(rcu_global_ptr_rib);
#endif /*HAVE_LIBURCU_QSBR*/

        // Logica elaborazione pacchetti qui
        // Accesso informazioni porta tramite rib->rib_info->port[portid]
        
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

## Applicazioni Integrate

Le seguenti applicazioni DPDK sono state integrate con successo in sdplane utilizzando l'approccio DPDK-dock:

### Integrazione L2FWD (`module/l2fwd/`)
**Stato**: ✅ Integrazione riuscita
- Applicazione DPDK l2fwd originale adattata per sdplane
- Dimostra forwarding pacchetti base tra porte
- **Materiali di riferimento**: File patch disponibili per codice sorgente DPDK l2fwd originale

### Integrazione PKTGEN (`module/pktgen/`)
**Stato**: ✅ Integrazione riuscita
- Integrazione completa applicazione DPDK PKTGEN
- Esempio di applicazione multi-thread complessa
- Integrazione libreria esterna con CLI sdplane

## Riferimenti Integrazione Raccomandati

Per sviluppatori che integrano nuove applicazioni DPDK, si raccomanda l'uso dei seguenti come riferimenti principali:

### 1. Integrazione L2FWD (Raccomandato)
- **Ubicazione**: `module/l2fwd/`
- **Ambito**: Comprensione conversione pthread-to-lcore base
- **Pattern Integrazione**: Worker forwarding pacchetti semplice

### 2. Integrazione PKTGEN (Riferimento Avanzato)
- **Ubicazione**: `module/pktgen/` e `sdplane/pktgen_cmd.c`
- **Ambito**: Integrazione applicazioni complesse con comandi CLI
- **Pattern Integrazione**: Applicazioni multi-componente con dipendenze esterne

## Best Practice

### Considerazioni Prestazioni
- Utilizzo elaborazione burst per prestazioni ottimali
- Minimizzazione copia pacchetti quando possibile
- Sfruttamento meccanismi zero-copy DPDK
- Considerazione topologia NUMA in assegnazioni worker

### Gestione Errori
- Controllo valori di ritorno funzioni DPDK
- Implementazione corretta gestione shutdown
- Utilizzo livelli log appropriati
- Gestione corretta pulizia risorse

## Debug e Monitoraggio

### Log Debug
Abilitazione log debug per worker:

```bash
debug sdplane my-worker-type
```

### Monitoraggio CLI
Fornitura comandi stato per monitoraggio:

```bash
show my-worker status
show my-worker statistics
```

## Pattern Integrazione Comuni

### Pipeline Elaborazione Pacchetti
```c
// Elaborazione pacchetti tipica in worker sdplane
while (!force_quit && !force_stop[lcore_id]) {
    // 1. Ricezione pacchetti
    nb_rx = rte_eth_rx_burst(portid, queueid, pkts_burst, MAX_PKT_BURST);
    
    // 2. Elaborazione pacchetti
    for (i = 0; i < nb_rx; i++) {
        // Logica elaborazione
        process_packet(pkts_burst[i]);
    }
    
    // 3. Trasmissione pacchetti
    rte_eth_tx_burst(dst_port, queueid, pkts_burst, nb_rx);
}
```

## Risoluzione Problemi

### Problemi Comuni
- **Worker non si avvia**: Verificare assegnazione lcore e configurazione
- **Drop pacchetti**: Verificare configurazione code e dimensioni buffer
- **Problemi prestazioni**: Verificare affinità CPU e configurazione NUMA
- **Comandi CLI non funzionano**: Verificare corretta registrazione comandi

Per esempi implementativi dettagliati, fare riferimento ai moduli worker esistenti nel codebase e alle relative definizioni comandi CLI.