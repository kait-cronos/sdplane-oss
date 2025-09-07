# Configurazione Ripetitore Avanzato

**Language:** [English](../en/enhanced-repeater.md) | [日本語](../ja/enhanced-repeater.md) | [Français](../fr/enhanced-repeater.md) | [中文](../zh/enhanced-repeater.md) | [Deutsch](../de/enhanced-repeater.md) | **Italiano** | [한국어](../ko/enhanced-repeater.md) | [ไทย](../th/enhanced-repeater.md) | [Español](../es/enhanced-repeater.md)

Il Ripetitore Avanzato fornisce funzionalità di switching VLAN avanzate con interfacce TAP per routing L3 e cattura pacchetti. Supporta la creazione di switch virtuali, il collegamento di porte DPDK con tagging VLAN, interfacce router per l'integrazione kernel e interfacce di cattura per il monitoraggio.

## Comandi Switch Virtuali

### set vswitch
```
set vswitch <1-4094>
```
**Descrizione**: Crea uno switch virtuale con l'ID VLAN specificato

**Parametri**:
- `<1-4094>`: ID VLAN dello switch virtuale

**Esempi**:
```
set vswitch 2031
set vswitch 2032
```

### delete vswitch
```
delete vswitch <0-3>
```
**Descrizione**: Elimina lo switch virtuale specificato per ID

**Parametri**:
- `<0-3>`: ID dello switch virtuale da eliminare

**Esempio**:
```
delete vswitch 0
```

### show vswitch_rib
```
show vswitch_rib
```
**Descrizione**: Visualizza informazioni RIB dello switch virtuale incluse configurazione e stato

## Comandi Collegamento Switch Virtuali

### set vswitch-link
```
set vswitch-link vswitch <0-3> port <0-7> tag <0-4094>
```
**Descrizione**: Collega una porta DPDK a uno switch virtuale con configurazione tagging VLAN

**Parametri**:
- `vswitch <0-3>`: ID switch virtuale (0-3)
- `port <0-7>`: ID porta DPDK (0-7)  
- `tag <0-4094>`: ID tag VLAN (0: nativo/untagged, 1-4094: VLAN tagged)

**Esempi**:
```
# Collega porta 0 a switch virtuale 0 con tag VLAN 2031
set vswitch-link vswitch 0 port 0 tag 2031

# Collega porta 0 a switch virtuale 1 come nativo/untagged
set vswitch-link vswitch 1 port 0 tag 0
```

### delete vswitch-link
```
delete vswitch-link <0-7>
```
**Descrizione**: Elimina il collegamento switch virtuale specificato per ID

**Parametri**:
- `<0-7>`: ID collegamento switch virtuale

### show vswitch-link
```
show vswitch-link
```
**Descrizione**: Visualizza tutte le configurazioni collegamenti switch virtuali

## Comandi Interfacce Router

### set vswitch router-if
```
set vswitch <1-4094> router-if <WORD>
```
**Descrizione**: Crea un'interfaccia router per connettività L3 nello switch virtuale specificato

**Parametri**:
- `<1-4094>`: ID VLAN dello switch virtuale
- `<WORD>`: Nome interfaccia TAP

**Esempi**:
```
set vswitch 2031 router-if rif2031
set vswitch 2032 router-if rif2032
```

### no set vswitch router-if
```
no set vswitch <1-4094> router-if
```
**Descrizione**: Rimuove l'interfaccia router dallo switch virtuale specificato

**Parametri**:
- `<1-4094>`: ID VLAN dello switch virtuale

### show rib vswitch router-if
```
show rib vswitch router-if
```
**Descrizione**: Visualizza configurazioni interfacce router inclusi indirizzo MAC, indirizzo IP e stato interfaccia

## Comandi Interfacce Cattura

### set vswitch capture-if
```
set vswitch <1-4094> capture-if <WORD>
```
**Descrizione**: Crea un'interfaccia di cattura per monitoraggio pacchetti nello switch virtuale specificato

**Parametri**:
- `<1-4094>`: ID VLAN dello switch virtuale
- `<WORD>`: Nome interfaccia TAP

**Esempi**:
```
set vswitch 2031 capture-if cif2031
set vswitch 2032 capture-if cif2032
```

### no set vswitch capture-if
```
no set vswitch <1-4094> capture-if
```
**Descrizione**: Rimuove l'interfaccia di cattura dallo switch virtuale specificato

**Parametri**:
- `<1-4094>`: ID VLAN dello switch virtuale

### show rib vswitch capture-if
```
show rib vswitch capture-if
```
**Descrizione**: Visualizza configurazioni interfacce di cattura

## Funzionalità Elaborazione VLAN

Il Ripetitore Avanzato esegue elaborazione VLAN avanzata:

- **Traduzione VLAN**: Modifica ID VLAN basata su configurazioni vswitch-link
- **Rimozione VLAN**: Rimuove header VLAN quando il tag è impostato a 0 (nativo)  
- **Inserimento VLAN**: Aggiunge header VLAN quando forwarding pacchetti untagged a porte tagged
- **Split Horizon**: Previene loop non rimandando pacchetti alla porta di ricezione

## Esempio di Configurazione

```bash
# Crea switch virtuali
set vswitch 2031
set vswitch 2032

# Collega porte DPDK a switch virtuali con tag VLAN
set vswitch-link vswitch 0 port 0 tag 2031
set vswitch-link vswitch 1 port 0 tag 2032

# Crea interfacce router per elaborazione L3
set vswitch 2031 router-if rif2031
set vswitch 2032 router-if rif2032

# Crea interfacce cattura per monitoraggio
set vswitch 2031 capture-if cif2031
set vswitch 2032 capture-if cif2032

# Configura worker per usare ripetitore avanzato
set worker lcore 1 enhanced-repeater
```

## Integrazione Interfacce TAP

Le interfacce router e cattura creano interfacce TAP integrate con lo stack di rete del kernel Linux:

- **Interfacce Router**: Abilitano routing L3, indirizzamento IP e elaborazione di rete kernel
- **Interfacce Cattura**: Abilitano monitoraggio, analisi e debug pacchetti
- **Buffer Ring**: Utilizzano ring DPDK per trasferimento efficiente pacchetti tra data plane e kernel

## Posizione della Definizione

Questi comandi sono definiti nel seguente file:
- `sdplane/enhanced_repeater_cmd.c`

## Argomenti Correlati

- [Applicazione Ripetitore Avanzato](enhanced-repeater-application.md)
- [Gestione Worker e lcore](worker-lcore-thread-management.md)
- [Interfaccia TAP](tap-interface.md)