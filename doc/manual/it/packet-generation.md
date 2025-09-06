# Generazione Pacchetti

**Lingua / Language:** [English](../packet-generation.md) | [日本語](../ja/packet-generation.md) | [Français](../fr/packet-generation.md) | [中文](../zh/packet-generation.md) | [Deutsch](../de/packet-generation.md) | **Italiano** | [한국어](../ko/packet-generation.md) | [ไทย](../th/packet-generation.md) | [Español](../es/packet-generation.md)

Comandi per la funzionalità di generazione pacchetti usando PKTGEN (Packet Generator).

## Lista Comandi

### show_pktgen - Visualizzazione Informazioni PKTGEN
```
show pktgen
```

Visualizza lo stato e la configurazione attuali di PKTGEN (Packet Generator).

**Esempio d'uso:**
```bash
show pktgen
```

Questo comando visualizza le seguenti informazioni:
- Stato di inizializzazione PKTGEN
- Parametri di configurazione attuali
- Task in esecuzione
- Informazioni statistiche

### pktgen_init - Inizializzazione PKTGEN
```
pktgen init argv-list <0-7>
```

Inizializza PKTGEN usando l'argv-list specificato.

**Parametri:**
- `<0-7>` - Indice dell'argv-list da utilizzare

**Esempio d'uso:**
```bash
# Inizializza PKTGEN usando argv-list 0
pktgen init argv-list 0

# Inizializza PKTGEN usando argv-list 2
pktgen init argv-list 2
```

### pktgen_do_start - Avvio PKTGEN
```
pktgen do start (<0-7>|all)
```

Avvia la generazione pacchetti sulla porta specificata.

**Target:**
- `<0-7>` - Numero porta specifica
- `all` - Tutte le porte

**Esempio d'uso:**
```bash
# Avvia generazione pacchetti sulla porta 0
pktgen do start 0

# Avvia generazione pacchetti su tutte le porte
pktgen do start all
```

### pktgen_do_stop - Arresto PKTGEN
```
pktgen do stop (<0-7>|all)
```

Arresta la generazione pacchetti sulla porta specificata.

**Target:**
- `<0-7>` - Numero porta specifica
- `all` - Tutte le porte

**Esempio d'uso:**
```bash
# Arresta generazione pacchetti sulla porta 1
pktgen do stop 1

# Arresta generazione pacchetti su tutte le porte
pktgen do stop all
```

## Panoramica PKTGEN

### Cos'è PKTGEN
PKTGEN (Packet Generator) è uno strumento di generazione pacchetti per test di rete. Fornisce le seguenti funzionalità:

- **Generazione Pacchetti Ad Alte Prestazioni** - Generazione pacchetti ad alte prestazioni
- **Formati Pacchetti Diversi** - Supporto per vari protocolli
- **Configurazione Flessibile** - Configurazione dettagliata dei pacchetti possibile
- **Funzionalità Statistiche** - Fornisce informazioni statistiche dettagliate

### Utilizzi Principali
- **Test Prestazioni Rete** - Misurazione throughput e latenza
- **Test di Carico** - Test resistenza al carico del sistema
- **Test Funzionali** - Verifica funzionalità di rete
- **Benchmark** - Test di confronto prestazioni

## Configurazione PKTGEN

### Procedura di Configurazione Base
1. **Configurazione argv-list**
```bash
# Imposta parametri per PKTGEN
set argv-list 0 "-c 0x3 -n 4"
set argv-list 1 "--socket-mem 1024"
set argv-list 2 "--huge-dir /mnt/huge"
```

2. **Inizializzazione PKTGEN**
```bash
pktgen init argv-list 0
```

3. **Configurazione Worker**
```bash
set worker lcore 1 pktgen
start worker lcore 1
```

4. **Avvio Generazione Pacchetti**
```bash
pktgen do start 0
```

### Parametri di Configurazione
Esempi di parametri configurabili con argv-list:

- **-c** - Maschera CPU
- **-n** - Numero canali memoria
- **--socket-mem** - Dimensione memoria socket
- **--huge-dir** - Directory hugepage
- **--file-prefix** - Prefisso file

## Esempi d'Uso

### Generazione Pacchetti Base
```bash
# Configurazione
set argv-list 0 "-c 0x3 -n 4 --socket-mem 1024"

# Inizializzazione
pktgen init argv-list 0

# Configurazione worker
set worker lcore 1 pktgen
start worker lcore 1

# Avvio generazione pacchetti
pktgen do start 0

# Verifica stato
show pktgen

# Arresto generazione pacchetti
pktgen do stop 0
```

### Generazione su Porte Multiple
```bash
# Avvio su porte multiple
pktgen do start all

# Verifica stato
show pktgen

# Arresto su porte multiple
pktgen do stop all
```

## Monitoraggio e Statistiche

### Verifica Informazioni Statistiche
```bash
# Visualizzazione statistiche PKTGEN
show pktgen

# Visualizzazione statistiche porte
show port statistics

# Visualizzazione statistiche worker
show worker
```

### Monitoraggio Prestazioni
```bash
# Verifica PPS (Packets Per Second)
show port statistics pps

# Verifica numero totale pacchetti
show port statistics total

# Verifica byte/secondo
show port statistics Bps
```

## Risoluzione Problemi

### Quando PKTGEN Non Si Avvia
1. Verifica stato inizializzazione
```bash
show pktgen
```

2. Verifica stato worker
```bash
show worker
```

3. Verifica stato porta
```bash
show port
```

### Quando la Generazione Pacchetti Non Si Arresta
1. Arresta esplicitamente
```bash
pktgen do stop all
```

2. Riavvia worker
```bash
restart worker lcore 1
```

### Quando le Prestazioni Sono Basse
1. Verifica utilizzo CPU
2. Verifica configurazione memoria
3. Verifica configurazione porta

## Funzionalità Avanzate

### Configurazione Formati Pacchetti
PKTGEN può generare vari formati di pacchetti:
- **Ethernet** - Frame Ethernet base
- **IP** - Pacchetti IPv4/IPv6
- **UDP/TCP** - Pacchetti UDP/TCP
- **VLAN** - Pacchetti con tag VLAN

### Controllo Carico
- **Controllo Rate** - Controllo rate generazione pacchetti
- **Controllo Burst** - Generazione pacchetti burst
- **Controllo Dimensione** - Controllo dimensione pacchetti

## Posizione della Definizione

Questi comandi sono definiti nel seguente file:
- `sdplane/pktgen_cmd.c`

## Argomenti Correlati

- [Gestione Worker e lcore](worker-management.md)
- [Gestione Porte e Statistiche](port-management.md)
- [Informazioni Sistema e Monitoraggio](system-monitoring.md)