# Debug e Log

**Lingua / Language:** [English](../debug-logging.md) | [日本語](../ja/debug-logging.md) | [Français](../fr/debug-logging.md) | [中文](../zh/debug-logging.md) | [Deutsch](../de/debug-logging.md) | **Italiano**

Comandi per controllare le funzionalità di debug e logging di sdplane.

## Lista Comandi

### debug_sdplane - Impostazione Debug sdplane
```
debug sdplane [categoria] [livello]
```

Configura il debug log di sdplane. Questo comando è generato dinamicamente, quindi le categorie e i livelli disponibili sono determinati a runtime.

**Esempio d'uso:**
```bash
# Abilita impostazioni debug
debug sdplane

# Abilita debug per categoria specifica
debug sdplane [category] [level]
```

**Nota:** Le categorie e i livelli specifici possono essere verificati con il comando `show debugging sdplane`.

### show_debug_sdplane - Visualizzazione Informazioni Debug sdplane
```
show debugging sdplane
```

Visualizza le impostazioni di debug attuali di sdplane.

**Esempio d'uso:**
```bash
show debugging sdplane
```

Questo comando visualizza le seguenti informazioni:
- Categorie di debug attualmente abilitate
- Livello di debug per ogni categoria
- Opzioni di debug disponibili

## Panoramica del Sistema di Debug

Il sistema di debug di sdplane ha le seguenti caratteristiche:

### Debug Basato su Categorie
- Le categorie di debug sono separate per diversi moduli funzionali
- È possibile abilitare i log di debug solo per le funzionalità necessarie

### Controllo Basato su Livelli
- I messaggi di debug sono classificati per livelli in base alla loro importanza
- Impostando il livello appropriato, è possibile visualizzare solo le informazioni necessarie

### Configurazione Dinamica
- Le impostazioni di debug possono essere modificate durante il funzionamento del sistema
- È possibile regolare i livelli di debug senza riavvio

## Metodo di Utilizzo

### 1. Verifica Impostazioni Debug Attuali
```bash
show debugging sdplane
```

### 2. Verifica Categorie Debug
Verificare le categorie disponibili con il comando `show debugging sdplane`.

### 3. Modifica Impostazioni Debug
```bash
# Abilita debug per categoria specifica
debug sdplane [category] [level]
```

### 4. Verifica Log Debug
I log di debug vengono emessi su output standard o su file di log.

## Risoluzione Problemi

### Quando i Log Debug Non Vengono Emessi
1. Verificare che la categoria di debug sia impostata correttamente
2. Verificare che il livello di debug sia impostato appropriatamente
3. Verificare che la destinazione di output del log sia impostata correttamente

### Impatto sulle Prestazioni
- L'abilitazione dei log di debug può influire sulle prestazioni
- In ambiente di produzione si raccomanda di abilitare solo il debug minimo necessario

## Posizione della Definizione

Questi comandi sono definiti nel seguente file:
- `sdplane/debug_sdplane.c`

## Argomenti Correlati

- [Informazioni Sistema e Monitoraggio](system-monitoring.md)
- [Gestione VTY e Shell](vty-shell.md)