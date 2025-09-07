# Interfaccia TAP

**Language:** [English](../en/tap-interface.md) | [日本語](../ja/tap-interface.md) | [Français](../fr/tap-interface.md) | [中文](../zh/tap-interface.md) | [Deutsch](../de/tap-interface.md) | **Italiano** | [한국어](../ko/tap-interface.md) | [ไทย](../th/tap-interface.md) | [Español](../es/tap-interface.md)

Comandi per gestire le interfacce TAP.

## Lista Comandi

### set_tap_capture_ifname - Configurazione Nome Interfaccia Cattura TAP
```
set tap capture ifname <WORD>
```

Imposta il nome dell'interfaccia da utilizzare per la funzionalità di cattura TAP.

**Parametri:**
- `<WORD>` - Nome interfaccia

**Esempio d'uso:**
```bash
# Configura interfaccia tap0
set tap capture ifname tap0

# Configura interfaccia tap1
set tap capture ifname tap1
```

### set_tap_capture_persistent - Abilitazione Persistenza Cattura TAP
```
set tap capture persistent
```

Abilita la persistenza della cattura TAP.

**Esempio d'uso:**
```bash
# Abilita persistenza
set tap capture persistent
```

### no_tap_capture_persistent - Disabilitazione Persistenza Cattura TAP
```
no tap capture persistent
```

Disabilita la persistenza della cattura TAP.

**Esempio d'uso:**
```bash
# Disabilita persistenza
no tap capture persistent
```

### unset_tap_capture_persistent - Rimozione Configurazione Persistenza Cattura TAP
```
unset tap capture persistent
```

Rimuove la configurazione di persistenza della cattura TAP.

**Esempio d'uso:**
```bash
# Rimuove configurazione persistenza
unset tap capture persistent
```

## Panoramica Interfaccia TAP

### Cos'è l'Interfaccia TAP
L'interfaccia TAP (Network TAP) è un'interfaccia di rete virtuale utilizzata per il monitoraggio e test del traffico di rete.

### Funzionalità Principali
- **Cattura Pacchetti** - Cattura del traffico di rete
- **Iniezione Pacchetti** - Iniezione di pacchetti di test
- **Funzionalità Bridge** - Bridge tra diverse reti
- **Funzionalità Monitoraggio** - Monitoraggio e analisi del traffico

### Utilizzi in sdplane
- **Debug** - Debug del flusso pacchetti
- **Test** - Test delle funzionalità di rete
- **Monitoraggio** - Monitoraggio del traffico
- **Sviluppo** - Sviluppo e test di nuove funzionalità

## Configurazione Interfaccia TAP

### Procedura di Configurazione Base
1. **Creazione Interfaccia TAP**
```bash
# Crea interfaccia TAP a livello sistema
sudo ip tuntap add tap0 mode tap
sudo ip link set tap0 up
```

2. **Configurazione in sdplane**
```bash
# Imposta nome interfaccia cattura TAP
set tap capture ifname tap0

# Abilita persistenza
set tap capture persistent
```

3. **Configurazione Worker Gestore TAP**
```bash
# Configura worker gestore TAP
set worker lcore 2 tap-handler
start worker lcore 2
```

### Esempi di Configurazione

#### Configurazione TAP Base
```bash
# Configurazione interfaccia TAP
set tap capture ifname tap0
set tap capture persistent

# Configurazione worker
set worker lcore 2 tap-handler
start worker lcore 2

# Verifica configurazione
show worker
```

#### Configurazione Interfacce TAP Multiple
```bash
# Configura interfacce TAP multiple
set tap capture ifname tap0
set tap capture ifname tap1

# Abilita persistenza
set tap capture persistent
```

## Funzionalità Persistenza

### Cos'è la Persistenza
Abilitando la funzionalità di persistenza, le configurazioni dell'interfaccia TAP vengono mantenute anche dopo il riavvio del sistema.

### Vantaggi della Persistenza
- **Mantenimento Configurazione** - Configurazione efficace anche dopo il riavvio
- **Ripristino Automatico** - Ripristino automatico da guasti del sistema
- **Efficienza Operativa** - Riduzione della configurazione manuale

### Configurazione Persistenza
```bash
# Abilita persistenza
set tap capture persistent

# Disabilita persistenza
no tap capture persistent

# Rimuove configurazione persistenza
unset tap capture persistent
```

## Esempi d'Uso

### Utilizzo per Debug
```bash
# Configurazione interfaccia TAP per debug
set tap capture ifname debug-tap
set tap capture persistent

# Configura worker gestore TAP
set worker lcore 3 tap-handler
start worker lcore 3

# Avvia cattura pacchetti
tcpdump -i debug-tap
```

### Utilizzo per Test
```bash
# Configurazione interfaccia TAP per test
set tap capture ifname test-tap
set tap capture persistent

# Preparazione iniezione pacchetti di test
set worker lcore 4 tap-handler
start worker lcore 4
```

## Monitoraggio e Gestione

### Verifica Stato Interfaccia TAP
```bash
# Verifica stato worker
show worker

# Verifica informazioni thread
show thread

# Verifica a livello sistema
ip link show tap0
```

### Monitoraggio Traffico
```bash
# Monitoraggio usando tcpdump
tcpdump -i tap0

# Monitoraggio usando Wireshark
wireshark -i tap0
```

## Risoluzione Problemi

### Quando l'Interfaccia TAP Non Viene Creata
1. Verifica a livello sistema
```bash
# Verifica esistenza interfaccia TAP
ip link show tap0

# Verifica permessi
sudo ip tuntap add tap0 mode tap
```

2. Verifica in sdplane
```bash
# Verifica configurazione
show worker

# Verifica stato worker
show thread
```

### Quando i Pacchetti Non Vengono Catturati
1. Verifica stato interfaccia
```bash
ip link show tap0
```

2. Verifica stato worker
```bash
show worker
```

3. Riavvia gestore TAP
```bash
restart worker lcore 2
```

### Quando la Persistenza Non Funziona
1. Verifica configurazione persistenza
```bash
# Verifica configurazione attuale (verifica con comandi show)
show worker
```

2. Verifica configurazione sistema
```bash
# Verifica configurazione a livello sistema
systemctl status sdplane
```

## Funzionalità Avanzate

### Integrazione con VLAN
L'interfaccia TAP può essere utilizzata in integrazione con la funzionalità VLAN:
```bash
# Integrazione con worker switch VLAN
set worker lcore 5 vlan-switch
start worker lcore 5
```

### Funzionalità Bridge
Utilizzo con bridge di interfacce TAP multiple:
```bash
# Configurazione interfacce TAP multiple
set tap capture ifname tap0
set tap capture ifname tap1
```

## Posizione della Definizione

Questi comandi sono definiti nel seguente file:
- `sdplane/tap_cmd.c`

## Argomenti Correlati

- [Gestione Worker e lcore](worker-lcore-thread-management.md)
- [Gestione VTY e Shell](vty-shell.md)
- [Debug e Log](debug-logging.md)