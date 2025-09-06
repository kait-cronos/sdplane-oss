# Guide d'Intégration d'Applications DPDK

**Language:** [English](../dpdk-integration-guide.md) | [Japanese](../ja/dpdk-integration-guide.md) | **Français**

Ce guide explique comment intégrer des applications DPDK dans sdplane en utilisant l'approche DPDK-dock.

## Vue d'ensemble

L'approche DPDK-dock permet d'exécuter plusieurs applications DPDK dans un seul espace de processus, partageant l'initialisation DPDK, les pools mémoire et la configuration des ports. Cette méthode offre :

- **Espace processus partagé** : Applications DPDK multiples dans un processus
- **Ressources communes** : Pools mémoire et ports DPDK partagés
- **Gestion unifiée** : Configuration et surveillance via interface sdplane
- **Performance optimisée** : Évite surcharge multi-processus
- **Intégration CLI** : Contrôle via interface ligne de commande sdplane

## Applications Intégrées

### Applications Intégrées avec Succès
- **L2FWD** : Transmission Layer 2 basique
- **PKTGEN** : Génération de paquets haute performance
- **Enhanced Repeater** : Commutation VLAN avec interfaces TAP
- **L2 Repeater** : Transmission simple port-à-port

### Intégration Partielle
- **L3FWD** : Transmission Layer 3 (intégration basique)

## Modèles d'Intégration

### 1. Initialisation Partagée DPDK
Toutes les applications utilisent une initialisation DPDK commune :

```c
// Une seule initialisation DPDK pour toutes les applications
rte_eal_init()    // Appelé par : rte_eal_init
// OU
pktgen init       // Appelle rte_eal_init() en interne
// OU
l2fwd init        // Appelle rte_eal_init() en interne
```

**Important** : Un seul appel d'initialisation DPDK par session sdplane.

### 2. Accès Thread-Safe aux Données avec RCU
Utilise RCU (Read-Copy-Update) pour accès thread-safe à la RIB :

```c
static __thread struct rib *rib = NULL;

void my_worker_function(void) {
    uint64_t loop_counter = 0;
    
    // Enregistrer compteur boucle pour surveillance performance
    int thread_id = thread_lookup_by_lcore(my_worker_function, lcore_id);
    thread_register_loop_counter(thread_id, &loop_counter);
    
    while (1) {
        loop_counter++;
        
        #if HAVE_LIBURCU_QSBR
            urcu_qsbr_read_lock();
            rib = (struct rib *) rcu_dereference(rcu_global_ptr_rib);
        #endif
        
        // Utiliser données RIB de manière thread-safe
        if (rib && rib->rib_info) {
            // Traitement application...
        }
        
        #if HAVE_LIBURCU_QSBR
            urcu_qsbr_read_unlock();
            urcu_qsbr_quiescent_state();
        #endif
    }
}
```

### 3. Variables Thread-Local
Utilise variables thread-local pour état par worker :

```c
// Variables globales thread-local
static __thread struct rib *rib = NULL;
static __thread uint64_t loop_counter = 0;
```

## Étapes d'Intégration

### Étape 1 : Analyse de l'Application
1. **Identifier points d'initialisation DPDK** dans code application
2. **Localiser boucles principales** et logique traitement
3. **Identifier données globales** nécessitant synchronisation
4. **Analyser dépendances** sur services système

### Étape 2 : Adaptation Code
1. **Modifier initialisation** pour utiliser contexte sdplane partagé
2. **Implémenter accès RCU** pour données partagées
3. **Ajouter variables thread-local** pour état worker
4. **Intégrer surveillance compteur boucle**

### Étape 3 : Intégration CLI
1. **Ajouter commandes configuration** à système CLI sdplane
2. **Implémenter commandes contrôle** (start/stop/configuration)
3. **Ajouter commandes statut** pour surveillance application
4. **Intégrer aide en ligne** et complétion

### Étape 4 : Configuration Worker
1. **Enregistrer type worker** dans système sdplane
2. **Implémenter fonction worker** avec modèles appropriés
3. **Configurer affectations lcore** pour worker
4. **Tester intégration** avec configuration exemple

## Exemple d'Intégration : L2FWD

### Code Worker Adapté
```c
#include "rib_manager.h"

static __thread struct rib *rib = NULL;
static __thread uint64_t loop_counter = 0;

void l2fwd_worker_function(void *arg) {
    unsigned lcore_id = rte_lcore_id();
    int thread_id = thread_lookup_by_lcore(l2fwd_worker_function, lcore_id);
    thread_register_loop_counter(thread_id, &loop_counter);
    
    while (1) {
        loop_counter++;
        
        #if HAVE_LIBURCU_QSBR
            urcu_qsbr_read_lock();
            rib = (struct rib *) rcu_dereference(rcu_global_ptr_rib);
        #endif
        
        if (rib && rib->rib_info) {
            // Logique transmission L2FWD utilisant données RIB
            l2fwd_main_loop();
        }
        
        #if HAVE_LIBURCU_QSBR
            urcu_qsbr_read_unlock();
            urcu_qsbr_quiescent_state();
        #endif
    }
}
```

### Configuration CLI
```bash
# Initialisation DPDK (une seule fois)
set rte_eal argv -c 0x7
l2fwd init

# Configuration workers
set worker lcore 1 l2fwd
set worker lcore 2 l2fwd

# Configuration ports
stop port all
set port all dev-configure 1 4
start port all

# Démarrer workers
start worker lcore all
```

## Considérations Performance

### Surveillance Compteur Boucle
Le compteur boucle surveille performance worker :
- Incrémenté à chaque itération boucle worker
- Utilisé pour détecter workers bloqués ou lents
- Accessible via commandes surveillance système

### Optimisation RCU
- Sections critiques RCU courtes pour performance
- Éviter opérations blocantes dans sections RCU
- Utiliser état quiescent approprié entre lectures

### Affectation lcore
- Dédier lcores spécifiques pour applications haute performance
- Éviter partage lcores entre applications conflictuelles
- Considérer topologie NUMA pour optimisation mémoire

## Outils de Développement

### Commandes Debug
```bash
# Debug intégration DPDK
debug sdplane dpdk

# Debug workers spécifiques
debug sdplane l2fwd
debug sdplane pktgen

# Surveillance performance
show thread counter
show worker
```

### Surveillance Performance
```c
// Variables surveillance application
static uint64_t packets_processed = 0;
static uint64_t processing_errors = 0;

// Dans boucle worker
packets_processed++;
if (error_condition) {
    processing_errors++;
}
```

## Bonnes Pratiques

### Gestion Ressources
1. **Pools mémoire** : Utiliser pools sdplane existants
2. **Configuration ports** : Respecter configuration sdplane
3. **Gestion threads** : Suivre modèles thread sdplane
4. **Nettoyage ressources** : Implémenter nettoyage approprié

### Sécurité Thread
1. **Accès RCU** : Toujours utiliser primitives RCU pour RIB
2. **Variables thread-local** : Utiliser pour état spécifique worker
3. **Synchronisation** : Minimiser partage données entre threads
4. **État quiescent** : Signaler appropriément points quiescents

### Intégration CLI
1. **Commandes cohérentes** : Suivre conventions nommage sdplane
2. **Aide complète** : Fournir descriptions et exemples
3. **Gestion erreurs** : Validation entrée et messages erreur clairs
4. **Complétion** : Implémenter complétion commandes appropriée

## Modèles Applications

### Application Traitement Paquets
Applications focalisées sur traitement paquets haute performance :
- Utilise modèle RCU pour accès données
- Implémente surveillance compteur boucle
- Intègre avec système surveillance sdplane

### Application Contrôle
Applications fournissant services contrôle :
- Peut utiliser threading différent
- Focus sur intégration CLI
- Moins critique pour performance

## Dépannage

### Problèmes Communs
1. **Conflits initialisation DPDK** : S'assurer une seule initialisation
2. **Accès données non thread-safe** : Implémenter protection RCU appropriée
3. **Performance worker** : Vérifier surveillance compteur boucle
4. **Configuration CLI** : Valider intégration commandes

### Outils Debug
- Journalisation debug pour flux application
- Surveillance métriques performance
- Validation configuration via commandes show
- Test avec charges diverses

## Sujets Associés

- [Gestion Workers](worker-management.md) - Configuration et gestion workers
- [RIB & Routage](routing.md) - Accès données système avec RCU
- [Information Threads](thread-information.md) - Surveillance performance threads
- [Gestion Ports](port-management.md) - Configuration ports DPDK partagés