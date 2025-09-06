# Gestion lthread

**Language:** [English](../lthread-management.md) | [日本語](../ja/lthread-management.md) | **Français** | [中文](../zh/lthread-management.md) | [Deutsch](../de/lthread-management.md) | [Italiano](../it/lthread-management.md) | [한국어](../ko/lthread-management.md) | [ไทย](../th/lthread-management.md) | [Español](../es/lthread-management.md)

Gestion des threads légers coopératifs (lthread) pour services système.

## Vue d'ensemble

lthread est une bibliothèque de threading coopératif léger utilisée dans sdplane pour services système :

- **Threading Coopératif** : Threads s'exécutent coopérativement sans préemption
- **Faible Surcharge** : Changement contexte minimal comparé aux threads OS
- **Services Système** : Idéal pour tâches E/O et gestion
- **Intégration sdplane** : Framework intégré pour workers système

## lthreads Système

### Types lthread Disponibles
```bash
# Collecteur statistiques système
set worker lthread stat-collector

# Gestionnaire RIB (base informations routage)
set worker lthread rib-manager

# Gestionnaire table voisinage  
set worker lthread neigh-manager

# Thread communication netlink Linux
set worker lthread netlink-thread

# Gestionnaire interfaces TAP
set worker lthread tap-handler
```

### Ordre Configuration Recommandé
**Important** : Respecter ordre pour dépendances :

```bash
# 1. Collecteur statistiques (premier)
set worker lthread stat-collector

# 2. Gestionnaire RIB (avant neigh-manager)
set worker lthread rib-manager

# 3. Gestionnaire voisinage (après rib-manager)
set worker lthread neigh-manager

# 4. Thread netlink (après neigh-manager)
set worker lthread netlink-thread

# 5. Autres services optionnels
set worker lthread tap-handler
```

## Architecture lthread

### Modèle Exécution
lthread utilise modèle coopératif :
- **Thread Principal** : Contrôle et coordination
- **Scheduler Coopératif** : Planification basée yield volontaire
- **Event Loop** : Traitement événements asynchrone
- **I/O Non-Bloquant** : Opérations I/O asynchrones

### Intégration avec lcores
```
lcore 0 (principal)
├── lthread stat-collector
├── lthread rib-manager  
├── lthread neigh-manager
├── lthread netlink-thread
└── lthread tap-handler

lcore 1-N (workers)
├── worker applications
└── traitement paquets
```

## Services lthread

### stat-collector
Collecte métriques système périodiquement :

**Fonctions :**
- Rassemblement statistiques ports DPDK
- Métriques performance workers  
- Surveillance ressources système
- Agrégation données pour surveillance

**Configuration :**
```bash
set worker lthread stat-collector
```

### rib-manager
Gère base informations routage (RIB) :

**Fonctions :**
- Synchronisation données RIB entre threads
- Mises à jour configuration ports/workers
- Coordination état système
- Thread-safety via RCU

**Configuration :**
```bash
set worker lthread rib-manager
```

### neigh-manager
Gestion table voisinage L2/L3 :

**Fonctions :**
- Maintenance tables ARP/ND
- Apprentissage adresses MAC
- Expiration entrées obsolètes
- Synchronisation avec noyau Linux

**Configuration :**
```bash
set worker lthread neigh-manager
```

### netlink-thread
Communication avec noyau Linux via netlink :

**Fonctions :**
- Écoute événements réseau noyau
- Synchronisation état interfaces
- Propagation changements topologie
- Intégration services système Linux

**Configuration :**
```bash
set worker lthread netlink-thread
```

### tap-handler
Gestion interfaces TAP (si utilisées) :

**Fonctions :**
- Création/destruction interfaces TAP
- Transfer paquets DPDK ↔ noyau
- Gestion événements interface TAP
- Coordination avec enhanced-repeater

**Configuration :**
```bash
set worker lthread tap-handler
```

## Cycle Vie lthread

### Initialisation
```bash
# lthreads démarrent automatiquement après configuration
set worker lthread stat-collector
set worker lthread rib-manager

# Pas de commande start explicite nécessaire
```

### Surveillance
```bash
# Vérifier état lthreads via workers
show worker

# Information threads générales
show thread information

# Debug services spécifiques
debug sdplane worker
```

### Arrêt
lthreads s'arrêtent avec processus principal sdplane.

## Performance lthread

### Caractéristiques Performance
- **Latence Faible** : Changements contexte <100ns
- **Mémoire Réduite** : Stack ~64KB par thread
- **CPU Efficace** : Pas de préemption OS coûteuse
- **Scalabilité** : Milliers threads possibles

### Optimisation
```bash
# Minimiser lthreads actifs pour performance
# Utiliser seulement services nécessaires

# Configuration minimale
set worker lthread stat-collector
set worker lthread rib-manager

# Configuration complète (si nécessaire)  
set worker lthread stat-collector
set worker lthread rib-manager
set worker lthread neigh-manager
set worker lthread netlink-thread
```

## Programmation lthread

### Structure lthread Typique
```c
void my_lthread_function(void *arg) {
    while (1) {
        // Traitement tâches
        process_work();
        
        // Yield coopératif pour autres threads
        lthread_yield();
        
        // Sleep si pas de travail
        if (no_work_pending) {
            lthread_sleep(100); // 100ms
        }
    }
}
```

### Synchronisation
```c
// Utiliser primitives lthread pour synchronisation
lthread_mutex_t mutex = LTHREAD_MUTEX_INITIALIZER;
lthread_cond_t cond = LTHREAD_COND_INITIALIZER;

void synchronized_operation(void) {
    lthread_mutex_lock(&mutex);
    // Section critique
    lthread_mutex_unlock(&mutex);
}
```

## Intégration Système

### Communication Inter-lthread
```c
// Files message pour communication
struct lthread_queue *msg_queue;

// Envoyer message
lthread_queue_push(msg_queue, &message);

// Recevoir message (non-bloquant)
if (lthread_queue_pop(msg_queue, &message) == 0) {
    // Traiter message
    handle_message(&message);
}
```

### Événements I/O
```c
// Attendre événements I/O sans bloquer
int fd = socket(AF_INET, SOCK_STREAM, 0);
lthread_set_nonblocking(fd);

// Lecture asynchrone
ssize_t result = lthread_recv(fd, buffer, size, 0);
if (result == -1 && errno == EAGAIN) {
    // Yield et réessayer plus tard
    lthread_yield();
}
```

## Dépannage

### Problèmes Courants
1. **lthread non démarré** : Vérifier ordre configuration
2. **Performance dégradée** : Trop de lthreads actifs
3. **Blocages** : Opérations bloquantes dans lthread
4. **Synchronisation** : Race conditions entre lthreads

### Diagnostic
```bash
# Vérifier état lthreads
show worker | grep lthread

# Debug services système
debug sdplane worker

# Surveiller performance
show thread counter
```

### Solutions
```bash
# Redémarrer services lthread (redémarrage sdplane)
sudo systemctl restart sdplane

# Réduire lthreads actifs si performance problématique
# Commenter services non essentiels dans configuration
```

## Configuration Avancée

### Paramètres lthread
Paramètres configurables (code source) :
```c
// Taille stack par lthread
#define LTHREAD_STACK_SIZE (64 * 1024)

// Nombre maximum lthreads
#define LTHREAD_MAX_COUNT 1024

// Quantum scheduling
#define LTHREAD_SCHEDULER_QUANTUM 1000
```

### Priorités lthread
Certains lthreads plus critiques :
1. **rib-manager** : Haute priorité (données partagées)
2. **stat-collector** : Priorité normale
3. **neigh-manager** : Priorité normale  
4. **netlink-thread** : Priorité faible (non critique)

## Surveillance Avancée

### Métriques lthread
```bash
# Dans show worker, chercher :
# - État threads (running/stopped)
# - Utilisation CPU approximative
# - Erreurs/warnings

show worker | grep -E "(stat-collector|rib-manager|neigh-manager)"
```

### Logs Services
```bash
# Logs spécifiques services
journalctl -u sdplane | grep -i rib
journalctl -u sdplane | grep -i neigh
journalctl -u sdplane | grep -i netlink
```

## Bonnes Pratiques

### Configuration
1. **Ordre Respect** : Suivre ordre configuration recommandé
2. **Services Essentiels** : stat-collector et rib-manager toujours requis
3. **Services Optionnels** : neigh-manager, netlink-thread selon besoins
4. **Simplicité** : Éviter services non utilisés

### Développement
1. **Coopératif** : Toujours yield appropriément
2. **Non-Bloquant** : Éviter opérations bloquantes
3. **Gestion Erreurs** : Robustesse face aux erreurs
4. **Ressources** : Libération appropriée ressources

### Surveillance
1. **État Services** : Vérifier régulièrement show worker
2. **Performance** : Surveiller impact sur performance globale
3. **Logs** : Examiner logs pour erreurs services
4. **Resources** : Surveiller utilisation mémoire lthreads

## Emplacement de Définition

lthread management défini dans :
- `sdplane/lthread_main.c` - Framework lthread principal
- `sdplane/*_manager.c` - Implémentations services spécifiques
- `lib/lthread/` - Bibliothèque lthread (externe)

## Sujets Associés

- [Gestion Workers](worker-management.md) - Configuration workers lcore vs lthread
- [RIB & Routage](routing.md) - Services rib-manager et neigh-manager
- [Interface TAP](tap-interface.md) - Service tap-handler
- [Debug & Logging](debug-logging.md) - Debug services lthread