# Guide Utilisateur sdplane-oss

**Language:** [English](../en/README.md) | [日本語](../ja/README.md) | **Français** | [中文](../zh/README.md) | [Deutsch](../de/README.md) | [Italiano](../it/README.md) | [한국어](../ko/README.md) | [ไทย](../th/README.md) | [Español](../es/README.md) 

sdplane-oss est un routeur logiciel haute performance basé sur DPDK. Ce guide utilisateur décrit toutes les commandes et fonctionnalités de sdplane.

## Table des Matières

1. [Gestion des Ports & Statistiques](port-management.md) - Gestion des ports DPDK et statistiques
2. [Gestion Workers & lcores & Informations Thread](worker-lcore-thread-management.md) - Gestion des threads workers, lcores et informations thread
3. [Débogage & Journalisation](debug-logging.md) - Fonctions de débogage et journalisation
4. [Gestion VTY & Shell](vty-shell.md) - Gestion VTY et shell
5. [Information Système & Surveillance](system-monitoring.md) - Information système et surveillance
6. [RIB & Routage](routing.md) - Fonctions RIB et routage
7. [Configuration des Files](queue-configuration.md) - Configuration et gestion des files
8. [Génération de Paquets](packet-generation.md) - Génération de paquets utilisant PKTGEN

9. [Interface TAP](tap-interface.md) - Gestion des interfaces TAP
10. [Gestion lthread](lthread-management.md) - Gestion des threads légers coopératifs
11. [Gestion des Périphériques](device-management.md) - Gestion des périphériques et pilotes

## Utilisation de Base

### Connexion

Pour se connecter à sdplane :

```bash
# Démarrer sdplane
sudo ./sdplane/sdplane

# Se connecter au CLI (depuis un autre terminal)
telnet localhost 9882
```

### Commandes de Base

```bash
# Afficher l'aide
sdplane> help

# Afficher la version
sdplane> show version

# Afficher l'état des ports
sdplane> show port all

# Afficher les workers
sdplane> show worker
```

## Applications

### Application Répéteur L2
Transmission simple de paquets Layer 2 avec apprentissage MAC :
- [Guide Application Répéteur L2](l2-repeater-application.md)

### Application Répéteur Amélioré  
Commutation VLAN avancée avec interfaces TAP :
- [Guide Application Répéteur Amélioré](enhanced-repeater-application.md)

### Application Générateur de Paquets
Génération de trafic haute performance et tests :
- [Guide Application Générateur de Paquets](packet-generator-application.md)

## Configuration

### Configuration de Base

1. **Configuration Système** :
   ```bash
   # Configurer hugepages
   echo 1024 > /sys/kernel/mm/hugepages/hugepages-2048kB/nr_hugepages
   
   # Lier NICs aux pilotes DPDK
   sudo modprobe vfio-pci
   echo 0000:03:00.0 > /sys/bus/pci/drivers/virtio-pci/unbind
   echo vfio-pci > /sys/bus/pci/devices/0000:03:00.0/driver_override
   echo 0000:03:00.0 > /sys/bus/pci/drivers/vfio-pci/bind
   ```

2. **Configuration sdplane** :
   ```bash
   # Exemple de configuration de base
   set rte_eal argv -c 0x7
   rte_eal_init
   
   set worker lthread stat-collector
   set worker lthread rib-manager
   set worker lcore 1 enhanced-repeater
   
   set port all dev-configure 1 4
   set port all promiscuous enable
   start port all
   start worker lcore all
   ```

### Guides de Configuration Détaillés

- [Répéteur Amélioré](enhanced-repeater.md) - Configuration répéteur amélioré
- [Gestion des Ports](port-management.md) - Configuration des ports DPDK
- [Gestion des Workers](worker-lcore-thread-management.md) - Configuration des workers et lcores
- [Gestion des Périphériques](device-management.md) - Configuration des périphériques et pilotes

## Surveillance et Débogage

### Surveillance Système
- [Information Système & Surveillance](system-monitoring.md) - Surveillance performance et état système
- [Information des Threads](worker-lcore-thread-management.md) - Surveillance des threads et performance

### Outils de Débogage
- [Débogage & Journalisation](debug-logging.md) - Outils de débogage et logs
- [Gestion VTY & Shell](vty-shell.md) - Interface CLI et gestion des sessions

## Développement et Intégration

### Guides de Développeur
- [Guide d'Intégration DPDK](dpdk-integration-guide.md) - Intégration d'applications DPDK personnalisées
- [Configuration des Files](queue-configuration.md) - Configuration avancée des files RX/TX
- [Gestion lthread](lthread-management.md) - Threading coopératif pour services système

### Interface et Virtualisation
- [Interface TAP](tap-interface.md) - Intégration avec Linux kernel via TAP
- [Génération de Paquets](packet-generation.md) - Outils de test et benchmarking

## Exemples Courants

### Configuration L2 Repeater
```bash
# Configuration minimale pour répéteur L2
set rte_eal argv -c 0x3
rte_eal_init
set worker lcore 1 l2-repeater
set port all dev-configure 1 1
start port all
start worker lcore all
```

### Configuration Enhanced Repeater avec TAP
```bash
# Configuration répéteur amélioré avec TAP
set rte_eal argv -c 0x7
rte_eal_init
set worker lthread stat-collector
set worker lthread rib-manager  
set worker lcore 1 enhanced-repeater
set worker lcore 2 l3-tap-handler
set port all dev-configure 1 4
start port all
start worker lcore all
```

### Test de Performance avec PKTGEN
```bash
# Configuration générateur de paquets
set rte_eal argv -c 0xf
pktgen init
set worker lcore 1 pktgen
set worker lcore 2 pktgen
set port all dev-configure 1 4
start port all
start worker lcore all

# Configuration PKTGEN pour test
pktgen set 0 count 0
pktgen set 0 size 64
pktgen set 0 rate 100
pktgen start 0
```

## Dépannage

### Problèmes Courants

1. **sdplane ne démarre pas** :
   ```bash
   # Vérifier hugepages
   cat /proc/meminfo | grep HugePages
   
   # Vérifier permissions
   sudo ./sdplane/sdplane
   ```

2. **Ports non détectés** :
   ```bash
   # Vérifier liaison pilotes
   dpdk-devbind.py -s
   
   # Vérifier configuration DPDK
   show port all
   ```

3. **Performance faible** :
   ```bash
   # Surveiller statistiques
   show port statistics all
   show thread counter
   
   # Vérifier configuration workers
   show worker
   ```

### Diagnostic
```bash
# Informations système
show system
show version
show memory

# État des composants
show port all
show worker  
show thread information

# Debug détaillé
debug sdplane worker
debug sdplane port
```

## Ressources Supplémentaires

- **Documentation DPDK** : https://doc.dpdk.org/
- **Repository GitHub** : https://github.com/kait-cronos/sdplane-oss
- **Issues et Support** : https://github.com/kait-cronos/sdplane-oss/issues

Pour assistance technique ou questions spécifiques, consultez les guides détaillés listés ci-dessus ou créez une issue sur GitHub.