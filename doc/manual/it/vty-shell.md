# Gestione VTY e Shell

**Lingua / Language:** [English](../vty-shell.md) | [日本語](../ja/vty-shell.md) | [Français](../fr/vty-shell.md) | [中文](../zh/vty-shell.md) | [Deutsch](../de/vty-shell.md) | **Italiano**

Comandi per gestire VTY (Virtual Terminal) e shell.

## Lista Comandi

### clear_cmd - Pulizia Schermo
```
clear
```

Pulisce lo schermo del VTY.

**Esempio d'uso:**
```bash
clear
```

### vty_exit_cmd - Uscita VTY
```
(exit|quit|logout)
```

Termina la sessione VTY. Sono disponibili più alias.

**Esempio d'uso:**
```bash
exit
# oppure
quit
# oppure
logout
```

### shutdown_cmd - Spegnimento Sistema
```
shutdown
```

Spegne il sistema sdplane.

**Esempio d'uso:**
```bash
shutdown
```

**Attenzione:** Questo comando arresta l'intero sistema. Prima dell'esecuzione, verificare il salvataggio delle configurazioni e la chiusura delle sessioni attive.

### exit_cmd - Uscita Console
```
(exit|quit)
```

Esce dalla shell della console.

**Esempio d'uso:**
```bash
exit
# oppure
quit
```

## Differenza tra VTY e Console

### Shell VTY
- Shell remota accessibile tramite Telnet
- Supporta multiple sessioni simultanee
- Accessibile tramite rete

### Shell Console
- Accesso da console locale
- Accesso diretto al sistema
- Principalmente per gestione locale

## Metodo di Connessione

### Connessione al VTY
```bash
telnet localhost 9882
```

### Connessione alla Console
```bash
# Eseguire sdplane direttamente
sudo ./sdplane/sdplane
```

## Gestione Sessioni

### Verifica Sessioni
Lo stato delle sessioni VTY può essere verificato con il seguente comando:
```bash
show worker
```

### Chiusura Sessioni
- Chiusura normale con comandi `exit`, `quit`, `logout`
- In caso di chiusura anomala, arrestare l'intero sistema con il comando `shutdown`

## Considerazioni di Sicurezza

### Controllo Accessi
- VTY permette l'accesso solo da localhost (127.0.0.1) per impostazione predefinita
- Si raccomanda di limitare l'accesso alla porta 9882 con le impostazioni del firewall

### Monitoraggio Sessioni
- Chiudere appropriatamente le sessioni non necessarie
- Verificare periodicamente le sessioni inattive per lungo tempo

## Risoluzione Problemi

### Impossibilità di Connettersi al VTY
1. Verificare che sdplane sia avviato correttamente
2. Verificare che la porta 9882 sia disponibile
3. Controllare le impostazioni del firewall

### Sessione Non Risponde
1. Verificare lo stato con `show worker` da un'altra sessione VTY
2. Se necessario, riavviare il sistema con il comando `shutdown`

### Comando Non Riconosciuto
1. Verificare di essere nella shell corretta (VTY o console)
2. Visualizzare l'aiuto con `?` per verificare i comandi disponibili

## Posizione della Definizione

Questi comandi sono definiti nei seguenti file:
- `sdplane/vty_shell.c` - Comandi relativi a VTY
- `sdplane/console_shell.c` - Comandi relativi alla console

## Argomenti Correlati

- [Debug e Log](debug-logging.md)
- [Informazioni Sistema e Monitoraggio](system-monitoring.md)