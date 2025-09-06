# Génération de Paquets

**Language:** [English](../packet-generation.md) | [Japanese](../ja/packet-generation.md) | **Français**

Configuration et utilisation du système de génération de paquets utilisant PKTGEN.

## Vue d'ensemble

Le système de génération de paquets de sdplane utilise PKTGEN pour créer du trafic réseau haute performance :

- **PKTGEN intégré** : Générateur de paquets DPDK intégré dans sdplane
- **Contrôle CLI** : Configuration via interface ligne de commande sdplane
- **Performance élevée** : Génération multi-gigabit avec DPDK
- **Flexibilité** : Patterns de trafic configurables et tests de charge

## Configuration de Base

### Initialisation PKTGEN
```bash
# Initialisation PKTGEN (inclut rte_eal_init)
set rte_eal argv -c 0x7
pktgen init

# Configuration workers PKTGEN
set worker lcore 1 pktgen
set worker lcore 2 pktgen
```

### Configuration Ports
```bash
# Configuration standard ports pour PKTGEN
stop port all
set port all dev-configure 1 4
set port all nrxdesc 1024
set port all ntxdesc 1024
set port all promiscuous enable
start port all

# Démarrer workers
start worker lcore all
```

## Commandes PKTGEN

### Configuration Paquets de Base

#### Nombre de Paquets
```bash
# Définir nombre paquets à envoyer
pktgen set <port> count <nombre>

# Exemples
pktgen set 0 count 1000      # Envoyer 1000 paquets
pktgen set 0 count 0         # Mode continu (infini)
```

#### Taille Paquets
```bash
# Définir taille paquets en octets
pktgen set <port> size <octets>

# Exemples
pktgen set 0 size 64         # Paquets 64 octets (minimum Ethernet)
pktgen set 0 size 1518       # Paquets taille maximum Ethernet
pktgen set 0 size 1024       # Paquets taille personnalisée
```

#### Taux Transmission
```bash  
# Définir taux en pourcentage débit ligne
pktgen set <port> rate <pourcentage>

# Exemples
pktgen set 0 rate 100        # Débit ligne complet
pktgen set 0 rate 50         # 50% débit ligne
pktgen set 0 rate 10         # 10% débit ligne
```

### Configuration Adresses

#### Adresses MAC
```bash
# Configurer adresses MAC source et destination
pktgen set <port> src mac <adresse-mac>
pktgen set <port> dst mac <adresse-mac>

# Exemples
pktgen set 0 src mac 00:11:22:33:44:55
pktgen set 0 dst mac 00:66:77:88:99:AA
```

#### Adresses IPv4
```bash
# Configurer adresses IP source et destination
pktgen set <port> src ip <adresse-ip>
pktgen set <port> dst ip <adresse-ip>

# Exemples
pktgen set 0 src ip 192.168.1.1
pktgen set 0 dst ip 192.168.1.2
```

#### Configuration Plages IPv4
```bash
# Définir plages d'adresses IP pour variation
pktgen set <port> src ip min <ip-min>
pktgen set <port> src ip max <ip-max>
pktgen set <port> src ip inc <incrément>

# Exemple : plage 192.168.1.1 à 192.168.1.100
pktgen set 0 src ip min 192.168.1.1
pktgen set 0 src ip max 192.168.1.100
pktgen set 0 src ip inc 1
```

### Contrôle Génération

#### Démarrer/Arrêter
```bash
# Démarrer génération sur port spécifique
pktgen start <port>
pktgen start 0

# Démarrer sur tous ports
pktgen start all

# Arrêter génération
pktgen stop <port>
pktgen stop 0
pktgen stop all
```

#### Pause/Reprise
```bash
# Pause génération (suspend temporairement)
pktgen pause <port>
pktgen pause 0

# Reprendre génération
pktgen resume <port>
pktgen resume 0
```

#### Reset
```bash
# Reset configuration et statistiques
pktgen reset <port>
pktgen reset 0
pktgen reset all
```

### Surveillance et Statistiques

#### Affichage Statistiques
```bash
# Afficher statistiques port spécifique
pktgen show <port>
pktgen show 0

# Afficher tous ports
pktgen show all

# Afficher configuration
pktgen show config <port>
```

#### Effacement Statistiques
```bash
# Effacer compteurs statistiques
pktgen clear <port>
pktgen clear 0
pktgen clear all
```

## Exemples Configuration

### Test Débit Basique
```bash
# Configuration simple débit maximum
pktgen set 0 count 0           # Mode continu
pktgen set 0 size 64          # Paquets minimaux
pktgen set 0 rate 100         # Débit ligne complet
pktgen set 0 src mac 00:11:22:33:44:55
pktgen set 0 dst mac 00:66:77:88:99:AA
pktgen set 0 src ip 10.0.0.1
pktgen set 0 dst ip 10.0.0.2

# Démarrer test
pktgen start 0

# Surveiller statistiques
watch -n 1 "pktgen show 0"
```

### Test Latence  
```bash
# Configuration paquets plus gros, taux réduit
pktgen set 0 count 1000       # Nombre fini pour mesure
pktgen set 0 size 1024        # Paquets plus gros
pktgen set 0 rate 1           # Taux faible pour latence
pktgen set 0 src ip 192.168.1.1
pktgen set 0 dst ip 192.168.1.2

# Démarrer et mesurer
pktgen start 0
```

### Test Charge Variable
```bash
# Configuration plage adresses pour diversité
pktgen set 0 count 0
pktgen set 0 size 512
pktgen set 0 rate 50
pktgen set 0 src ip min 10.0.1.1
pktgen set 0 src ip max 10.0.1.254
pktgen set 0 src ip inc 1
pktgen set 0 dst ip min 10.0.2.1  
pktgen set 0 dst ip max 10.0.2.254
pktgen set 0 dst ip inc 1

pktgen start 0
```

## Scripts Automation

### Script Test Performance
```bash
#!/bin/bash
# Script test performance automatisé

PKTGEN_PORT=0
TEST_DURATION=60

echo "Configuration PKTGEN..."
pktgen set $PKTGEN_PORT count 0
pktgen set $PKTGEN_PORT size 64
pktgen set $PKTGEN_PORT rate 100
pktgen set $PKTGEN_PORT src mac 00:11:22:33:44:55
pktgen set $PKTGEN_PORT dst mac 00:66:77:88:99:AA
pktgen set $PKTGEN_PORT src ip 10.0.0.1
pktgen set $PKTGEN_PORT dst ip 10.0.0.2

echo "Démarrage test ($TEST_DURATION secondes)..."
pktgen clear $PKTGEN_PORT
pktgen start $PKTGEN_PORT

sleep $TEST_DURATION

echo "Arrêt test et collecte résultats..."
pktgen stop $PKTGEN_PORT
pktgen show $PKTGEN_PORT
```

### Script Test Tailles Multiples
```bash
#!/bin/bash
# Test performance avec tailles paquets variables

SIZES=(64 128 256 512 1024 1518)
PORT=0

for size in "${SIZES[@]}"; do
    echo "Test avec paquets $size octets..."
    
    pktgen set $PORT size $size
    pktgen set $PORT count 10000
    pktgen clear $PORT
    pktgen start $PORT
    
    # Attendre fin génération
    sleep 5
    
    echo "Résultats pour $size octets:"
    pktgen show $PORT | grep -E "(TX|RX)"
    echo ""
done
```

## Surveillance Performance

### Métriques Clés
- **TX PPS** : Paquets transmis par seconde
- **TX Mbps** : Mégabits transmis par seconde  
- **RX PPS** : Paquets reçus par seconde (si applicable)
- **TX Count** : Nombre total paquets transmis
- **Errors** : Erreurs transmission

### Surveillance Continue
```bash
# Surveillance temps réel
watch -n 1 "pktgen show all"

# Logging automatique
while true; do
    date >> pktgen-stats.log
    pktgen show 0 >> pktgen-stats.log
    sleep 10
done
```

## Optimisation Performance

### Configuration Workers
```bash
# Utiliser workers multiples pour débit maximum
set worker lcore 1 pktgen     # TX worker
set worker lcore 2 pktgen     # RX worker  
set worker lcore 3 pktgen     # Worker additionnel
```

### Configuration Mémoire
```bash
# Augmenter descripteurs pour performance
set port all nrxdesc 2048
set port all ntxdesc 2048

# Configuration pools mémoire appropriés
set mempool
```

### Affinité CPU
- Affecter workers PKTGEN à lcores dédiés
- Éviter contention avec autres workers
- Utiliser lcores sur même nœud NUMA que ports

## Dépannage

### Problèmes Courants
1. **Pas de génération** : Vérifier configuration workers et ports
2. **Débit faible** : Vérifier configuration workers et mémoire
3. **Erreurs transmission** : Vérifier configuration ports et câblage
4. **Commandes non reconnues** : Vérifier initialisation PKTGEN

### Diagnostic
```bash
# Vérifier état PKTGEN
show worker | grep pktgen

# Vérifier configuration ports
show port all

# Vérifier statistiques
pktgen show all

# Debug PKTGEN
debug sdplane pktgen
```

## Limitations

### Contraintes Système
- **Initialisation unique** : Un seul appel `pktgen init` par session
- **Workers dédiés** : PKTGEN nécessite workers lcore dédiés
- **Ports partagés** : Conflit possible avec autres applications
- **Mémoire** : Utilisation significative pour génération haute performance

### Considérations Performance
- **CPU intensif** : Génération haute vitesse consomme CPU
- **Bande passante réseau** : Limitée par capacité interface
- **Cohérence temporelle** : Variations possibles selon charge système

## Emplacement de Définition

PKTGEN défini dans :
- `sdplane/pktgen_cmd.c` - Commandes CLI PKTGEN
- `module/pktgen/` - Intégration PKTGEN dans sdplane
- Code PKTGEN externe - Bibliothèque PKTGEN originale

## Sujets Associés

- [Application Générateur Paquets](packet-generator-application.md) - Guide application complet
- [Gestion Workers](worker-management.md) - Configuration workers PKTGEN
- [Gestion Ports](port-management.md) - Configuration ports pour PKTGEN  
- [Surveillance Système](system-monitoring.md) - Surveillance performance PKTGEN