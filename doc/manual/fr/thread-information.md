# Information des Threads

**Language:** [English](../thread-information.md) | [日本語](../ja/thread-information.md) | **Français** | [中文](../zh/thread-information.md)

Surveillance et informations détaillées sur les threads et leur performance.

## Commandes de Thread

### show_thread_information - Information Thread
```
show thread information
```

Affiche les informations détaillées sur tous les threads actifs dans le système.

### show_thread_counter - Compteurs Thread
```
show thread counter
```

Affiche les compteurs de performance et statistiques pour chaque thread.

## Informations Thread

### Métriques de Base
Les informations thread incluent :
- **ID Thread** : Identifiant unique thread
- **lcore Affecté** : Cœur CPU assigné au thread
- **Type Worker** : Type de worker exécuté (l2-repeater, enhanced-repeater, etc.)
- **État** : Statut actuel thread (running, stopped, error)
- **Compteur Boucle** : Nombre itérations boucle principale

### Données Performance
```bash
# Afficher performance détaillée threads
show thread counter

# Informations incluent :
# - Compteurs boucle par thread
# - Taux traitement paquets
# - Cycles CPU utilisés
# - Temps exécution
```

## Architecture Threading

### Modèle Threading sdplane
sdplane utilise modèle threading coopératif basé sur lthread :
- **Thread Principal** : Gestion système et CLI
- **lthreads Système** : stat-collector, rib-manager, neigh-manager
- **Workers lcore** : Traitement paquets haute performance
- **Threads TAP** : Gestion interfaces TAP

### Types de Threads

#### Threads Système (lthread)
```bash
# Threads coopératifs pour services système
set worker lthread stat-collector    # Collecte statistiques
set worker lthread rib-manager       # Gestion RIB
set worker lthread neigh-manager     # Gestion voisinage
set worker lthread netlink-thread    # Communication netlink
```

#### Workers lcore  
```bash
# Workers dédiés sur lcores spécifiques
set worker lcore 1 l2-repeater      # Transmission L2
set worker lcore 2 enhanced-repeater # Commutation VLAN
set worker lcore 3 pktgen           # Génération paquets
set worker lcore 4 l3-tap-handler   # Traitement TAP L3
```

## Surveillance Performance

### Compteurs Boucle
Chaque worker maintient compteur boucle :
- **Incrémenté** à chaque itération boucle principale
- **Surveillance** pour détecter threads bloqués
- **Performance** indicateur débit traitement

### Exemple Implémentation
```c
static __thread uint64_t loop_counter = 0;

void my_worker_function(void) {
    // Enregistrer compteur pour surveillance
    int thread_id = thread_lookup_by_lcore(my_worker_function, lcore_id);
    thread_register_loop_counter(thread_id, &loop_counter);
    
    while (1) {
        loop_counter++;  // Incrément chaque itération
        
        // Traitement worker...
        process_packets();
    }
}
```

### Métriques Dérivées
À partir des compteurs de base :
- **Taux Boucles/Seconde** : Fréquence exécution boucle
- **Paquets/Boucle** : Efficacité traitement
- **Temps/Boucle** : Latence moyenne traitement
- **Utilisation CPU** : Pourcentage cycles utilisés

## Analyse Performance

### Identification Goulots
```bash
# Surveiller compteurs en temps réel
watch -n 1 "show thread counter"

# Identifier threads avec compteurs bas/stables
# = possibles goulots ou blocages

# Corréler avec statistiques ports
show port statistics all
```

### Optimisation Basée Métriques
1. **Threads Lents** : Redistribuer charge ou optimiser code
2. **Déséquilibres** : Réaffecter workers entre lcores  
3. **Blocages** : Identifier et résoudre contentions ressources
4. **Surcharge** : Ajouter workers ou ajuster configuration

## Surveillance Continue

### Monitoring Automatique
```bash
#!/bin/bash
# Script surveillance threads

while true; do
    echo "=== Thread Counters $(date) ==="
    show thread counter
    echo "=== Thread Information ==="
    show thread information
    echo ""
    sleep 10
done
```

### Alertes Performance
- **Compteur Stagnant** : Thread possiblement bloqué
- **Taux Bas** : Performance sous-optimale
- **Variations Importantes** : Instabilité performance
- **Erreurs Thread** : Problèmes critiques

## Debug Thread

### Commandes Debug
```bash
# Debug workers spécifiques
debug sdplane worker

# Informations détaillées threads
show thread information

# Performance en temps réel  
show thread counter
```

### Diagnostic Problèmes
1. **Thread Non Démarré** :
   ```bash
   show thread information  # Vérifier état
   show worker             # Confirmer configuration
   ```

2. **Performance Dégradée** :
   ```bash
   show thread counter     # Compteurs boucle
   show port statistics all # Corrélation trafic
   ```

3. **Blocage Suspect** :
   ```bash
   debug sdplane worker    # Logs détaillés
   show system            # Charge système
   ```

## Optimisation Thread

### Affectation lcore
```bash
# Optimiser affectation workers
set worker lcore 1 enhanced-repeater  # lcore dédié
set worker lcore 2 pktgen            # Isolation génération
set worker lcore 3 l3-tap-handler    # TAP séparé
```

### Équilibrage Charge
```bash
# Distribuer files entre threads
set thread 1 port 0 queue 0
set thread 2 port 1 queue 0
set thread 3 port 2 queue 0

# Éviter surcharge single thread
```

### Affinité CPU
- Assigner lcores spécifiques pour applications critiques
- Éviter migration threads entre cœurs
- Utiliser affinité NUMA pour optimisation mémoire
- Isoler lcores des interruptions système

## Modèles Performance

### Applications Traitement Paquets
Compteurs élevés et stables :
```
l2-repeater:     1,234,567 loops/sec
enhanced-repeater: 987,654 loops/sec  
pktgen:         2,345,678 loops/sec
```

### Services Système
Compteurs plus variables :
```
stat-collector:    1,000 loops/sec
rib-manager:       5,000 loops/sec
neigh-manager:     2,000 loops/sec
```

### Détection Anomalies
- **Compteur = 0** : Thread arrêté ou bloqué
- **Variations Extrêmes** : Instabilité ou contention
- **Déclin Progressif** : Dégradation performance
- **Pics Soudains** : Événements système

## Corrélation Métriques

### Thread vs Port
```bash
# Corréler performance thread avec trafic port
show thread counter
show port statistics all

# Threads traitant plus paquets = compteurs plus élevés
```

### Thread vs Système
```bash
# Performance thread vs ressources système
show thread counter
show memory
show lcore

# Corrélation charge CPU et compteurs threads
```

## Historique Performance

### Collecte Données
```bash
# Log périodique compteurs
while true; do
    date >> thread-performance.log
    show thread counter >> thread-performance.log
    sleep 60
done
```

### Analyse Tendances
- Évolution compteurs dans temps
- Identification dégradations graduelles  
- Corrélation avec événements système
- Planification capacité basée historique

## Emplacement de Définition

Information threads définie dans :
- `sdplane/thread_cmd.c` - Commandes information threads
- `sdplane/lthread_main.c` - Gestion threads principaux
- `sdplane/worker_*.c` - Implémentations workers spécifiques

## Sujets Associés

- [Gestion Workers](worker-management.md) - Configuration workers et lcores
- [Surveillance Système](system-monitoring.md) - Métriques système globales
- [RIB & Routage](routing.md) - Configuration threads dans RIB
- [Debug & Logging](debug-logging.md) - Debug performance threads