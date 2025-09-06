# Guida Utente sdplane-oss

**Lingua / Language:** [English](../README.md) | [日本語](../ja/README.md) | [Français](../fr/README.md) | [中文](../zh/README.md) | [Deutsch](../de/README.md) | **Italiano**

sdplane-oss è un router software ad alte prestazioni basato su DPDK. Questa guida utente descrive tutti i comandi e le funzionalità di sdplane.

## Indice

1. [Gestione Porte e Statistiche](port-management.md) - Gestione delle porte DPDK e informazioni statistiche
2. [Gestione Worker e lcore](worker-management.md) - Gestione dei thread worker e lcore
3. [Debug e Log](debug-logging.md) - Funzionalità di debug e logging
4. [Gestione VTY e Shell](vty-shell.md) - Gestione di VTY e shell
5. [Informazioni Sistema e Monitoraggio](system-monitoring.md) - Informazioni di sistema e funzionalità di monitoraggio
6. [RIB e Routing](routing.md) - Funzionalità RIB e routing
7. [Configurazione Code](queue-configuration.md) - Configurazione e gestione delle code
8. [Generazione Pacchetti](packet-generation.md) - Generazione pacchetti usando PKTGEN
9. [Informazioni Thread](thread-information.md) - Informazioni e monitoraggio dei thread
10. [Interfaccia TAP](tap-interface.md) - Gestione delle interfacce TAP
11. [Gestione lthread](lthread-management.md) - Gestione di lthread
12. [Gestione Dispositivi](device-management.md) - Gestione di dispositivi e driver

## Uso Base

### Metodo di Connessione

Per connettersi a sdplane:

```bash
# Avviare sdplane
sudo ./sdplane/sdplane

# Connettersi alla CLI da un altro terminale
telnet localhost 9882
```

### Visualizzazione dell'Aiuto

È possibile utilizzare `?` per visualizzare l'aiuto per ogni comando:

```
sdplane# ?
sdplane# show ?
sdplane# set ?
```

### Comandi Base

- `show version` - Visualizzazione delle informazioni sulla versione
- `show port` - Visualizzazione delle informazioni sulle porte
- `show worker` - Visualizzazione delle informazioni sui worker
- `exit` - Uscita dalla CLI

## Classificazione dei Comandi

sdplane ha 79 comandi definiti, classificati nelle seguenti 13 categorie funzionali:

1. **Gestione Porte e Statistiche** (10 comandi) - Controllo e statistiche delle porte DPDK
2. **Gestione Worker e lcore** (6 comandi) - Gestione dei thread worker e lcore
3. **Debug e Log** (2 comandi) - Funzionalità di debug e logging
4. **Gestione VTY e Shell** (4 comandi) - Controllo di VTY e shell
5. **Informazioni Sistema e Monitoraggio** (10 comandi) - Informazioni di sistema e monitoraggio
6. **RIB e Routing** (1 comando) - Gestione delle informazioni di routing
7. **Configurazione Code** (3 comandi) - Configurazione delle code
8. **Generazione Pacchetti** (3 comandi) - Generazione pacchetti con PKTGEN
9. **Informazioni Thread** (2 comandi) - Monitoraggio dei thread
10. **Interfaccia TAP** (2 comandi) - Gestione delle interfacce TAP
11. **Gestione lthread** (3 comandi) - Gestione di lthread
12. **Gestione Dispositivi** (2 comandi) - Gestione di dispositivi e driver

Per informazioni dettagliate sull'utilizzo, consultare la documentazione di ogni categoria.