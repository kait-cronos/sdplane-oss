# Gestion des Workers & lcores

**Language:** [English](../worker-lcore-thread-management.md) | [日本語](../ja/worker-lcore-thread-management.md) | **Français** | [中文](../zh/worker-lcore-thread-management.md) | [Deutsch](../de/worker-lcore-thread-management.md) | [Italiano](../it/worker-lcore-thread-management.md) | [한국어](../ko/worker-lcore-thread-management.md) | [ไทย](../th/worker-lcore-thread-management.md) | [Español](../es/worker-lcore-thread-management.md)

Commandes pour la gestion des threads workers et la configuration des lcores.

## Liste des Commandes

### set_worker - Définir Worker
```
set worker <lcore|lthread> <id> <type>
```

Affecte un type de worker spécifique à un lcore ou lthread.

**Paramètres :**
- `lcore <id>` : ID lcore (0-7)
- `lthread <type>` : Type lthread spécifique
- `<type>` : Type de worker (voir types disponibles ci-dessous)

**Exemples :**
```bash
set worker lcore 1 l2-repeater
set worker lcore 2 enhanced-repeater
set worker lthread stat-collector
set worker lthread rib-manager
```

### start_worker - Démarrer Worker
```
start worker lcore <id|all>
```

Démarre les workers sur lcores spécifiés.

**Exemples :**
```bash
start worker lcore 1
start worker lcore all
```

### show_worker - Afficher Workers
```
show worker
```

Affiche l'état actuel et la configuration de tous les workers.

### set_thread - Configuration Thread
```
set thread <id> port <port-id> queue <queue-id>
```

Affecte une file de port spécifique à un thread.

**Paramètres :**
- `<id>` : ID thread
- `<port-id>` : ID port DPDK
- `<queue-id>` : ID file sur le port

**Exemple :**
```bash
set thread 1 port 0 queue 0
set thread 2 port 1 queue 0
```

## Types de Workers

### Workers d'Application
- **l2-repeater** : Transmission simple Layer 2
- **enhanced-repeater** : Commutation VLAN avancée
- **l3fwd** : Transmission Layer 3
- **pktgen** : Génération de paquets

### Workers Système
- **stat-collector** : Collecte de statistiques système
- **rib-manager** : Gestion base d'informations routage
- **neigh-manager** : Gestion table de voisinage
- **netlink-thread** : Communication netlink Linux
- **tap-handler** : Gestion interface TAP
- **l3-tap-handler** : Gestion TAP Layer 3

## Configuration des Workers

### Ordre de Configuration Recommandé
1. **Workers système d'abord** :
   ```bash
   set worker lthread stat-collector
   set worker lthread rib-manager
   set worker lthread neigh-manager
   set worker lthread netlink-thread
   ```

2. **Configuration pools mémoire** :
   ```bash
   set mempool
   ```

3. **Configuration affectations threads** :
   ```bash
   set thread 1 port 0 queue 0
   set thread 2 port 1 queue 0
   ```

4. **Workers d'application** :
   ```bash
   set worker lcore 1 l2-repeater
   set worker lcore 2 enhanced-repeater
   ```

5. **Démarrage workers** :
   ```bash
   start worker lcore all
   ```

### Exemple de Configuration Complète
```bash
# Workers système (doivent être démarrés en premier)
set worker lthread stat-collector
set worker lthread rib-manager
set worker lthread neigh-manager
set worker lthread netlink-thread

# Configuration pools mémoire
set mempool

# Configuration affectations port-file-thread
set thread 1 port 0 queue 0
set thread 1 port 1 queue 0
set thread 2 port 2 queue 0

# Workers d'application
set worker lcore 1 l2-repeater
set worker lcore 2 enhanced-repeater
set worker lcore 3 l3-tap-handler

# Démarrer workers
start worker lcore all
```

## Gestion des lcores

### Allocation lcore
- **lcore 0** : Réservé pour thread principal
- **lcore 1-N** : Disponible pour workers d'application
- **lthreads** : Threads coopératifs pour tâches système

### Bonnes Pratiques d'Affectation
- Dédier lcores séparés pour applications haute performance
- Grouper workers liés sur lcores adjacents
- Considérer topologie NUMA pour optimisation

### Optimisation Performance
```bash
# Dédier lcores pour applications spécifiques
set worker lcore 1 enhanced-repeater  # Commutation VLAN
set worker lcore 2 pktgen            # Génération paquets
set worker lcore 3 l3-tap-handler    # Traitement L3

# Équilibrer charge entre lcores
set thread 1 port 0 queue 0
set thread 2 port 1 queue 0
set thread 3 port 2 queue 0
```

## Configuration des Threads

### Affectation Port-File-Thread
Chaque thread peut être affecté à des ports et files spécifiques :

```bash
# Thread 1 gère port 0, file 0
set thread 1 port 0 queue 0

# Thread 2 gère port 1, file 0
set thread 2 port 1 queue 0

# Thread peut gérer files multiples
set thread 1 port 0 queue 0
set thread 1 port 0 queue 1
```

### Équilibrage de Charge
- Distribuer files entre threads pour équilibrage
- Éviter surcharge d'un seul thread
- Considérer capacité de traitement par lcore

## Surveillance des Workers

### Commandes de Statut
```bash
# Afficher tous les workers
show worker

# Informations détaillées threads
show thread information

# Compteurs performance threads
show thread counter
```

### Métriques Performance
- **Utilisation CPU** : Charge par lcore
- **Débit Paquets** : Paquets traités par seconde
- **Latence** : Temps traitement par paquet
- **Compteurs Erreurs** : Erreurs et pertes

## Types de Workers Spécialisés

### Workers de Traitement Paquets
- **l2-repeater** : Répétition simple port-à-port
- **enhanced-repeater** : Commutation VLAN avec TAP
- **l3fwd** : Routage IP avec recherche LPM
- **pktgen** : Génération trafic haute performance

### Workers d'Infrastructure
- **rib-manager** : Synchronisation données système
- **stat-collector** : Agrégation métriques performance
- **tap-handler** : Interface avec pile réseau noyau
- **netlink-thread** : Communication avec noyau Linux

## Dépannage

### Problèmes Courants Workers
1. **Worker ne démarre pas** : Vérifier ordre configuration
2. **Performance faible** : Vérifier affectation lcores
3. **Contention ressources** : Équilibrer charge entre workers

### Stratégies de Diagnostic
```bash
# Vérifier configuration workers
show worker

# Surveiller performance threads
show thread counter

# Vérifier affectations port-file
show rib
```

### Optimisation Performance
- Isoler lcores critiques des interruptions système
- Utiliser affinité CPU pour threads haute performance
- Équilibrer charge entre lcores disponibles

## Emplacement de Définition

Ces commandes sont définies dans :
- `sdplane/worker_cmd.c` - Commandes gestion workers
- `sdplane/lthread_main.c` - Implémentation threads principaux
- `sdplane/rib.h` - Structures données workers

## Sujets Associés

- [Gestion Ports](port-management.md) - Configuration ports pour workers
- [Configuration Files](queue-configuration.md) - Affectation files aux workers
- [Information Threads](worker-lcore-thread-management.md) - Surveillance détaillée threads
- [RIB & Routage](routing.md) - Configuration workers dans RIB