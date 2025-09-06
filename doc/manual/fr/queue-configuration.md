# Configuration des Files

**Language:** [English](../queue-configuration.md) | [日本語](../ja/queue-configuration.md) | **Français** | [中文](../zh/queue-configuration.md)

Configuration et gestion des files de réception et transmission pour ports DPDK.

## Vue d'ensemble

Les files (queues) dans DPDK permettent la parallélisation du traitement de paquets :
- **Files RX** : Réception de paquets depuis interfaces réseau
- **Files TX** : Transmission de paquets vers interfaces réseau
- **Affectation Multi-Core** : Distribution files entre lcores
- **Équilibrage Charge** : Optimisation performance via répartition

## Configuration des Files

### Configuration Port-File
```bash
# Configurer nombre files RX/TX par port
set port <port-id|all> dev-configure <nrxq> <ntxq>

# Exemple : 1 file RX, 4 files TX
set port all dev-configure 1 4
```

### Affectation Thread-File
```bash
# Affecter file spécifique à thread worker
set thread <thread-id> port <port-id> queue <queue-id>

# Exemples d'affectation
set thread 1 port 0 queue 0  # Thread 1 gère port 0, file 0
set thread 2 port 1 queue 0  # Thread 2 gère port 1, file 0
```

### Configuration Descripteurs
```bash
# Configurer nombre descripteurs RX par file
set port <port-id|all> nrxdesc <nombre>

# Configurer nombre descripteurs TX par file
set port <port-id|all> ntxdesc <nombre>

# Exemple configuration
set port all nrxdesc 1024   # 1024 descripteurs RX
set port all ntxdesc 1024   # 1024 descripteurs TX
```

## Types de Files

### Files de Réception (RX)
- **Fonction** : Recevoir paquets depuis interface réseau
- **Buffer Ring** : Anneau descripteurs pour paquets entrants
- **Traitement** : Distribution paquets vers workers appropriés
- **Performance** : Impact direct sur débit réception

### Files de Transmission (TX)
- **Fonction** : Transmettre paquets vers interface réseau
- **Buffer Ring** : Anneau descripteurs pour paquets sortants
- **Agrégation** : Combinaison paquets de sources multiples
- **Performance** : Impact sur débit transmission et latence

## Modèles de Configuration

### Configuration Mono-File
```bash
# Une file RX, une file TX par port
set port all dev-configure 1 1

# Affectation simple
set thread 1 port 0 queue 0
set thread 2 port 1 queue 0
```

**Avantages :**
- Configuration simple
- Faible complexité gestion
- Approprié pour applications basiques

**Inconvénients :**
- Limitation débit
- Pas de parallélisation

### Configuration Multi-File TX
```bash
# Une file RX, multiples files TX
set port all dev-configure 1 4

# Affectation files TX multiples
set thread 1 port 0 queue 0  # File RX
# Files TX gérées automatiquement par système
```

**Avantages :**
- Amélioration débit transmission
- Parallélisation transmission
- Réduction contention TX

### Configuration Multi-File RX/TX
```bash
# Files RX et TX multiples
set port all dev-configure 4 4

# Affectation équilibrée
set thread 1 port 0 queue 0
set thread 2 port 0 queue 1
set thread 3 port 0 queue 2
set thread 4 port 0 queue 3
```

**Avantages :**
- Performance maximale
- Parallélisation complète
- Scalabilité optimale

**Inconvénients :**
- Complexité configuration
- Plus de ressources CPU

## Optimisation Performance

### Dimensionnement Descripteurs
- **Taille Anneau** : Balance mémoire vs performance
- **Petites Tailles** : Moins mémoire, plus cache-friendly
- **Grandes Tailles** : Plus de tamponnage, moins débordements

```bash
# Configuration équilibrée
set port all nrxdesc 1024   # Standard pour la plupart cas
set port all ntxdesc 1024

# Configuration haute performance
set port all nrxdesc 2048   # Plus tamponnage
set port all ntxdesc 2048
```

### Affinité NUMA
- Aligner files avec cœurs CPU locaux
- Utiliser mémoire locale au nœud CPU
- Éviter trafic inter-NUMA

```bash
# Exemple affinité NUMA
# NUMA node 0 : ports 0-1, lcores 0-3
set thread 1 port 0 queue 0  # lcore 1, port 0
set thread 2 port 1 queue 0  # lcore 2, port 1

# NUMA node 1 : ports 2-3, lcores 4-7
set thread 5 port 2 queue 0  # lcore 5, port 2
set thread 6 port 3 queue 0  # lcore 6, port 3
```

## Équilibrage de Charge

### Distribution RSS (Receive Side Scaling)
RSS distribue paquets entrants entre files RX :
- **Hachage** : Basé sur en-têtes paquets
- **Distribution** : Paquets flux similaires vers même file
- **Performance** : Parallélisation réception

### Stratégies Affectation
```bash
# Affectation 1:1 (un thread par file)
set thread 1 port 0 queue 0
set thread 2 port 0 queue 1
set thread 3 port 0 queue 2

# Affectation N:1 (threads multiples par file)
set thread 1 port 0 queue 0
set thread 1 port 1 queue 0  # Thread gère ports multiples
```

## Surveillance Files

### Métriques Clés
- **Débit Files** : Paquets par seconde par file
- **Utilisation Descripteurs** : Pourcentage anneaux utilisés
- **Débordements** : Files saturées perdant paquets
- **Distribution** : Équilibrage charge entre files

### Commandes Surveillance
```bash
# Afficher état files
show rib                    # Information configuration files
show port statistics all    # Statistiques niveau port
show thread information     # Performance threads par file
```

## Dépannage

### Problèmes Courants
1. **Files saturées** : Débordements et perte paquets
2. **Déséquilibre charge** : Files inégalement utilisées
3. **Performance faible** : Mauvaise affectation files
4. **Contention mémoire** : Problèmes accès NUMA

### Solutions
```bash
# Augmenter tailles descripteurs
set port all nrxdesc 2048
set port all ntxdesc 2048

# Redistribuer files
set thread 1 port 0 queue 0
set thread 2 port 0 queue 1  # Ajouter thread pour équilibrage

# Vérifier configuration
show rib
show port statistics all
```

### Indicateurs Performance
- **Taux utilisation files** : < 80% recommandé
- **Débordements** : Zéro idéal
- **Distribution uniforme** : Files équitablement chargées
- **Latence traitement** : Temps réponse acceptable

## Configuration Avancée

### Optimisations Spécialisées
```bash
# Configuration haute fréquence (trading)
set port all dev-configure 1 1    # Files minimales
set port all nrxdesc 512          # Anneaux plus petits
set port all ntxdesc 512          # Latence réduite

# Configuration haut débit (streaming)
set port all dev-configure 4 4    # Files maximales
set port all nrxdesc 4096         # Grands anneaux
set port all ntxdesc 4096         # Débit maximal
```

### Profiles Application
- **L2 Repeater** : Configuration simple mono-file
- **Enhanced Repeater** : Multi-file TX pour performance
- **PKTGEN** : Configuration optimisée génération
- **L3FWD** : Files multiples pour routage parallèle

## Bonnes Pratiques

### Configuration Initiale
1. Commencer avec configuration simple (1 RX, 4 TX)
2. Tester performance avec charge représentative
3. Ajuster based sur métriques observées
4. Optimiser pour cas d'usage spécifique

### Surveillance Continue
1. Surveiller métriques files régulièrement
2. Identifier goulots d'étranglement
3. Ajuster configuration dynamiquement
4. Documenter configurations optimales

### Maintenance
1. Rotation logs surveillance
2. Nettoyage statistiques périodique
3. Validation configuration après changements
4. Tests régression performance

## Emplacement de Définition

Configuration files définie dans :
- `sdplane/dpdk_port_cmd.c` - Commandes configuration ports/files
- `sdplane/rib.h` - Structures données configuration files

## Sujets Associés

- [Gestion Ports](port-management.md) - Configuration ports DPDK
- [Gestion Workers](worker-management.md) - Affectation workers aux files
- [Information Threads](thread-information.md) - Surveillance performance threads
- [Surveillance Système](system-monitoring.md) - Métriques système globales