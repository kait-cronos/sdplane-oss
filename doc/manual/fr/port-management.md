# Gestion des Ports & Statistiques

**Language:** [English](../port-management.md) | [日本語](../ja/port-management.md) | **Français** | [中文](../zh/port-management.md) | [Deutsch](../de/port-management.md) | [Italiano](../it/port-management.md) | [한국어](../ko/port-management.md) | [ไทย](../th/port-management.md) | [Español](../es/port-management.md)

Commandes pour la gestion des ports DPDK et la surveillance des statistiques.

## Liste des Commandes

### **start port**

Démarre les ports DPDK (comportement par défaut).

**Exemples :**
```bash
# Démarrer port (défaut)
start port
```

---

### **start port \<0-16\>**

Démarre un port DPDK spécifique.

**Exemples :**
```bash
# Démarrer le port 0
start port 0

# Démarrer le port 1
start port 1
```

---

### **start port all**

Démarre tous les ports DPDK.

**Exemples :**
```bash
# Démarrer tous les ports
start port all
```

---

### **stop port**

Arrête les ports DPDK (comportement par défaut).

**Exemples :**
```bash
# Arrêter port (défaut)
stop port
```

---

### **stop port \<0-16\>**

Arrête un port DPDK spécifique.

**Exemples :**
```bash
# Arrêter le port 0
stop port 0

# Arrêter le port 1
stop port 1
```

---

### **stop port all**

Arrête tous les ports DPDK.

**Exemples :**
```bash
# Arrêter tous les ports
stop port all
```

---

### **reset port**

Remet à zéro les ports DPDK (comportement par défaut).

**Exemples :**
```bash
# Remettre à zéro port (défaut)
reset port
```

---

### **reset port \<0-16\>**

Remet à zéro un port DPDK spécifique.

**Exemples :**
```bash
# Remettre à zéro le port 0
reset port 0

# Remettre à zéro le port 1
reset port 1
```

---

### **reset port all**

Remet à zéro tous les ports DPDK.

**Exemples :**
```bash
# Remettre à zéro tous les ports
reset port all
```

---

### **show port**

Affiche les informations de base pour tous les ports (comportement par défaut).

**Exemples :**
```bash
# Afficher les informations pour tous les ports
show port
```

---

### **show port \<0-16\>**

Affiche les informations de base pour un port spécifique.

**Exemples :**
```bash
# Afficher les informations du port 0
show port 0

# Afficher les informations du port 1
show port 1
```

---

### **show port all**

Affiche explicitement les informations pour tous les ports.

**Exemples :**
```bash
# Afficher explicitement les informations pour tous les ports
show port all
```

---

### **show port statistics**

Affiche toutes les informations statistiques des ports.

**Exemples :**
```bash
# Afficher toutes les statistiques
show port statistics
```

---

### **show port statistics pps**

Affiche les statistiques de paquets par seconde.

**Exemples :**
```bash
# Afficher uniquement les statistiques PPS
show port statistics pps
```

---

### **show port statistics total**

Affiche les statistiques du nombre total de paquets.

**Exemples :**
```bash
# Afficher le nombre total de paquets
show port statistics total
```

---

### **show port statistics bps**

Affiche les statistiques de bits par seconde.

**Exemples :**
```bash
# Afficher les bits par seconde
show port statistics bps
```

---

### **show port statistics Bps**

Affiche les statistiques d'octets par seconde.

**Exemples :**
```bash
# Afficher les octets par seconde
show port statistics Bps
```

---

### **show port statistics total-bytes**

Affiche les statistiques du nombre total d'octets.

**Exemples :**
```bash
# Afficher le total des octets
show port statistics total-bytes
```

---

### **show port \<0-16\> promiscuous**

Affiche le statut du mode promiscuous pour le port spécifié.

**Exemples :**
```bash
# Afficher le mode promiscuous du port 0
show port 0 promiscuous

# Afficher le mode promiscuous du port 1
show port 1 promiscuous
```

---

### **show port all promiscuous**

Affiche le statut du mode promiscuous pour tous les ports.

**Exemples :**
```bash
# Afficher le mode promiscuous pour tous les ports
show port all promiscuous
```

---

### **show port \<0-16\> flowcontrol**

Affiche les paramètres de contrôle de flux pour le port spécifié.

**Exemples :**
```bash
# Afficher les paramètres de contrôle de flux du port 0
show port 0 flowcontrol

# Afficher les paramètres de contrôle de flux du port 1
show port 1 flowcontrol
```

---

### **show port all flowcontrol**

Affiche les paramètres de contrôle de flux pour tous les ports.

**Exemples :**
```bash
# Afficher les paramètres de contrôle de flux pour tous les ports
show port all flowcontrol
```

---

### **set port \<0-16\> promiscuous enable**

Active le mode promiscuous pour le port spécifié.

**Exemples :**
```bash
# Activer le mode promiscuous du port 0
set port 0 promiscuous enable

# Activer le mode promiscuous du port 1
set port 1 promiscuous enable
```

---

### **set port \<0-16\> promiscuous disable**

Désactive le mode promiscuous pour le port spécifié.

**Exemples :**
```bash
# Désactiver le mode promiscuous du port 0
set port 0 promiscuous disable

# Désactiver le mode promiscuous du port 1
set port 1 promiscuous disable
```

---

### **set port all promiscuous enable**

Active le mode promiscuous pour tous les ports.

**Exemples :**
```bash
# Activer le mode promiscuous pour tous les ports
set port all promiscuous enable
```

---

### **set port all promiscuous disable**

Désactive le mode promiscuous pour tous les ports.

**Exemples :**
```bash
# Désactiver le mode promiscuous pour tous les ports
set port all promiscuous disable
```

---

### **set port \<0-16\> flowcontrol (rx|tx|autoneg|send-xon|fwd-mac-ctrl) (on|off)**

Configure les paramètres de contrôle de flux pour le port spécifié.

**Options :**
- `rx` - Contrôle de flux en réception
- `tx` - Contrôle de flux en transmission
- `autoneg` - Auto-négociation
- `send-xon` - Transmission XON
- `fwd-mac-ctrl` - Forwarding des trames de contrôle MAC

**Exemples :**
```bash
# Activer le contrôle de flux en réception pour le port 0
set port 0 flowcontrol rx on

# Désactiver l'auto-négociation pour le port 1
set port 1 flowcontrol autoneg off
```

---

### **set port all flowcontrol (rx|tx|autoneg|send-xon|fwd-mac-ctrl) (on|off)**

Configure les paramètres de contrôle de flux pour tous les ports.

**Options :**
- `rx` - Contrôle de flux en réception
- `tx` - Contrôle de flux en transmission
- `autoneg` - Auto-négociation
- `send-xon` - Transmission XON
- `fwd-mac-ctrl` - Forwarding des trames de contrôle MAC

**Exemples :**
```bash
# Désactiver l'auto-négociation pour tous les ports
set port all flowcontrol autoneg off

# Activer le contrôle de flux en transmission pour tous les ports
set port all flowcontrol tx on
```

---

### **set port \<0-16\> dev-configure \<0-64\> \<0-64\>**

Configure les paramètres de périphérique DPDK pour le port spécifié.

**Paramètres :**
- 1er argument : Nombre de files de réception (0-64)
- 2ème argument : Nombre de files de transmission (0-64)

**Exemples :**
```bash
# Configurer le port 0 avec 4 files RX et 4 files TX
set port 0 dev-configure 4 4

# Configurer le port 1 avec 2 files RX et 2 files TX
set port 1 dev-configure 2 2
```

---

### **set port all dev-configure \<0-64\> \<0-64\>**

Configure les paramètres de périphérique DPDK pour tous les ports.

**Paramètres :**
- 1er argument : Nombre de files de réception (0-64)
- 2ème argument : Nombre de files de transmission (0-64)

**Exemples :**
```bash
# Configurer tous les ports avec 1 file RX et 1 file TX
set port all dev-configure 1 1
```

---

### **set port \<0-16\> nrxdesc \<0-16384\>**

Définit le nombre de descripteurs de réception pour le port spécifié.

**Exemples :**
```bash
# Définir le nombre de descripteurs de réception à 1024 pour le port 0
set port 0 nrxdesc 1024

# Définir le nombre de descripteurs de réception à 512 pour le port 1
set port 1 nrxdesc 512
```

---

### **set port all nrxdesc \<0-16384\>**

Définit le nombre de descripteurs de réception pour tous les ports.

**Exemples :**
```bash
# Définir le nombre de descripteurs de réception à 512 pour tous les ports
set port all nrxdesc 512
```

---

### **set port \<0-16\> ntxdesc \<0-16384\>**

Définit le nombre de descripteurs de transmission pour le port spécifié.

**Exemples :**
```bash
# Définir le nombre de descripteurs de transmission à 1024 pour le port 0
set port 0 ntxdesc 1024

# Définir le nombre de descripteurs de transmission à 512 pour le port 1
set port 1 ntxdesc 512
```

---

### **set port all ntxdesc \<0-16384\>**

Définit le nombre de descripteurs de transmission pour tous les ports.

**Exemples :**
```bash
# Définir le nombre de descripteurs de transmission à 512 pour tous les ports
set port all ntxdesc 512
```

---

### **set port \<0-16\> link up**

Active la liaison pour le port spécifié.

**Exemples :**
```bash
# Activer la liaison du port 0
set port 0 link up

# Activer la liaison du port 1
set port 1 link up
```

---

### **set port \<0-16\> link down**

Désactive la liaison pour le port spécifié.

**Exemples :**
```bash
# Désactiver la liaison du port 0
set port 0 link down

# Désactiver la liaison du port 1
set port 1 link down
```

---

### **set port all link up**

Active la liaison pour tous les ports.

**Exemples :**
```bash
# Activer la liaison pour tous les ports
set port all link up
```

---

### **set port all link down**

Désactive la liaison pour tous les ports.

**Exemples :**
```bash
# Désactiver la liaison pour tous les ports
set port all link down
```

## Concepts Fondamentaux

### Architecture des Ports DPDK

Les ports DPDK constituent l'interface principale entre sdplane-oss et les interfaces réseau physiques ou virtuelles. Chaque port représente une interface réseau qui peut être :

- **Interfaces physiques** : Cartes réseau Ethernet supportant DPDK
- **Interfaces virtuelles** : Interfaces virtio-net dans des environnements virtualisés
- **Interfaces de test** : Ports de génération de paquets pour les tests

### Modèle de Données des Ports

Les ports dans sdplane-oss utilisent une architecture basée sur des files (queues) :
- **Files de réception (RX)** : Reçoivent les paquets entrants
- **Files de transmission (TX)** : Transmettent les paquets sortants
- **Descripteurs** : Structures de données gérant les tampons de paquets

## Gestion du Cycle de Vie des Ports

### 1. Initialisation des Ports

L'initialisation des ports suit un processus séquentiel strict :

```bash
# Phase 1 : Arrêter tous les ports existants
stop port all

# Phase 2 : Configuration du périphérique
set port all dev-configure 4 4

# Phase 3 : Configuration des descripteurs
set port all nrxdesc 1024
set port all ntxdesc 1024

# Phase 4 : Configuration des paramètres avancés
set port all promiscuous enable
set port all flowcontrol rx on
set port all flowcontrol tx on

# Phase 5 : Activation des ports
start port all
```

### 2. États des Ports

Les ports peuvent être dans différents états :

- **Arrêté (Stopped)** : Port inactif, aucun traitement de paquets
- **Démarré (Started)** : Port actif, traitement de paquets en cours
- **Lien Inactif (Link Down)** : Port démarré mais sans connectivité physique
- **Lien Actif (Link Up)** : Port démarré avec connectivité physique

### 3. Configuration Optimale des Ports

#### Configuration Multi-Cœurs
```bash
# Configuration pour système 8 cœurs
set port all dev-configure 8 8
set port all nrxdesc 2048
set port all ntxdesc 2048
```

#### Configuration Faible Latence
```bash
# Configuration optimisée pour la latence
set port all dev-configure 1 1
set port all nrxdesc 512
set port all ntxdesc 512
```

#### Configuration Haute Performance
```bash
# Configuration optimisée pour le débit
set port all dev-configure 16 16
set port all nrxdesc 4096
set port all ntxdesc 4096
```

## Surveillance et Statistiques Détaillées

### Types de Métriques

#### 1. Statistiques de Trafic
- **Paquets par Seconde (PPS)** : `show port statistics pps`
- **Bits par Seconde (bps)** : `show port statistics bps`
- **Octets par Seconde (Bps)** : `show port statistics Bps`

#### 2. Statistiques Cumulatives
- **Total des Paquets** : `show port statistics total`
- **Total des Octets** : `show port statistics total-bytes`

#### 3. Surveillance en Temps Réel
```bash
# Surveillance continue des statistiques PPS
watch -n 1 "show port statistics pps"

# Surveillance des erreurs
watch -n 1 "show port statistics"

# Surveillance d'un port spécifique
watch -n 1 "show port 0"
```

### Analyse des Performances

#### Métriques Clés de Performance
```bash
# Afficher toutes les métriques pour analyse
show port statistics

# Analyser les performances par port
for i in {0..7}; do
    echo "Port $i:"
    show port $i
    show port statistics pps
    echo "---"
done
```

## Configuration Avancée des Ports

### 1. Contrôle de Flux (Flow Control)

Le contrôle de flux prévient la congestion réseau :

```bash
# Configuration complète du contrôle de flux
set port all flowcontrol rx on          # Accepter les trames de pause
set port all flowcontrol tx on          # Envoyer des trames de pause
set port all flowcontrol autoneg on     # Auto-négociation activée
set port all flowcontrol send-xon on    # Envoyer des trames XON
set port all flowcontrol fwd-mac-ctrl on # Forwarder les trames de contrôle MAC
```

#### Scénarios d'Usage du Contrôle de Flux

**1. Réseaux à Haute Charge :**
```bash
set port all flowcontrol rx on
set port all flowcontrol tx on
```

**2. Réseaux Faible Latence :**
```bash
set port all flowcontrol rx off
set port all flowcontrol tx off
```

### 2. Mode Promiscuous

Le mode promiscuous permet de recevoir tous les paquets :

```bash
# Activation sélective du mode promiscuous
set port 0 promiscuous enable    # Port bridge/switch
set port 1 promiscuous disable   # Port normal

# Vérification du statut
show port all promiscuous
```

### 3. Configuration des Files et Descripteurs

#### Calcul Optimal des Descripteurs
```bash
# Pour 10 Gbps avec paquets 1500 octets :
# PPS max ≈ 820,000 paquets/s
# Latency buffer = 1ms = 820 paquets
# Recommended: 1024-2048 descripteurs

set port all nrxdesc 2048
set port all ntxdesc 2048
```

#### Configuration Adaptative par Port
```bash
# Port haute performance (port 0-1)
set port 0 dev-configure 8 8
set port 0 nrxdesc 4096
set port 0 ntxdesc 4096

# Port standard (port 2-3)
set port 2 dev-configure 2 2
set port 2 nrxdesc 1024
set port 2 ntxdesc 1024
```

## Optimisation des Performances

### 1. Configuration NUMA

#### Identification NUMA
```bash
# Vérifier la topologie NUMA du système
numactl --hardware

# Afficher l'affectation NUMA des ports
show port all
```

#### Configuration NUMA Optimisée
```bash
# Aligner les ports sur les nœuds NUMA appropriés
# Port 0-1 → NUMA node 0
# Port 2-3 → NUMA node 1

# Configuration workers par NUMA node
set worker 0 cpu 0    # NUMA node 0, core 0
set worker 1 cpu 2    # NUMA node 0, core 2
set worker 2 cpu 8    # NUMA node 1, core 8
set worker 3 cpu 10   # NUMA node 1, core 10
```

### 2. Optimisation des Hugepages

#### Vérification des Hugepages
```bash
# Vérifier les hugepages disponibles
cat /proc/meminfo | grep Huge

# Configuration recommandée
echo 2048 > /sys/devices/system/node/node0/hugepages/hugepages-2048kB/nr_hugepages
echo 2048 > /sys/devices/system/node/node1/hugepages/hugepages-2048kB/nr_hugepages
```

### 3. Réglage des Interruptions

#### Configuration des Interruptions
```bash
# Désactiver les interruptions pour les ports DPDK
echo 0 > /proc/irq/24/smp_affinity  # Port 0
echo 0 > /proc/irq/25/smp_affinity  # Port 1

# Isoler les cœurs CPU pour DPDK
# Ajouter à la ligne de commande du noyau : isolcpus=2-15
```

## Dépannage Avancé

### 1. Diagnostic des Problèmes de Port

#### Vérification de l'État des Ports
```bash
# Vérification complète de l'état
show port all
show port all promiscuous
show port all flowcontrol

# Vérification des statistiques d'erreur
show port statistics
```

#### Analyse des Problèmes Courants

**1. Port ne démarre pas :**
```bash
# Vérifier la liaison physique
show port 0

# Réinitialiser le port
stop port 0
reset port 0
start port 0
```

**2. Performance faible :**
```bash
# Vérifier la configuration
show port statistics pps
show port statistics bps

# Optimiser la configuration
set port 0 dev-configure 8 8
set port 0 nrxdesc 4096
```

**3. Perte de paquets :**
```bash
# Surveiller les compteurs d'erreur
watch -n 1 "show port statistics"

# Augmenter les tampons
set port all nrxdesc 8192
```

### 2. Problèmes Spécifiques aux Fournisseurs

#### Cartes Mellanox
```bash
# Mise à jour du statut requise pour Mellanox
update port status

# Vérification après mise à jour
show port all
```

#### Cartes Intel
```bash
# Configuration optimisée pour Intel
set port all flowcontrol autoneg off
set port all dev-configure 4 4
```

#### Interfaces Virtio-net
```bash
# Configuration pour environnements virtualisés
set port all dev-configure 1 1
set port all nrxdesc 512
set port all ntxdesc 512
```

### 3. Scripts de Diagnostic

#### Script de Vérification Globale
```bash
#!/bin/bash
echo "=== Diagnostic des Ports ==="
show port all
echo ""
echo "=== Statistiques ==="
show port statistics
echo ""
echo "=== Mode Promiscuous ==="
show port all promiscuous
echo ""
echo "=== Contrôle de Flux ==="
show port all flowcontrol
```

#### Script de Test de Performance
```bash
#!/bin/bash
echo "=== Test Performance Initiale ==="
show port statistics pps

echo "Attente 10 secondes..."
sleep 10

echo "=== Test Performance Après 10s ==="
show port statistics pps
```

## Configuration en Production

### 1. Configuration Type Routeur
```bash
# Arrêter tous les ports
stop port all

# Configuration haute performance
set port all dev-configure 8 8
set port all nrxdesc 4096
set port all ntxdesc 4096

# Désactiver le mode promiscuous (routage L3)
set port all promiscuous disable

# Contrôle de flux activé
set port all flowcontrol rx on
set port all flowcontrol tx on

# Démarrer tous les ports
start port all
```

### 2. Configuration Type Commutateur
```bash
# Arrêter tous les ports
stop port all

# Configuration multi-files
set port all dev-configure 4 4
set port all nrxdesc 2048
set port all ntxdesc 2048

# Mode promiscuous activé (commutation L2)
set port all promiscuous enable

# Contrôle de flux standard
set port all flowcontrol rx on
set port all flowcontrol tx on

# Démarrer tous les ports
start port all
```

### 3. Configuration Type Générateur de Trafic
```bash
# Arrêter tous les ports
stop port all

# Configuration maximale pour génération
set port all dev-configure 16 16
set port all nrxdesc 8192
set port all ntxdesc 8192

# Mode promiscuous pour capture
set port all promiscuous enable

# Pas de contrôle de flux (performance max)
set port all flowcontrol rx off
set port all flowcontrol tx off

# Démarrer tous les ports
start port all
```

## Automatisation et Scripts

### 1. Script d'Initialisation Automatique
```bash
#!/bin/bash
# init_ports.sh - Initialisation automatique des ports

PORT_CONFIG_FILE="/etc/sdplane/port_config.conf"

# Lecture de la configuration
source $PORT_CONFIG_FILE

# Application de la configuration
stop port all
set port all dev-configure $RX_QUEUES $TX_QUEUES
set port all nrxdesc $RX_DESC
set port all ntxdesc $TX_DESC

if [ "$PROMISCUOUS_MODE" = "enable" ]; then
    set port all promiscuous enable
else
    set port all promiscuous disable
fi

start port all
echo "Configuration des ports terminée"
```

### 2. Script de Surveillance Continue
```bash
#!/bin/bash
# monitor_ports.sh - Surveillance continue des ports

LOG_FILE="/var/log/sdplane/port_stats.log"

while true; do
    TIMESTAMP=$(date '+%Y-%m-%d %H:%M:%S')
    echo "[$TIMESTAMP] Port Statistics:" >> $LOG_FILE
    show port statistics >> $LOG_FILE
    echo "---" >> $LOG_FILE
    sleep 60
done
```

## Intégration avec d'Autres Composants

### 1. Coordination avec RIB
```bash
# Synchronisation des états de port avec RIB
show port all          # État physique des ports
show rib interfaces    # État logique dans RIB

# Mise à jour forcée de la synchronisation
update port status
update rib
```

### 2. Coordination avec Workers
```bash
# Vérifier l'affectation des workers aux ports
show worker all
show port all

# Optimiser l'affectation worker-port
set worker 0 ports 0,1
set worker 1 ports 2,3
```

## Emplacement de Définition

Ces commandes sont définies dans les fichiers suivants :
- `sdplane/dpdk_port_cmd.c` - Implémentations des commandes de port
- `lib/sdplane/port_management.c` - Fonctions de gestion des ports  
- `include/sdplane/port.h` - Structures de données et définitions
- `sdplane/rib.h` - Intégration avec le système RIB

## Sujets Associés

- [Gestion des Workers & lcore](worker-management.md) - Configuration des workers et affectation aux ports
- [Surveillance Système & Monitoring](system-monitoring.md) - Surveillance globale du système incluant les ports
- [Configuration des Files](queue-configuration.md) - Configuration détaillée des files de traitement