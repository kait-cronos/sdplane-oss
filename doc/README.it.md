<div align="center">
<img src="../sdplane-logo.png" alt="sdplane-oss Logo" width="160">
</div>

# sdplane-oss (Piano Dati Software)

Un "Ambiente di Sviluppo DPDK-dock" composto da una shell interattiva che può controllare le operazioni dei thread DPDK e un ambiente di esecuzione dei thread DPDK (sd-plane)

**Language:** [English](../README.md) | [日本語](README.ja.md) | [Français](README.fr.md) | [中文](README.zh.md) | [Deutsch](README.de.md) | **Italiano** | [한국어](README.ko.md) | [ไทย](README.th.md) | [Español](README.es.md)

## Caratteristiche

- **Elaborazione Pacchetti ad Alte Prestazioni**:
  Elaborazione pacchetti zero-copy nello spazio utente utilizzando DPDK
- **Inoltro Layer 2/3**:
  Inoltro L2 e L3 integrato con supporto ACL, LPM e FIB
- **Generazione Pacchetti**:
  Generatore di pacchetti integrato per test e benchmarking
- **Virtualizzazione di Rete**:
  Supporto interfacce TAP e funzionalità di switching VLAN
- **Gestione CLI**:
  Interfaccia a riga di comando interattiva per configurazione e monitoraggio
- **Multi-threading**:
  Modello di threading cooperativo con worker per core

### Architettura
- **Applicazione Principale**: Logica router centrale e inizializzazione
- **Moduli DPDK**: Inoltro L2/L3 e generazione pacchetti
- **Sistema CLI**: Interfaccia a riga di comando con completamento e aiuto
- **Threading**: Multitasking cooperativo basato su lthread
- **Virtualizzazione**: Interfacce TAP e switching virtuale

## Sistemi Supportati

### Requisiti Software
- **OS**:
  Ubuntu 24.04 LTS (attualmente supportato)
- **NIC**:
  [Driver](https://doc.dpdk.org/guides/nics/) | [NIC Supportate](https://core.dpdk.org/supported/)
- **Memoria**:
  Supporto hugepage richiesto
- **CPU**:
  Processore multi-core raccomandato

### Piattaforme Hardware Target

Il progetto è stato testato su:
- **Topton (N305/N100)**: Mini-PC con NIC 10G (testato)
- **Partaker (J3160)**: Mini-PC con NIC 1G (testato)
- **PC Intel Generico**: Con Intel x520 / Mellanox ConnectX5
- **Altre CPU**: Dovrebbe funzionare con processori AMD, ARM, ecc.

## 1. Installazione delle Dipendenze

[Installazione delle Dipendenze](manual/it/install-dependencies.md)

## 2. Installazione da Pacchetto Debian Precompilato

Per Intel Core i3-n305/Celeron j3160, è possibile un'installazione rapida con pacchetti Debian.

Scaricare e installare il pacchetto Debian precompilato:

```bash
# Scaricare il pacchetto più recente per n305
wget https://www.yasuhironet.net/download/n305/sdplane_0.1.4-36_amd64.deb
wget https://www.yasuhironet.net/download/n305/sdplane-dbgsym_0.1.4-36_amd64.ddeb

# o per j3160
wget https://www.yasuhironet.net/download/j3160/sdplane_0.1.4-35_amd64.deb
wget https://www.yasuhironet.net/download/j3160/sdplane-dbgsym_0.1.4-35_amd64.ddeb

# Installare il pacchetto
sudo apt install ./sdplane_0.1.4-*_amd64.deb
sudo apt install ./sdplane-dbgsym_0.1.4-*_amd64.ddeb
```

**Nota**: L'uso di questo binario precompilato su altre CPU potrebbe causare SIGILL (Istruzione Illegale). In tal caso dovete compilare voi stessi. Controllare [download yasuhironet.net](https://www.yasuhironet.net/download/) per la versione pacchetto più recente.

Saltare a 5. Configurazione Sistema.

## 3. Compilazione e Installazione dal Codice Sorgente

[Compilazione e Installazione dal Codice Sorgente](manual/it/build-install-source.md)

## 4. Creazione e Installazione del Pacchetto Debian

[Creazione e Installazione del Pacchetto Debian](manual/it/build-debian-package.md)

## 5. Configurazione Sistema

[Configurazione Sistema](manual/it/system-configuration.md)

## 6. Configurazione sdplane

### File di Configurazione

sdplane utilizza file di configurazione per definire il comportamento all'avvio e l'ambiente di esecuzione.

#### Esempi di Configurazione OS (`etc/`)
- `systemd/sdplane.service`: File servizio systemd
- `netplan/01-netcfg.yaml`: Configurazione rete con netplan

#### Esempi di Configurazione Applicazione (`example-config/`)
- File di configurazione esempio per varie applicazioni
- Script di avvio e profili di configurazione

## 7. Eseguire Applicazioni utilizzando sdplane-oss

```bash
# Esecuzione di base
sudo ./sdplane/sdplane

# Eseguire con file di configurazione
sudo ./sdplane/sdplane -f /path/to/config-file

# Connessione al CLI (da un altro terminale)
telnet localhost 9882

# La shell interattiva è ora disponibile
sdplane> help
sdplane> show version
```

## Suggerimenti

### IOMMU è richiesto quando si usa vfio-pci come driver NIC

Le capacità di virtualizzazione devono essere abilitate:
- Intel: Intel VT-d
- AMD: AMD IOMMU / AMD-V

Queste opzioni devono essere abilitate nelle impostazioni BIOS.
Potrebbe essere necessario modificare anche la configurazione GRUB:

```conf
# /etc/default/grub
GRUB_CMDLINE_LINUX="iommu=pt intel_iommu=on"
```

Applicare le modifiche:
```bash
sudo update-grub
sudo reboot
```

### Configurazione per caricare permanentemente il modulo kernel Linux vfio-pci

```bash
# Creare file di configurazione per caricamento automatico
sudo tee /etc/modules-load.d/vfio-pci.conf > /dev/null <<EOF
vfio-pci
EOF
```

### Per la Serie Mellanox ConnectX

L'installazione del driver è richiesta dal seguente link:

https://network.nvidia.com/products/ethernet-drivers/linux/mlnx_en/

Durante l'installazione, eseguire `./install --dpdk`.
**L'opzione `--dpdk` è obbligatoria.**

Commentare le seguenti impostazioni in sdplane.conf poiché non sono necessarie:

```conf
#set device {pcie-id} driver unbind
#set device {pcie-id} driver {driver-name} driver_override  
#set device {pcie-id} driver {driver-name} bind
```

Per le NIC Mellanox, è necessario eseguire il comando `update port status` per aggiornare le informazioni delle porte.

### Come controllare i numeri bus PCIe

È possibile utilizzare il comando dpdk-devbind.py in DPDK per controllare i numeri bus PCIe delle NIC:

```bash
# Mostrare stato dispositivi di rete  
dpdk-devbind.py -s

# Esempio di output:
Network devices using kernel driver
===================================
0000:04:00.0 'NetXtreme BCM5720 Gigabit Ethernet PCIe 165f' numa_node=0 if=eno8303 drv=tg3 unused= *Active*
0000:b1:00.0 'MT27800 Family [ConnectX-5] 1017' numa_node=1 if=enp177s0np0 drv=mlx5_core unused= *Active*
```

### Ordine thread worker nel file di configurazione

Se si configurano i worker `rib-manager`, `neigh-manager` e `netlink-thread`, devono essere configurati in questo ordine se utilizzati.

### Inizializzazione DPDK

Solo un comando che chiama `rte_eal_init()` dovrebbe essere invocato dal file di configurazione. La funzione `rte_eal_init()` è chiamata da comandi come `rte_eal_init`, `pktgen init`, `l2fwd init` e `l3fwd init`.

## Guida Utente (Manuale)

Guide utente complete e riferimenti comandi sono disponibili:

- [Guida Utente](manual/it/README.md) - Panoramica completa e classificazione comandi

**Guide Applicazioni:**
- [Applicazione Ripetitore L2](manual/it/l2-repeater-application.md) - Inoltro pacchetti Layer 2 semplice con apprendimento MAC
- [Applicazione Ripetitore Avanzato](manual/it/enhanced-repeater-application.md) - Switching consapevole VLAN con interfacce TAP  
- [Applicazione Generatore Pacchetti](manual/it/packet-generator-application.md) - Generazione traffico ad alte prestazioni e test

**Guide Configurazione:**
- [RIB & Routing](manual/it/routing.md) - Funzioni RIB e routing
- [Ripetitore Avanzato](manual/it/enhanced-repeater.md) - Configurazione ripetitore avanzato
- [Gestione Porte & Statistiche](manual/it/port-management.md) - Gestione porte DPDK e statistiche
- [Gestione Worker & lcore & Informazioni Thread](manual/it/worker-lcore-thread-management.md) - Gestione worker threads, lcore e informazioni thread
- [Informazioni Sistema & Monitoraggio](manual/it/system-monitoring.md) - Informazioni sistema e monitoraggio
- [Gestione Dispositivi](manual/it/device-management.md) - Gestione dispositivi e driver

**Guide Sviluppatori:**
- [Guida Integrazione DPDK](manual/it/dpdk-integration-guide.md) - Come integrare applicazioni DPDK
- [Debug & Logging](manual/it/debug-logging.md) - Funzioni debug e logging
- [Configurazione Code](manual/it/queue-configuration.md) - Configurazione e gestione code
- [Interfaccia TAP](manual/it/tap-interface.md) - Gestione interfacce TAP
- [Gestione VTY & Shell](manual/it/vty-shell.md) - Gestione VTY e shell
- [Gestione lthread](manual/it/lthread-management.md) - Gestione thread leggeri cooperativi
- [Generazione Pacchetti](manual/it/packet-generation.md) - Generazione pacchetti utilizzando PKTGEN

## Guida Sviluppatore

### Guida Integrazione
- [Guida Integrazione Applicazioni DPDK](manual/it/dpdk-integration-guide.md) - Come integrare le tue applicazioni DPDK con sdplane-oss utilizzando l'approccio DPDK-dock

### Documentazione
- Tutta la documentazione sviluppatori è inclusa in `doc/`
- Le guide integrazione e esempi sono in `doc/manual/it/`

### Stile Codice

Questo progetto segue gli GNU Coding Standards. Per controllare e formattare il codice:

```bash
# Controllare stile (richiede clang-format 18.1.3+)
./style/check_gnu_style.sh check

# Formattare automaticamente il codice
./style/check_gnu_style.sh update  

# Mostrare differenze
./style/check_gnu_style.sh diff
```

Installare strumenti richiesti:
```bash
# Per Ubuntu 24.04
sudo apt install clang-format-18
```

## Licenza

Questo progetto è sotto licenza MIT - vedere il file [LICENSE](LICENSE) per i dettagli.

## Contatto

- GitHub: https://github.com/kait-cronos/sdplane-oss  
- Issues: https://github.com/kait-cronos/sdplane-oss/issues

## Acquisto Attrezzatura di Valutazione

L'attrezzatura di valutazione può includere funzionalità aggiuntive e modifiche software.

Per richieste sull'attrezzatura di valutazione, contattateci tramite GitHub Issues o direttamente via email.