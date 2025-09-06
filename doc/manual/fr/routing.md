# RIB & Routage

**Language:** [English](../routing.md) | [Japanese](../ja/routing.md) | **Français**

Commandes pour gérer la RIB (Routing Information Base) et les informations de ressources système.

## Liste des Commandes

### show_rib - Afficher les Informations RIB
```
show rib
```

Affiche les informations de la RIB (Routing Information Base).

**Exemples :**
```bash
show rib
```

Cette commande affiche les informations suivantes :
- Version RIB et pointeur mémoire
- Configurations de commutateurs virtuels et affectations VLAN
- État des ports DPDK et configurations des files
- Affectations des files lcore-to-port
- Tables de voisinage pour transmission L2/L3

## Vue d'ensemble de la RIB

### Qu'est-ce que la RIB
La RIB (Routing Information Base) est une base de données centralisée qui stocke les ressources système et informations réseau. Dans sdplane, elle gère les informations suivantes :

- **Configuration de Commutateur Virtuel** - Commutation VLAN et affectations de ports
- **Informations des Ports DPDK** - État des liens, configuration des files et capacités
- **Configuration des Files lcore** - Affectations de traitement de paquets par cœur CPU
- **Tables de Voisinage** - Entrées de base de données de transmission L2/L3

### Structure RIB
La RIB consiste en deux structures principales :

```c
struct rib {
    struct rib_info *rib_info;  // Pointeur vers les données réelles
};

struct rib_info {
    uint32_t ver;                                    // Numéro de version
    uint8_t vswitch_size;                           // Nombre de commutateurs virtuels
    uint8_t port_size;                              // Nombre de ports DPDK
    uint8_t lcore_size;                             // Nombre de lcores
    struct vswitch_conf vswitch[MAX_VSWITCH];       // Configuration commutateur virtuel
    struct vswitch_link vswitch_link[MAX_VSWITCH_LINK]; // Liens ports VLAN
    struct port_conf port[MAX_ETH_PORTS];           // Configuration ports DPDK
    struct lcore_qconf lcore_qconf[RTE_MAX_LCORE];  // Affectations files lcore
    struct neigh_table neigh_tables[NEIGH_NR_TABLES]; // Tables voisinage/transmission
};
```

## Lecture des Informations RIB

### Éléments d'Affichage de Base
- **Version RIB** - Numéro de version RIB actuel et pointeur mémoire
- **Commutateurs Virtuels** - Configurations VLAN et affectations de ports
- **Ports DPDK** - État des liens, vitesse, duplex, configurations des files
- **lcores** - Affectations de cœurs CPU et files de traitement de paquets
- **Tables de Voisinage** - Entrées de base de données de transmission L2/L3

### État des Ports DPDK
- **speed** - Vitesse du lien en Mbps
- **duplex** - Mode duplex full/half
- **autoneg** - État de l'auto-négociation (on/off)
- **status** - État du lien (up/down)
- **nrxq/ntxq** - Nombre de files RX/TX

## Exemples d'Utilisation

### Utilisation de Base
```bash
# Afficher les informations RIB
show rib
```

### Interprétation d'Exemple de Sortie
```
rib information version: 21 (0x55555dd42010)
vswitches: 
dpdk ports: 
  dpdk_port[0]: 
    link: speed=1000Mbps duplex=full autoneg=on status=up
    nb_rxd=1024 nb_txd=1024
    queues: nrxq=1 ntxq=4
  dpdk_port[1]: 
    link: speed=0Mbps duplex=half autoneg=on status=down
    nb_rxd=1024 nb_txd=1024
    queues: nrxq=1 ntxq=4
  dpdk_port[2]: 
    link: speed=0Mbps duplex=half autoneg=off status=down
    nb_rxd=1024 nb_txd=1024
    queues: nrxq=1 ntxq=4
lcores: 
  lcore[0]: nrxq=0
  lcore[1]: nrxq=1
    rxq[0]: dpdk_port[0], queue_id=0
  lcore[2]: nrxq=1
    rxq[0]: dpdk_port[1], queue_id=0
  lcore[3]: nrxq=1
    rxq[0]: dpdk_port[2], queue_id=0
  lcore[4]: nrxq=0
  lcore[5]: nrxq=0
  lcore[6]: nrxq=0
  lcore[7]: nrxq=0
```

Dans cet exemple :
- La version RIB 21 montre l'état système actuel
- Le port DPDK 0 est actif (up) avec une vitesse de lien 1Gbps
- Les ports DPDK 1 et 2 sont inactifs (down) sans lien
- Les lcores 1, 2, 3 sont affectés pour gérer les paquets des ports 0, 1, 2 respectivement
- Chaque port utilise 1 file RX et 4 files TX
- Les anneaux de descripteurs RX/TX sont configurés avec 1024 entrées chacun

## Gestion RIB

### Mises à Jour Automatiques
La RIB est automatiquement mise à jour aux moments suivants :
- Changements d'état des liens de ports DPDK
- Changements de configuration des commutateurs virtuels
- Modifications d'affectation des files lcore
- Opérations de démarrage/arrêt de ports

### Mises à Jour Manuelles
Pour actualiser et vérifier manuellement les informations RIB :
```bash
# Afficher l'état RIB actuel
show rib

# Mettre à jour l'état des ports (utile pour les NICs Mellanox)
update port status
```

## Dépannage

### Quand le Traitement de Paquets Ne Fonctionne Pas Correctement
1. Vérifier les informations RIB pour les affectations de ports et files
```bash
show rib
```

2. Vérifier l'état des ports DPDK et l'état des liens
```bash
show port all
show port statistics all
```

3. Vérifier les affectations workers et lcore
```bash
show worker
show thread
```

### Quand les Ports Ne Sont Pas Affichés dans la RIB
- Vérifier l'initialisation des ports DPDK avec `rte_eal_init`
- Vérifier la configuration des ports avec `set port all dev-configure`
- S'assurer que les ports sont démarrés avec `start port all`
- Pour les NICs Mellanox, exécuter la commande `update port status`

## Fonctionnalités Avancées

### Gestionnaire RIB
Le gestionnaire RIB opère comme un thread indépendant et fournit les fonctions suivantes :
- Surveillance automatique de l'état des ports DPDK
- Gestion de configuration des commutateurs virtuels
- Coordination d'affectation des files lcore
- Synchronisation de l'état des ressources système

### Sécurité Thread basée RCU
La RIB utilise RCU (Read-Copy-Update) pour un accès thread-safe :
```c
// Accès RIB depuis les threads workers
#if HAVE_LIBURCU_QSBR
    urcu_qsbr_read_lock();
    rib = (struct rib *) rcu_dereference(rcu_global_ptr_rib);
#endif
    // Utiliser les données rib...
#if HAVE_LIBURCU_QSBR
    urcu_qsbr_read_unlock();
#endif
```

### Workers Associés
- **rib-manager** - Worker qui gère les mises à jour et synchronisation RIB
- **enhanced-repeater** - Utilise RIB pour commutation VLAN et état des ports
- **l2-repeater** - Utilise RIB pour décisions de transmission de paquets de base
- **l3-tap-handler** - Accède à RIB pour gestion des interfaces TAP

## Commandes RIB Supplémentaires

### Information des Commutateurs Virtuels
```bash
# Afficher les configurations des commutateurs virtuels
show vswitch_rib
show vswitch-link

# Afficher les interfaces router et capture
show rib vswitch router-if
show rib vswitch capture-if
```

## Emplacement de Définition

Cette commande est définie dans les fichiers suivants :
- `sdplane/rib.c` - Fonctions d'affichage RIB principales
- `sdplane/rib.h` - Définitions des structures de données RIB
- `sdplane/rib_manager.h` - Interface du gestionnaire RIB

## Sujets Associés

- [Répéteur Amélioré](enhanced-repeater.md) - Configuration commutateur virtuel
- [Gestion des Ports & Statistiques](port-management.md) - Gestion des ports DPDK
- [Gestion des Workers & lcores](worker-management.md) - Affectations des threads workers
- [Information Système & Surveillance](system-monitoring.md) - Commandes de surveillance système