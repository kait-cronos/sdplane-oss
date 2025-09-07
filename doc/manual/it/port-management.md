# Gestione Porte e Statistiche

**Language:** [English](../en/port-management.md) | [日本語](../ja/port-management.md) | [Français](../fr/port-management.md) | [中文](../zh/port-management.md) | [Deutsch](../de/port-management.md) | **Italiano** | [한국어](../ko/port-management.md) | [ไทย](../th/port-management.md) | [Español](../es/port-management.md)

Comandi per gestire le porte DPDK e le informazioni statistiche.

## Lista Comandi

### **start port**

Avvia le porte DPDK (comportamento predefinito).

**Esempio d'uso:**
```bash
# Avvia porta (predefinito)
start port
```

---

### **start port \<0-16\>**

Avvia una porta DPDK specifica.

**Esempio d'uso:**
```bash
# Avvia porta 0
start port 0

# Avvia porta 1
start port 1
```

---

### **start port all**

Avvia tutte le porte DPDK.

**Esempio d'uso:**
```bash
# Avvia tutte le porte
start port all
```

---

### **stop port**

Arresta le porte DPDK (comportamento predefinito).

**Esempio d'uso:**
```bash
# Arresta porta (predefinito)
stop port
```

---

### **stop port \<0-16\>**

Arresta una porta DPDK specifica.

**Esempio d'uso:**
```bash
# Arresta porta 0
stop port 0

# Arresta porta 1
stop port 1
```

---

### **stop port all**

Arresta tutte le porte DPDK.

**Esempio d'uso:**
```bash
# Arresta tutte le porte
stop port all
```

---

### **reset port**

Resetta le porte DPDK (comportamento predefinito).

**Esempio d'uso:**
```bash
# Resetta porta (predefinito)
reset port
```

---

### **reset port \<0-16\>**

Resetta una porta DPDK specifica.

**Esempio d'uso:**
```bash
# Resetta porta 0
reset port 0

# Resetta porta 1
reset port 1
```

---

### **reset port all**

Resetta tutte le porte DPDK.

**Esempio d'uso:**
```bash
# Resetta tutte le porte
reset port all
```

---

### **show port**

Visualizza le informazioni di base di tutte le porte (comportamento predefinito).

**Esempio d'uso:**
```bash
# Visualizza informazioni di tutte le porte
show port
```

---

### **show port \<0-16\>**

Visualizza le informazioni di base di una porta specifica.

**Esempio d'uso:**
```bash
# Visualizza informazioni della porta 0
show port 0

# Visualizza informazioni della porta 1
show port 1
```

---

### **show port all**

Visualizza esplicitamente le informazioni di tutte le porte.

**Esempio d'uso:**
```bash
# Visualizza esplicitamente informazioni di tutte le porte
show port all
```

---

### **show port statistics**

Visualizza tutte le informazioni statistiche delle porte.

**Esempio d'uso:**
```bash
# Visualizza tutte le statistiche
show port statistics
```

---

### **show port statistics pps**

Visualizza le statistiche pacchetti/secondo.

**Esempio d'uso:**
```bash
# Visualizza solo statistiche PPS
show port statistics pps
```

---

### **show port statistics total**

Visualizza le statistiche del numero totale di pacchetti.

**Esempio d'uso:**
```bash
# Visualizza il numero totale di pacchetti
show port statistics total
```

---

### **show port statistics bps**

Visualizza le statistiche bit/secondo.

**Esempio d'uso:**
```bash
# Visualizza bit/secondo
show port statistics bps
```

---

### **show port statistics Bps**

Visualizza le statistiche byte/secondo.

**Esempio d'uso:**
```bash
# Visualizza byte/secondo
show port statistics Bps
```

---

### **show port statistics total-bytes**

Visualizza le statistiche del numero totale di byte.

**Esempio d'uso:**
```bash
# Visualizza il numero totale di byte
show port statistics total-bytes
```

---

### **show port \<0-16\> promiscuous**

Visualizza lo stato della modalità promiscua della porta specificata.

**Esempio d'uso:**
```bash
# Visualizza modalità promiscua della porta 0
show port 0 promiscuous

# Visualizza modalità promiscua della porta 1
show port 1 promiscuous
```

---

### **show port all promiscuous**

Visualizza lo stato della modalità promiscua di tutte le porte.

**Esempio d'uso:**
```bash
# Visualizza modalità promiscua di tutte le porte
show port all promiscuous
```

---

### **show port \<0-16\> flowcontrol**

Visualizza le impostazioni del controllo di flusso della porta specificata.

**Esempio d'uso:**
```bash
# Visualizza impostazioni controllo di flusso della porta 0
show port 0 flowcontrol

# Visualizza impostazioni controllo di flusso della porta 1
show port 1 flowcontrol
```

---

### **show port all flowcontrol**

Visualizza le impostazioni del controllo di flusso di tutte le porte.

**Esempio d'uso:**
```bash
# Visualizza impostazioni controllo di flusso di tutte le porte
show port all flowcontrol
```

---

### **set port \<0-16\> promiscuous enable**

Abilita la modalità promiscua della porta specificata.

**Esempio d'uso:**
```bash
# Abilita modalità promiscua della porta 0
set port 0 promiscuous enable

# Abilita modalità promiscua della porta 1
set port 1 promiscuous enable
```

---

### **set port \<0-16\> promiscuous disable**

Disabilita la modalità promiscua della porta specificata.

**Esempio d'uso:**
```bash
# Disabilita modalità promiscua della porta 0
set port 0 promiscuous disable

# Disabilita modalità promiscua della porta 1
set port 1 promiscuous disable
```

---

### **set port all promiscuous enable**

Abilita la modalità promiscua di tutte le porte.

**Esempio d'uso:**
```bash
# Abilita modalità promiscua di tutte le porte
set port all promiscuous enable
```

---

### **set port all promiscuous disable**

Disabilita la modalità promiscua di tutte le porte.

**Esempio d'uso:**
```bash
# Disabilita modalità promiscua di tutte le porte
set port all promiscuous disable
```

---

### **set port \<0-16\> flowcontrol (rx|tx|autoneg|send-xon|fwd-mac-ctrl) (on|off)**

Modifica le impostazioni del controllo di flusso della porta specificata.

**Opzioni:**
- `rx` - Controllo di flusso in ricezione
- `tx` - Controllo di flusso in trasmissione
- `autoneg` - Auto-negoziazione
- `send-xon` - Invio XON
- `fwd-mac-ctrl` - Inoltro frame controllo MAC

**Esempio d'uso:**
```bash
# Abilita controllo di flusso in ricezione della porta 0
set port 0 flowcontrol rx on

# Disabilita auto-negoziazione della porta 1
set port 1 flowcontrol autoneg off
```

---

### **set port all flowcontrol (rx|tx|autoneg|send-xon|fwd-mac-ctrl) (on|off)**

Modifica le impostazioni del controllo di flusso di tutte le porte.

**Opzioni:**
- `rx` - Controllo di flusso in ricezione
- `tx` - Controllo di flusso in trasmissione
- `autoneg` - Auto-negoziazione
- `send-xon` - Invio XON
- `fwd-mac-ctrl` - Inoltro frame controllo MAC

**Esempio d'uso:**
```bash
# Disabilita auto-negoziazione di tutte le porte
set port all flowcontrol autoneg off

# Abilita controllo di flusso in trasmissione di tutte le porte
set port all flowcontrol tx on
```

---

### **set port \<0-16\> dev-configure \<0-64\> \<0-64\>**


Configura il dispositivo DPDK della porta specificata.

**Parametri:**
- 1° argomento: Numero di code di ricezione (0-64)
- 2° argomento: Numero di code di trasmissione (0-64)

**Esempio d'uso:**
```bash
# Configura porta 0 con 4 code di ricezione e 4 di trasmissione
set port 0 dev-configure 4 4

# Configura porta 1 con 2 code di ricezione e 2 di trasmissione
set port 1 dev-configure 2 2
```

---

### **set port all dev-configure \<0-64\> \<0-64\>**

Configura i dispositivi DPDK di tutte le porte.

**Parametri:**
- 1° argomento: Numero di code di ricezione (0-64)
- 2° argomento: Numero di code di trasmissione (0-64)

**Esempio d'uso:**
```bash
# Configura tutte le porte con 1 coda di ricezione e 1 di trasmissione
set port all dev-configure 1 1
```

---

### **set port \<0-16\> nrxdesc \<0-16384\>**


Imposta il numero di descrittori di ricezione della porta specificata.

**Esempio d'uso:**
```bash
# Imposta il numero di descrittori di ricezione della porta 0 a 1024
set port 0 nrxdesc 1024

# Imposta il numero di descrittori di ricezione della porta 1 a 512
set port 1 nrxdesc 512
```

---

### **set port all nrxdesc \<0-16384\>**

Imposta il numero di descrittori di ricezione di tutte le porte.

**Esempio d'uso:**
```bash
# Imposta il numero di descrittori di ricezione di tutte le porte a 512
set port all nrxdesc 512
```

---

### **set port \<0-16\> ntxdesc \<0-16384\>**


Imposta il numero di descrittori di trasmissione della porta specificata.

**Esempio d'uso:**
```bash
# Imposta il numero di descrittori di trasmissione della porta 0 a 1024
set port 0 ntxdesc 1024

# Imposta il numero di descrittori di trasmissione della porta 1 a 512
set port 1 ntxdesc 512
```

---

### **set port all ntxdesc \<0-16384\>**

Imposta il numero di descrittori di trasmissione di tutte le porte.

**Esempio d'uso:**
```bash
# Imposta il numero di descrittori di trasmissione di tutte le porte a 512
set port all ntxdesc 512
```

---

### **set port \<0-16\> link up**


Attiva il link della porta specificata.

**Esempio d'uso:**
```bash
# Attiva link della porta 0
set port 0 link up

# Attiva link della porta 1
set port 1 link up
```

---

### **set port \<0-16\> link down**

Disattiva il link della porta specificata.

**Esempio d'uso:**
```bash
# Disattiva link della porta 0
set port 0 link down

# Disattiva link della porta 1
set port 1 link down
```

---

### **set port all link up**

Attiva il link di tutte le porte.

**Esempio d'uso:**
```bash
# Attiva link di tutte le porte
set port all link up
```

---

### **set port all link down**

Disattiva il link di tutte le porte.

**Esempio d'uso:**
```bash
# Disattiva link di tutte le porte
set port all link down
```

## Posizione della Definizione

Questi comandi sono definiti nel seguente file:
- `sdplane/dpdk_port_cmd.c`

## Argomenti Correlati

- [Gestione Worker e lcore](worker-lcore-thread-management.md)
- [Informazioni Sistema e Monitoraggio](system-monitoring.md)
- [Configurazione Code](queue-configuration.md)