# Application Répéteur L2

**Language:** [English](../l2-repeater-application.md) | [日本語](../ja/l2-repeater-application.md) | **Français** | [中文](../zh/l2-repeater-application.md)

L'application Répéteur L2 fournit une transmission simple de paquets Layer 2 entre ports DPDK avec fonctionnalité de répétition port-à-port basique.

## Vue d'ensemble

Le Répéteur L2 est une application de transmission Layer 2 intuitive avec les fonctionnalités suivantes :
- Transmission de paquets entre ports DPDK appairés (répétition simple port-à-port)
- Fonctionnalité de répétition de paquets basique sans apprentissage d'adresses MAC
- Fonctionnalité optionnelle de mise à jour d'adresses MAC (modification de l'adresse MAC source)
- Fonctionnement haute performance avec traitement de paquets zero-copy DPDK

## Fonctionnalités Clés

### Transmission Layer 2
- **Répétition Port-à-Port** : Transmission simple de paquets entre paires de ports préconfigurés
- **Sans Apprentissage MAC** : Répétition directe de paquets sans construction de table de transmission
- **Transmission Transparente** : Transmet tous les paquets indépendamment de l'adresse MAC de destination
- **Appariement de Ports** : Configuration fixe de transmission port-à-port

### Caractéristiques de Performance
- **Traitement Zero-Copy** : Utilise le traitement efficace de paquets DPDK
- **Traitement par Rafales** : Traitement de paquets par rafales pour débit optimal
- **Faible Latence** : Surcharge de traitement minimale pour transmission rapide
- **Support Multi-Cœur** : Exécution sur lcore dédié pour mise à l'échelle

## Configuration

### Configuration de Base
Le Répéteur L2 est configuré via le système de configuration sdplane principal :

```bash
# Définir le type de worker en répéteur L2
set worker lcore 1 l2-repeater

# Configuration des ports et files
set thread 1 port 0 queue 0  
set thread 1 port 1 queue 0

# Activer le mode promiscuous pour l'apprentissage
set port all promiscuous enable
```

### Exemple de Fichier de Configuration
Voir l'exemple de configuration complet dans [`example-config/sdplane_l2_repeater.conf`](../../example-config/sdplane_l2_repeater.conf) :

```bash
# Liaison de périphérique
set device 02:00.0 driver vfio-pci bind
set device 03:00.0 driver vfio-pci bind

# Initialisation DPDK
set rte_eal argv -c 0x7
rte_eal_init

# Workers d'arrière-plan (doivent être démarrés avant configuration des files)
set worker lthread stat-collector
set worker lthread rib-manager
set worker lthread netlink-thread

set mempool

# Configuration des files (doit être exécutée après démarrage rib-manager)
set thread 1 port 0 queue 0
set thread 1 port 1 queue 0

# Configuration des ports
stop port all
set port all dev-configure 1 4
set port all nrxdesc 1024
set port all ntxdesc 1024

# Configuration des workers
set worker lcore 1 l2-repeater
set worker lcore 2 tap-handler
set port all promiscuous enable
start port all

# Démarrer tous les workers
start worker lcore all
```

## Comportement de Transmission

### Modèle de Transmission par Diffusion
Le Répéteur L2 utilise un modèle de transmission par diffusion :

1. **Réception de Paquet** : Reçoit un paquet sur n'importe quel port DPDK actif
2. **Transmission par Diffusion** : Transmet le paquet vers tous les autres ports actifs
3. **Prévention de Boucle** : Évite de retransmettre vers le port d'entrée
4. **Aucun Apprentissage** : Ne construit ni ne maintient de tables MAC

### Flux de Traitement des Paquets

```
Port 0 ──→ [Répéteur L2] ──→ Ports 1,2,3 (actifs)
Port 1 ──→ [Répéteur L2] ──→ Ports 0,2,3 (actifs)
Port 2 ──→ [Répéteur L2] ──→ Ports 0,1,3 (actifs)
```

## Surveillance et Gestion

### Commandes de Statut
```bash
# Afficher les statistiques des ports
show port statistics all

# Afficher les informations des workers
show worker

# Afficher les informations des threads
show thread information
```

### Commandes de Débogage
```bash
# Débogage du répéteur L2
debug sdplane l2-repeater

# Débogage RIB et transmission
debug sdplane rib
debug sdplane fdb-change
```

## Cas d'Utilisation

### Hub Ethernet Simple
- Émulation d'un hub Ethernet traditionnel
- Transmission de tous les paquets vers tous les ports
- Idéal pour surveillance de réseau passive

### Test et Développement
- Configuration simple pour test d'applications réseau
- Répétition de trafic pour analyse
- Configuration de test basique pour validation DPDK

### Surveillance de Réseau
- Copie de paquets vers ports de surveillance
- Analyse de trafic et débogage
- Capture de paquets passive

## Considérations de Performance

### Optimisation
- **Affectation de Worker** : Dédier un lcore pour performance optimale
- **Configuration de File** : Équilibrer les affectations de files entre cœurs
- **Taille de Tampon** : Configurer appropriément les tampons de transmission

### Mise à l'Échelle
- **Multi-Port** : Supporte plusieurs ports DPDK simultanément
- **Traitement par Rafales** : Traite les paquets par rafales pour efficacité
- **Gestion Mémoire** : Utilise les pools mémoire DPDK pour allocation efficace

## Dépannage

### Problèmes Courants
- **Pas de transmission de paquets** : Vérifier l'état des ports et la configuration des workers
- **Performance faible** : Vérifier les affectations lcore et configuration des files
- **Perte de paquets** : Vérifier la taille des tampons et les statistiques des ports

### Stratégies de Débogage
- **Activer la journalisation de débogage** : Utiliser les commandes de débogage pour flux détaillé des paquets
- **Surveillance des statistiques** : Surveiller les statistiques de ports et workers
- **Vérification de configuration** : Confirmer la configuration des ports et workers

## Documentation Associée

- [Gestion des Workers](worker-management.md) - Détails de configuration des workers
- [Gestion des Ports](port-management.md) - Configuration des ports DPDK
- [Application Répéteur Amélioré](enhanced-repeater-application.md) - Fonctionnalités de commutation avancées