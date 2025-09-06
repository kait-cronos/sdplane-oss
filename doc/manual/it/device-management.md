# Gestione Dispositivi

**Lingua / Language:** [English](../device-management.md) | [日本語](../ja/device-management.md) | [Français](../fr/device-management.md) | [中文](../zh/device-management.md) | [Deutsch](../de/device-management.md) | **Italiano** | [한국어](../ko/device-management.md) | [ไทย](../th/device-management.md) | [Español](../es/device-management.md)

Comandi per gestire dispositivi e driver DPDK.

## Lista Comandi

### show_devices - Visualizzazione Informazioni Dispositivi
```
show devices
```

Visualizza le informazioni sui dispositivi disponibili nel sistema.

**Esempio d'uso:**
```bash
show devices
```

Questo comando visualizza le seguenti informazioni:
- Nome dispositivo
- Tipo dispositivo
- Driver attuale
- Stato del dispositivo
- Indirizzo PCI

### set_device_driver - Configurazione Driver Dispositivo
```
set device <WORD> driver (ixgbe|igb|igc|uio_pci_generic|igb_uio|vfio-pci|unbound) (|bind|driver_override)
```

Imposta il driver per il dispositivo specificato.

**Parametri:**
- `<WORD>` - Nome dispositivo o indirizzo PCI
- Tipi di driver:
  - `ixgbe` - Driver Intel 10GbE ixgbe
  - `igb` - Driver Intel 1GbE igb
  - `igc` - Driver Intel 2.5GbE igc
  - `uio_pci_generic` - Driver UIO generico
  - `igb_uio` - Driver DPDK UIO
  - `vfio-pci` - Driver VFIO PCI
  - `unbound` - Rimuove driver
- Modalità operativa:
  - `bind` - Esegue bind del driver
  - `driver_override` - Override del driver

**Esempio d'uso:**
```bash
# Bind dispositivo al driver vfio-pci
set device 0000:01:00.0 driver vfio-pci bind

# Imposta dispositivo su driver igb_uio
set device eth0 driver igb_uio

# Rimuove driver dal dispositivo
set device 0000:01:00.0 driver unbound
```

## Panoramica Gestione Dispositivi

### Gestione Dispositivi DPDK
DPDK utilizza driver dedicati per utilizzare efficientemente i dispositivi di rete.

### Tipi di Driver

#### Driver di Rete
- **ixgbe** - Per schede di rete Intel 10GbE
- **igb** - Per schede di rete Intel 1GbE
- **igc** - Per schede di rete Intel 2.5GbE

#### Driver UIO (Userspace I/O)
- **uio_pci_generic** - Driver UIO generico
- **igb_uio** - Driver UIO dedicato DPDK

#### Driver VFIO (Virtual Function I/O)
- **vfio-pci** - I/O ad alte prestazioni per ambienti virtualizzati

#### Configurazione Speciale
- **unbound** - Rimuove driver e disabilita dispositivo

## Procedura di Configurazione Dispositivi

### Procedura di Configurazione Base
1. **Verifica Dispositivi**
```bash
show devices
```

2. **Configurazione Driver**
```bash
set device <device> driver <driver> bind
```

3. **Verifica Configurazione**
```bash
show devices
```

4. **Configurazione Porta**
```bash
show port
```

### Esempi di Configurazione

#### Configurazione Scheda Intel 10GbE
```bash
# Verifica dispositivi
show devices

# Bind driver ixgbe
set device 0000:01:00.0 driver ixgbe bind
set device 0000:01:00.1 driver ixgbe bind

# Verifica configurazione
show devices
show port
```

#### Configurazione Driver DPDK UIO
```bash
# Verifica dispositivi
show devices

# Bind driver igb_uio
set device 0000:02:00.0 driver igb_uio bind
set device 0000:02:00.1 driver igb_uio bind

# Verifica configurazione
show devices
show port
```

#### Configurazione VFIO (Ambiente Virtualizzato)
```bash
# Verifica dispositivi
show devices

# Bind driver vfio-pci
set device 0000:03:00.0 driver vfio-pci bind
set device 0000:03:00.1 driver vfio-pci bind

# Verifica configurazione
show devices
show port
```

## Criteri di Selezione Driver

### ixgbe (Intel 10GbE)
- **Utilizzo**: Schede di rete Intel 10GbE
- **Vantaggi**: Alte prestazioni, stabilità
- **Condizioni**: Richiede scheda Intel 10GbE

### igb (Intel 1GbE)
- **Utilizzo**: Schede di rete Intel 1GbE
- **Vantaggi**: Ampia compatibilità, stabilità
- **Condizioni**: Richiede scheda Intel 1GbE

### igc (Intel 2.5GbE)
- **Utilizzo**: Schede di rete Intel 2.5GbE
- **Vantaggi**: Prestazioni intermedie, nuovo standard
- **Condizioni**: Richiede scheda Intel 2.5GbE

### uio_pci_generic
- **Utilizzo**: Dispositivi generici
- **Vantaggi**: Ampia compatibilità
- **Svantaggi**: Alcune limitazioni funzionali

### igb_uio
- **Utilizzo**: Ambiente dedicato DPDK
- **Vantaggi**: Ottimizzato per DPDK
- **Svantaggi**: Richiede installazione separata

### vfio-pci
- **Utilizzo**: Ambiente virtualizzato, focus sicurezza
- **Vantaggi**: Sicurezza, supporto virtualizzazione
- **Condizioni**: Richiede abilitazione IOMMU

## Risoluzione Problemi

### Quando i Dispositivi Non Vengono Riconosciuti
1. Verifica dispositivi
```bash
show devices
```

2. Verifica a livello sistema
```bash
lspci | grep Ethernet
```

3. Verifica moduli kernel
```bash
lsmod | grep uio
lsmod | grep vfio
```

### Quando il Bind Driver Fallisce
1. Verifica driver attuale
```bash
show devices
```

2. Rimuove driver esistente
```bash
set device <device> driver unbound
```

3. Bind driver target
```bash
set device <device> driver <target_driver> bind
```

### Quando le Porte Non Sono Utilizzabili
1. Verifica stato dispositivo
```bash
show devices
show port
```

2. Re-bind driver
```bash
set device <device> driver unbound
set device <device> driver <driver> bind
```

3. Verifica configurazione porta
```bash
show port
update port status
```

## Funzionalità Avanzate

### Override Driver
```bash
# Utilizzo override driver
set device <device> driver <driver> driver_override
```

### Configurazione Batch Dispositivi Multipli
```bash
# Configurazione sequenziale dispositivi multipli
set device 0000:01:00.0 driver vfio-pci bind
set device 0000:01:00.1 driver vfio-pci bind
set device 0000:02:00.0 driver vfio-pci bind
set device 0000:02:00.1 driver vfio-pci bind
```

## Considerazioni di Sicurezza

### Attenzioni nell'Uso di VFIO
- Necessita abilitazione IOMMU
- Configurazione security group
- Configurazione appropriata dei permessi

### Attenzioni nell'Uso di UIO
- Richiede privilegi root
- Comprensione dei rischi di sicurezza
- Controllo accessi appropriato

## Integrazione Sistema

### Integrazione con servizi systemd
```bash
# Configurazione automatica con servizio systemd
# Configurazione in /etc/systemd/system/sdplane.service
```

### Configurazione Automatica all'Avvio
```bash
# Configurazione con script di avvio
# /etc/init.d/sdplane o systemd unit file
```

## Posizione della Definizione

Questi comandi sono definiti nel seguente file:
- `sdplane/dpdk_devbind.c`

## Argomenti Correlati

- [Gestione Porte e Statistiche](port-management.md)
- [Gestione Worker e lcore](worker-management.md)
- [Informazioni Sistema e Monitoraggio](system-monitoring.md)