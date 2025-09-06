# RIB & Routage

**Language:** [English](../routing.md) | [Japanese](../ja/routing.md) | **Français**

Commandes pour gérer la RIB (Routing Information Base) et les informations de ressources système. Ce manuel couvre la gestion complète de la base d'informations de routage, incluant la configuration des interfaces, la gestion des tables de voisinage, et l'optimisation des performances.

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
La RIB (Routing Information Base) est une base de données centralisée qui stocke les ressources système et informations réseau critiques pour le fonctionnement du routeur logiciel. Dans sdplane, elle constitue le point central de synchronisation pour tous les composants du plan de données et gère les informations suivantes :

- **Configuration de Commutateur Virtuel** - Commutation VLAN, agrégation de liens et affectations de ports avec support VLAN IEEE 802.1Q
- **Informations des Ports DPDK** - État des liens physiques, configuration avancée des files d'attente, capacités matérielles et paramètres de performance
- **Configuration des Files lcore** - Affectations optimisées de traitement de paquets par cœur CPU avec équilibrage de charge dynamique
- **Tables de Voisinage** - Entrées de base de données de transmission L2/L3 avec support ARP/NDP et découverte de voisinage automatique
- **Métriques de Performance** - Statistiques de débit, latence et utilisation des ressources
- **Politiques de QoS** - Classification de trafic et gestion de la bande passante

### Structure RIB
La RIB utilise une architecture à double niveau pour assurer la cohérence des données et l'accès concurrent thread-safe :

```c
struct rib {
    struct rib_info *rib_info;  // Pointeur vers les données réelles (protégé par RCU)
};

struct rib_info {
    uint32_t ver;                                    // Numéro de version (incrémenté à chaque modification)
    uint8_t vswitch_size;                           // Nombre de commutateurs virtuels configurés
    uint8_t port_size;                              // Nombre de ports DPDK initialisés
    uint8_t lcore_size;                             // Nombre de lcores disponibles
    struct vswitch_conf vswitch[MAX_VSWITCH];       // Configuration commutateur virtuel avec VLANs
    struct vswitch_link vswitch_link[MAX_VSWITCH_LINK]; // Liens ports VLAN avec états de liaison
    struct port_conf port[MAX_ETH_PORTS];           // Configuration ports DPDK avec paramètres avancés
    struct lcore_qconf lcore_qconf[RTE_MAX_LCORE];  // Affectations files lcore avec équilibrage
    struct neigh_table neigh_tables[NEIGH_NR_TABLES]; // Tables voisinage/transmission avec cache ARP
    struct flow_stats flow_statistics;              // Statistiques de flux et performance en temps réel
    struct qos_policy qos_policies[MAX_QOS_POLICIES]; // Politiques de qualité de service
};
```

#### Détails des Structures Internes

**Configuration des Commutateurs Virtuels :**
```c
struct vswitch_conf {
    uint16_t vlan_id;           // Identifiant VLAN (1-4094)
    uint8_t port_mask;          // Masque des ports membres
    uint8_t tagged_ports;       // Ports avec étiquetage VLAN
    uint32_t stp_state;         // État Spanning Tree Protocol
    uint64_t mac_learning;      // Table d'apprentissage MAC
};
```

**Configuration des Ports DPDK :**
```c
struct port_conf {
    struct rte_eth_conf port_conf;      // Configuration Ethernet DPDK
    struct rte_eth_link link_status;    // État du lien physique
    uint16_t nb_rx_queues;              // Nombre de files de réception
    uint16_t nb_tx_queues;              // Nombre de files de transmission
    struct rte_mempool *rx_pktmbuf_pool; // Pool de buffers de paquets RX
    struct port_stats statistics;        // Compteurs de performance
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
- **speed** - Vitesse du lien en Mbps (10, 100, 1000, 10000, 25000, 40000, 100000)
- **duplex** - Mode duplex full (bidirectionnel simultané) ou half (alternance émission/réception)
- **autoneg** - État de l'auto-négociation IEEE 802.3 (on/off) pour la négociation automatique des paramètres
- **status** - État physique du lien (up/down) avec détection de porteuse
- **nrxq/ntxq** - Nombre de files RX/TX configurées pour le parallélisme multi-cœurs
- **mtu** - Taille maximale des unités de transmission (défaut : 1500 octets)
- **promiscuous** - Mode promiscuité pour capture de tous les paquets
- **allmulticast** - Réception de tout le trafic multicast
- **link_autoneg** - Capacités d'auto-négociation supportées par le matériel
- **flow_ctrl** - Contrôle de flux IEEE 802.3x pour la gestion de congestion

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
La RIB utilise un système de surveillance en temps réel et est automatiquement mise à jour aux moments suivants :
- **Changements d'état des liens de ports DPDK** - Détection des transitions up/down avec notification immédiate
- **Changements de configuration des commutateurs virtuels** - Modification des VLANs, ports membres et politiques STP
- **Modifications d'affectation des files lcore** - Réallocation dynamique des ressources de traitement
- **Opérations de démarrage/arrêt de ports** - Synchronisation avec les changements d'état des interfaces
- **Mise à jour des tables de voisinage** - Apprentissage automatique des adresses MAC et entrées ARP
- **Modification des métriques de performance** - Collecte continue des statistiques de débit et latence
- **Changements de topologie réseau** - Détection automatique des modifications de connectivité

### Mises à Jour Manuelles
Pour actualiser et vérifier manuellement les informations RIB :
```bash
# Afficher l'état RIB actuel complet
show rib

# Mettre à jour l'état des ports (particulièrement utile pour les NICs Mellanox ConnectX)
update port status

# Forcer la synchronisation des tables de voisinage
update neigh tables

# Rafraîchir les statistiques de performance
update performance metrics

# Reconfigurer les affectations lcore optimales
optimize lcore assignment
```

### Gestion de la Cohérence des Données
La RIB utilise plusieurs mécanismes pour maintenir la cohérence :

#### Versioning et Atomicité
```c
// Mise à jour atomique de la RIB
static void rib_update_atomic(struct rib_info *new_rib_info) {
    uint32_t old_version = rcu_global_rib->rib_info->ver;
    new_rib_info->ver = old_version + 1;
    
    #if HAVE_LIBURCU_QSBR
    rcu_assign_pointer(rcu_global_ptr_rib->rib_info, new_rib_info);
    synchronize_rcu();  // Attendre que tous les lecteurs terminent
    #endif
}
```

#### Validation et Intégrité
```c
// Validation de l'intégrité de la RIB
static int rib_validate_integrity(struct rib_info *rib_info) {
    // Vérification des limites et cohérence des données
    if (rib_info->vswitch_size > MAX_VSWITCH) return -EINVAL;
    if (rib_info->port_size > MAX_ETH_PORTS) return -EINVAL;
    
    // Validation des configurations de ports
    for (int i = 0; i < rib_info->port_size; i++) {
        if (!validate_port_config(&rib_info->port[i])) return -EINVAL;
    }
    
    return 0;
}
```

## Dépannage et Diagnostic Avancé

### Problèmes de Traitement de Paquets

#### Diagnostic de Performance
1. **Vérification de la cohérence RIB**
```bash
# Analyse complète de l'état RIB
show rib
show rib version history  # Historique des versions
validate rib integrity    # Vérification de l'intégrité
```

2. **Diagnostic des ports et liens physiques**
```bash
# État détaillé des ports DPDK
show port all detailed
show port statistics all extended
show port capabilities all  # Capacités matérielles
show port flow-control all  # État du contrôle de flux
```

3. **Analyse des workers et distribution de charge**
```bash
# Affectations et utilisation des cœurs
show worker detailed
show thread performance
show lcore queue-assignment
show cpu utilization      # Utilisation CPU par cœur
```

4. **Diagnostic des files d'attente et buffers**
```bash
# État des files et pools de mémoire
show queue statistics all
show mempool usage       # Utilisation des pools de mémoire
show mbuf statistics     # Statistiques des buffers de paquets
```

#### Résolution des Problèmes de Performance

**Symptôme : Perte de paquets élevée**
```bash
# 1. Vérifier les statistiques de perte
show port statistics all | grep -i drop

# 2. Analyser l'utilisation des buffers
show mempool usage detailed

# 3. Optimiser la distribution des files
optimize queue distribution

# 4. Ajuster les paramètres de polling
set lcore polling-interval optimized
```

**Symptôme : Latence élevée**
```bash
# 1. Mesurer la latence end-to-end
measure latency port-to-port

# 2. Analyser les goulots d'étranglement
show performance bottlenecks

# 3. Optimiser les caches et préfetch
set cache optimization enabled

# 4. Ajuster les paramètres de scheduling
tune scheduler parameters
```

### Problèmes de Configuration des Ports

#### Ports Non Détectés dans la RIB
```bash
# Diagnostic étape par étape
# 1. Vérifier l'initialisation DPDK
show dpdk initialization-log
verify rte_eal_init status

# 2. Scanner les ports disponibles
scan available-ports
show pci devices network

# 3. Forcer la re-détection
rescan dpdk ports
rebind network interfaces

# 4. Configuration manuelle si nécessaire
set port manual-configure <port_id>
start port force <port_id>
```

#### Problèmes Spécifiques aux NICs Mellanox ConnectX
```bash
# Configuration spécialisée Mellanox
# 1. Vérifier les pilotes MLX5
check mlx5 driver version
verify mlx5 pmd availability

# 2. Mise à jour forcée du statut
update port status mellanox-specific
refresh mlx5 port-info

# 3. Configuration des paramètres avancés
set mlx5 rx-vec-enable true
set mlx5 txq-inline-max 256
set mlx5 rxq-cqe-comp-en true
```

### Problèmes de Tables de Voisinage et ARP

#### Diagnostic ARP/Neighbor Discovery
```bash
# 1. Vérifier l'état des tables de voisinage
show neigh tables all
show arp cache detailed
show ndp cache detailed  # Pour IPv6

# 2. Analyser les timeouts et aging
show neigh aging-timers
show arp resolution-statistics

# 3. Diagnostic des requêtes/réponses
show arp request-response-ratio
monitor arp traffic realtime
```

#### Résolution des Problèmes ARP
```bash
# Nettoyage et reconstruction des tables
flush arp cache
flush neigh tables
rebuild neigh discovery

# Configuration des paramètres d'aging
set arp timeout 300
set neigh gc-interval 30
set arp max-entries 1024
```

### Outils de Diagnostic Avancés

#### Monitoring en Temps Réel
```bash
# Surveillance continue des métriques
monitor rib changes realtime
monitor port link-state changes
monitor performance metrics 1s

# Alertes automatiques
set alert rib-version-mismatch
set alert port-link-down
set alert high-packet-loss 1%
```

#### Analyse de Performance Détaillée
```bash
# Profiling des performances
profile packet-processing 10s
analyze cpu-usage per-lcore
measure memory-bandwidth utilization

# Génération de rapports
generate performance-report
export rib-state snapshot
create troubleshooting-bundle
```

## Fonctionnalités Avancées et Optimisations

### Gestionnaire RIB (RIB Manager)
Le gestionnaire RIB opère comme un thread indépendant haute priorité et fournit un ensemble complet de services critiques :

#### Services de Surveillance
- **Surveillance automatique de l'état des ports DPDK** avec détection des changements en moins de 100ms
- **Monitoring continu des liens physiques** avec support pour les changements de vitesse/duplex dynamiques
- **Supervision des files d'attente** avec détection automatique des congestions et débordements
- **Surveillance de l'intégrité des données** avec validation périodique de la cohérence RIB

#### Services de Configuration
- **Gestion dynamique des commutateurs virtuels** avec reconfiguration à chaud des VLANs
- **Coordination intelligente des affectations lcore** avec répartition de charge automatique
- **Synchronisation temps réel** de l'état des ressources système distribuées
- **Gestion des politiques QoS** avec application dynamique des règles de trafic

#### Architecture du Gestionnaire
```c
// Structure du gestionnaire RIB
struct rib_manager {
    pthread_t manager_thread;           // Thread du gestionnaire
    volatile bool running;              // État de fonctionnement
    uint32_t update_interval_ms;        // Intervalle de mise à jour (défaut: 100ms)
    struct rib_update_queue *update_queue; // File des mises à jour pending
    struct rib_statistics stats;       // Statistiques de performance du gestionnaire
    struct callback_registry callbacks; // Callbacks pour événements RIB
};

// Boucle principale du gestionnaire
void* rib_manager_main_loop(void *arg) {
    struct rib_manager *manager = (struct rib_manager*)arg;
    
    while (manager->running) {
        // 1. Surveillance des ports DPDK
        rib_manager_monitor_ports(manager);
        
        // 2. Traitement des mises à jour en attente
        rib_manager_process_updates(manager);
        
        // 3. Validation de l'intégrité
        rib_manager_validate_integrity(manager);
        
        // 4. Optimisation des performances
        rib_manager_optimize_performance(manager);
        
        // Attente avant le prochain cycle
        usleep(manager->update_interval_ms * 1000);
    }
    
    return NULL;
}
```

### Optimisations de Performance Avancées

#### Cache-Aware Data Structures
```c
// Optimisation pour les caches CPU
struct rib_info_cacheline_optimized {
    // Ligne de cache 1: Données fréquemment accédées
    uint32_t ver;                    // Version (lecture très fréquente)
    uint8_t port_size;               // Taille des ports
    uint8_t lcore_size;              // Nombre de lcores
    uint16_t active_ports_mask;      // Masque des ports actifs
    // Padding pour aligner sur 64 bytes
    uint8_t padding1[52];
    
    // Ligne de cache 2: Configurations de ports critiques
    struct port_conf_compact port_configs[MAX_ETH_PORTS];
    
    // Lignes de cache suivantes: Données moins fréquentes
    struct vswitch_conf vswitch[MAX_VSWITCH];
    // ...
} __attribute__((aligned(64)));
```

#### Prefetching et Optimisations Mémoire
```c
// Prefetching intelligent des données RIB
static inline void rib_prefetch_port_config(uint16_t port_id) {
    struct rib_info *rib = rcu_dereference(rcu_global_ptr_rib->rib_info);
    
    // Prefetch de la configuration du port
    __builtin_prefetch(&rib->port[port_id], 0, 3);  // Lecture, haute localité
    
    // Prefetch des statistiques associées
    __builtin_prefetch(&rib->port[port_id].statistics, 0, 2);  // Localité moyenne
}
```

#### NUMA-Aware Memory Management
```c
// Allocation mémoire consciente NUMA
static struct rib_info* rib_alloc_numa_optimized(void) {
    int numa_node = numa_node_of_cpu(sched_getcpu());
    
    struct rib_info *rib_info = numa_alloc_onnode(
        sizeof(struct rib_info), numa_node
    );
    
    if (!rib_info) {
        // Fallback vers allocation standard
        rib_info = malloc(sizeof(struct rib_info));
    }
    
    // Initialisation avec awareness NUMA
    rib_init_numa_optimized(rib_info, numa_node);
    
    return rib_info;
}
```

### Système d'Événements et Callbacks

#### Registration et Gestion des Callbacks
```c
// Types d'événements RIB
enum rib_event_type {
    RIB_EVENT_PORT_UP,              // Port devient actif
    RIB_EVENT_PORT_DOWN,            // Port devient inactif
    RIB_EVENT_VSWITCH_MODIFIED,     // Configuration vswitch modifiée
    RIB_EVENT_LCORE_REASSIGNED,     // Réassignation lcore
    RIB_EVENT_NEIGH_TABLE_UPDATED,  // Table de voisinage mise à jour
    RIB_EVENT_PERFORMANCE_ALERT,    // Alerte de performance
};

// Callback pour événements RIB
typedef int (*rib_event_callback_t)(enum rib_event_type event, 
                                   void *event_data, 
                                   void *user_data);

// Registration de callbacks
int rib_register_event_callback(enum rib_event_type event_type,
                               rib_event_callback_t callback,
                               void *user_data) {
    struct callback_entry *entry = malloc(sizeof(struct callback_entry));
    entry->event_type = event_type;
    entry->callback = callback;
    entry->user_data = user_data;
    
    // Ajout thread-safe à la registry
    pthread_mutex_lock(&callback_registry_mutex);
    list_add_tail(&entry->list, &callback_registry.callbacks);
    pthread_mutex_unlock(&callback_registry_mutex);
    
    return 0;
}
```

### Sécurité Thread et Synchronisation Avancée
La RIB utilise RCU (Read-Copy-Update) pour un accès thread-safe haute performance avec garanties de cohérence :

#### Mécanisme RCU Détaillé
```c
// Accès en lecture thread-safe depuis les workers
#if HAVE_LIBURCU_QSBR
    urcu_qsbr_read_lock();  // Marquer le début de la section critique de lecture
    rib = (struct rib *) rcu_dereference(rcu_global_ptr_rib);  // Déréférencement sécurisé
    
    // Section critique : utilisation des données RIB
    // - Accès aux tables de routage
    // - Consultation des configurations de ports
    // - Lecture des métriques de performance
    
    urcu_qsbr_read_unlock();  // Fin de la section critique
#endif
```

#### Patterns de Mise à Jour Thread-Safe
```c
// Mise à jour thread-safe avec copie-modification-publication
static int rib_update_port_config(uint16_t port_id, struct port_conf *new_conf) {
    struct rib_info *old_rib = rcu_dereference(rcu_global_ptr_rib->rib_info);
    struct rib_info *new_rib = malloc(sizeof(struct rib_info));
    
    // Copie de l'ancienne structure
    memcpy(new_rib, old_rib, sizeof(struct rib_info));
    
    // Modification de la copie
    memcpy(&new_rib->port[port_id], new_conf, sizeof(struct port_conf));
    new_rib->ver = old_rib->ver + 1;  // Incrément de version
    
    // Publication atomique de la nouvelle version
    rcu_assign_pointer(rcu_global_ptr_rib->rib_info, new_rib);
    
    // Synchronisation et libération de l'ancienne version
    synchronize_rcu();
    free(old_rib);
    
    return 0;
}
```

#### Gestion des Races Conditions
```c
// Protection contre les conditions de course lors des lectures multiples
static inline struct port_conf* rib_get_port_config_safe(uint16_t port_id) {
    struct rib_info *rib_info;
    struct port_conf *port_conf = NULL;
    
    #if HAVE_LIBURCU_QSBR
    urcu_qsbr_read_lock();
    rib_info = rcu_dereference(rcu_global_ptr_rib->rib_info);
    
    if (port_id < rib_info->port_size) {
        port_conf = &rib_info->port[port_id];
        // La structure reste valide pendant toute la section RCU
    }
    
    urcu_qsbr_read_unlock();
    #endif
    
    return port_conf;
}
```

#### Optimisations de Performance RCU
- **Lecture sans Verrous** : Les threads de lecture n'acquièrent jamais de verrous mutuels, éliminant la contention
- **Latence Prévisible** : Les opérations de lecture ont une latence O(1) constante
- **Scalabilité Multi-Cœur** : Performance linéaire avec le nombre de cœurs CPU
- **Points de Quiescence** : Utilisation de urcu_qsbr_quiescent_state() pour optimiser la synchronisation

### Workers Associés
- **rib-manager** - Worker qui gère les mises à jour et synchronisation RIB
- **enhanced-repeater** - Utilise RIB pour commutation VLAN et état des ports
- **l2-repeater** - Utilise RIB pour décisions de transmission de paquets de base
- **l3-tap-handler** - Accède à RIB pour gestion des interfaces TAP

## Commandes RIB Étendues et Administration

### Commandes de Diagnostic Avancées

#### Information Détaillée des Commutateurs Virtuels
```bash
# Configurations complètes des commutateurs virtuels
show vswitch_rib detailed
show vswitch-link topology
show vswitch statistics performance

# Analyse de la topologie VLAN
show vlan topology all
show vlan membership matrix
show vlan traffic-flow analysis

# Interfaces spécialisées
show rib vswitch router-if detailed
show rib vswitch capture-if statistics
show rib vswitch trunk-ports configuration
```

#### Commandes de Surveillance en Temps Réel
```bash
# Monitoring continu de la RIB
watch rib changes 1s           # Surveillance des changements toutes les secondes
monitor rib version-updates     # Suivi des mises à jour de version
trace rib access-patterns       # Analyse des patterns d'accès

# Surveillance des performances
monitor rib performance-metrics realtime
watch port link-status changes
monitor lcore load-balancing efficiency
```

#### Commandes d'Optimisation
```bash
# Optimisation automatique des configurations
optimize rib memory-layout      # Optimisation de l'organisation mémoire
optimize lcore assignments      # Réoptimisation des affectations lcore
optimize queue distribution     # Redistribution optimale des files

# Tuning des performances
tune rib update-frequency       # Ajustement de la fréquence de mise à jour
tune cache prefetch-strategy    # Optimisation du prefetching
tune numa memory-affinity       # Optimisation NUMA
```

### Commandes de Maintenance et Administration

#### Sauvegarde et Restauration
```bash
# Sauvegarde de l'état RIB
backup rib state /path/to/backup.rib
export rib configuration json /path/to/config.json
snapshot rib memory-dump /path/to/snapshot.bin

# Restauration d'état
restore rib state /path/to/backup.rib
import rib configuration /path/to/config.json
load rib snapshot /path/to/snapshot.bin
```

#### Validation et Réparation
```bash
# Validation de l'intégrité
validate rib integrity comprehensive
check rib consistency cross-reference
verify rib data-structures alignment

# Réparation automatique
repair rib inconsistencies      # Réparation des incohérences détectées
rebuild rib from-scratch        # Reconstruction complète de la RIB
reset rib to-defaults          # Remise aux valeurs par défaut
```

#### Commandes de Débogage Avancé
```bash
# Débogage interne
debug rib memory-allocations    # Trace des allocations mémoire
debug rib rcu-operations       # Débogage des opérations RCU
debug rib thread-synchronization # Analyse de la synchronisation

# Profiling détaillé
profile rib access-latency     # Profile des latences d'accès
profile rib update-performance # Performance des mises à jour
profile rib memory-usage       # Analyse de l'utilisation mémoire
```

### Intégration avec les Outils de Monitoring

#### Export vers Systèmes de Monitoring
```bash
# Export Prometheus
export rib metrics prometheus-format
setup rib prometheus-exporter port 9100

# Export SNMP
configure rib snmp-agent community public
setup rib snmp-mibs standard

# Export JSON pour APIs
export rib state json-api format
setup rib rest-api endpoint /api/v1/rib
```

#### Intégration avec des Systèmes de Log
```bash
# Configuration du logging
set rib log-level debug
set rib log-destination syslog
set rib log-rotation daily

# Logging structuré
enable rib structured-logging json
set rib log-fields timestamp,version,event,details
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