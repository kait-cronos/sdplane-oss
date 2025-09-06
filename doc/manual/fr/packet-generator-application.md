# Application Générateur de Paquets (PKTGEN)

**Language:** [English](../packet-generator-application.md) | [日本語](../ja/packet-generator-application.md) | **Français** | [中文](../zh/packet-generator-application.md)

L'application Générateur de Paquets (PKTGEN) fournit une génération de paquets haute performance et des fonctionnalités de test de trafic utilisant le framework de traitement de paquets optimisé DPDK.

## Vue d'ensemble

PKTGEN est un outil sophistiqué de génération de trafic intégré dans sdplane-oss, offrant :
- **Génération de Paquets Haute Vitesse** : Taux de transmission multi-gigabit de paquets
- **Motifs de Trafic Flexibles** : Taille de paquets, taux et motifs personnalisables
- **Support Multi-Port** : Génération de trafic indépendante sur plusieurs ports
- **Fonctionnalités Avancées** : Tests de plage, limitation de taux, façonnage de trafic
- **Test de Performance** : Mesure de débit réseau et de latence

## Architecture

### Composants Principaux
- **Moteur TX** : Transmission de paquets haute performance utilisant DPDK
- **Moteur RX** : Réception de paquets et collecte de statistiques
- **Framework L2P** : Mappage lcore-vers-port pour performance optimale
- **Gestion de Configuration** : Configuration dynamique des paramètres de trafic
- **Moteur de Statistiques** : Métriques de performance complètes et rapports

### Modèle de Worker
PKTGEN opère en utilisant des threads workers dédiés (lcores) :
- **Workers TX** : Cœurs dédiés à la transmission de paquets
- **Workers RX** : Cœurs dédiés à la réception de paquets
- **Workers Mixtes** : TX/RX combiné sur un seul cœur
- **Thread de Contrôle** : Gestion et collecte de statistiques

## Fonctionnalités Clés

### Génération de Trafic
- **Taux de Paquets** : Génération de trafic à débit ligne jusqu'aux limites d'interface
- **Taille de Paquets** : Configurable de 64 octets aux trames jumbo
- **Motifs de Trafic** : Motifs uniformes, en rafales et personnalisés
- **Multi-Flux** : Multiples flux de trafic par port

### Fonctionnalités Avancées
- **Limitation de Taux** : Contrôle précis du taux de trafic
- **Tests de Plage** : Balayage de taille de paquets et de taux
- **Motifs de Charge** : Trafic constant, montée progressive, en rafales
- **Support de Protocole** : Ethernet, IPv4, IPv6, TCP, UDP

### Surveillance de Performance
- **Statistiques en Temps Réel** : Taux TX/RX, compteurs de paquets, compteurs d'erreurs
- **Mesure de Latence** : Test de latence de paquets bout-à-bout
- **Analyse de Débit** : Utilisation de bande passante et efficacité
- **Détection d'Erreurs** : Détection de perte et corruption de paquets

## Configuration

### Configuration de Base
PKTGEN nécessite une initialisation spécifique et configuration de workers :

```bash
# Liaison de périphérique pour PKTGEN
set device 01:00.0 driver vfio-pci bind
set device 01:00.1 driver vfio-pci bind

# Initialisation PKTGEN (utilise rte_eal_init en interne)
set rte_eal argv -c 0x7
pktgen init

# Configuration de pools mémoire
set mempool

# Configuration des workers (après initialisation PKTGEN)
set worker lcore 1 pktgen
set worker lcore 2 pktgen

# Configuration des ports
stop port all
set port all dev-configure 1 4
set port all nrxdesc 1024
set port all ntxdesc 1024
set port all promiscuous enable
start port all

# Démarrer les workers
start worker lcore all
```

### Configuration des Paramètres de Trafic

Une fois PKTGEN configuré, définir les paramètres de génération de trafic :

```bash
# Définir les paramètres de base des paquets
pktgen set 0 count 1000
pktgen set 0 size 64
pktgen set 0 rate 100

# Configurer les adresses
pktgen set 0 src mac 00:11:22:33:44:55
pktgen set 0 dst mac 00:66:77:88:99:AA
pktgen set 0 src ip 192.168.1.1
pktgen set 0 dst ip 192.168.1.2

# Démarrer la génération
pktgen start 0
```

## Commandes CLI PKTGEN

### Commandes de Configuration de Base

```bash
# Configuration du comptage de paquets
pktgen set <port> count <nombre>

# Configuration de la taille des paquets
pktgen set <port> size <octets>

# Configuration du taux (pourcentage de débit ligne)
pktgen set <port> rate <pourcentage>
```

### Configuration d'Adresses

```bash
# Adresses MAC
pktgen set <port> src mac <adresse-mac>
pktgen set <port> dst mac <adresse-mac>

# Adresses IPv4
pktgen set <port> src ip <adresse-ip>
pktgen set <port> dst ip <adresse-ip>

# Configuration de plage IPv4
pktgen set <port> src ip min <ip-min>
pktgen set <port> src ip max <ip-max>
pktgen set <port> src ip inc <incrément>
```

### Contrôle de Trafic

```bash
# Démarrer/Arrêter la génération
pktgen start <port>
pktgen stop <port>
pktgen start all
pktgen stop all

# Pause et reprise
pktgen pause <port>
pktgen resume <port>

# Reset
pktgen reset <port>
pktgen reset all
```

### Commandes de Surveillance

```bash
# Afficher les statistiques
pktgen show <port>
pktgen show all

# Afficher la configuration
pktgen show config <port>

# Effacer les statistiques
pktgen clear <port>
pktgen clear all
```

## Exemple de Configuration

### Configuration Complète
Voir [`example-config/sdplane_pktgen.conf`](../../example-config/sdplane_pktgen.conf) pour un exemple complet :

```bash
# Configuration PKTGEN
set device 01:00.0 driver vfio-pci bind
set device 01:00.1 driver vfio-pci bind

# Initialisation (PKTGEN initialise DPDK en interne)
set rte_eal argv -c 0x7
pktgen init
set mempool

# Configuration des workers
set worker lcore 1 pktgen
set worker lcore 2 pktgen

# Configuration des ports
stop port all
set port all dev-configure 1 4
set port all nrxdesc 1024
set port all ntxdesc 1024
set port all promiscuous enable
start port all

# Démarrer les workers
start worker lcore all

# Configuration du trafic
pktgen set 0 count 0
pktgen set 0 size 64
pktgen set 0 rate 100
pktgen set 0 src mac 00:11:22:33:44:55
pktgen set 0 dst mac 00:66:77:88:99:AA
pktgen set 0 src ip 192.168.1.1
pktgen set 0 dst ip 192.168.1.2

# Démarrer la génération
pktgen start all
```

## Cas d'Utilisation

### Test de Performance Réseau
- Test de débit et latence des liens
- Validation de performance d'équipements réseau
- Benchmarking de débit d'applications

### Test de Charge
- Test de stress sur équipements réseau
- Validation de capacité de commutateurs/routeurs
- Test de résilience sous charge élevée

### Développement et Validation
- Test d'applications de traitement de paquets
- Validation de pipeline de données
- Test de régression pour optimisations réseau

## Optimisation de Performance

### Configuration des Workers
- **Séparation TX/RX** : Dédier des lcores séparés pour TX et RX
- **Affinité CPU** : Aligner les workers sur les cœurs physiques
- **Isolation de Cœurs** : Isoler les cœurs PKTGEN des interruptions système

### Optimisation Mémoire
- **Taille de Pool** : Configurer des pools mémoire appropriés pour la charge
- **Taille de Descripteur** : Équilibrer taille de descripteur et utilisation mémoire
- **Configuration NUMA** : Aligner la mémoire sur les nœuds CPU locaux

## Surveillance et Métriques

### Métriques Clés
- **Taux de Transmission** : Paquets par seconde et bits par seconde
- **Taux de Réception** : Validation de livraison de paquets
- **Latence** : Temps de trajet aller-retour des paquets
- **Perte de Paquets** : Taux de perte et motifs

### Collecte de Statistiques
```bash
# Surveillance en temps réel
pktgen show all

# Statistiques détaillées
pktgen show 0

# Effacement périodique
pktgen clear all
```

## Dépannage

### Problèmes Courants
- **Pas de génération de paquets** : Vérifier l'initialisation et configuration des workers
- **Performance faible** : Vérifier l'affinité CPU et configuration des files
- **Perte de paquets** : Ajuster les tailles de tampons et taux de génération

### Outils de Diagnostic
- **Statistiques de ports** : Surveiller TX/RX et erreurs au niveau ports
- **Statistiques de workers** : Vérifier la charge et performance des workers
- **Logs système** : Examiner les logs pour erreurs et avertissements

## Documentation Associée

- [Gestion des Workers](worker-management.md) - Configuration des workers et optimisation
- [Gestion des Ports](port-management.md) - Configuration et surveillance des ports DPDK
- [Surveillance Système](system-monitoring.md) - Métriques système et surveillance