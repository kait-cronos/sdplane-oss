# Information Système & Surveillance

**Language:** [English](../system-monitoring.md) | [Japanese](../ja/system-monitoring.md) | **Français**

Commandes pour la surveillance du système et la collecte d'informations de performance.

## Liste des Commandes

### show_version - Afficher Version
```
show version
```

Affiche les informations de version du système sdplane.

### show_memory - Afficher Informations Mémoire
```
show memory
```

Affiche l'utilisation mémoire et les statistiques des pools mémoire DPDK.

### show_system - Afficher Informations Système
```
show system
```

Affiche les informations système générales incluant CPU, mémoire et configuration.

### show_lcore - Afficher Informations lcore
```
show lcore
```

Affiche la configuration et l'état des lcores (cœurs logiques).

### show_ring - Afficher Informations Anneau
```
show ring
```

Affiche les statistiques et configuration des anneaux DPDK.

## Surveillance Mémoire

### Pools Mémoire DPDK
Les pools mémoire sont cruciaux pour performance DPDK :
- **Allocation** : Pré-allocation de tampons paquets
- **Utilisation** : Surveillance occupation pools
- **Performance** : Impact sur débit paquets

### Métriques Mémoire Clés
```bash
# Afficher utilisation mémoire détaillée
show memory

# Informations incluent :
# - Taille pool total
# - Tampons disponibles/utilisés
# - Statistiques allocation/libération
# - Informations hugepage
```

## Surveillance CPU et lcores

### Configuration lcore
```bash
# Afficher configuration lcore
show lcore

# Informations incluent :
# - État lcore (maître/esclave)
# - Affectations workers
# - Utilisation CPU
# - Configuration thread
```

### Métriques Performance
- **Utilisation lcore** : Pourcentage temps CPU utilisé
- **Changements contexte** : Commutations entre threads
- **Cycles CPU** : Cycles dépensés par worker
- **Efficacité** : Rapport travail utile/cycles totaux

## Surveillance Anneaux

### Anneaux DPDK
Les anneaux sont utilisés pour communication entre threads :
- **Anneaux RX/TX** : Transfer paquets entre workers
- **Anneaux TAP** : Communication avec interfaces TAP
- **Anneaux Contrôle** : Messages de contrôle système

### Statistiques Anneaux
```bash
# Afficher statistiques anneaux
show ring

# Métriques incluent :
# - Taille anneau
# - Éléments utilisés/libres
# - Taux production/consommation
# - Débordements/sous-débordements
```

## Information Système

### Configuration Système
```bash
# Afficher configuration système complète
show system

# Informations incluent :
# - Configuration DPDK
# - Information processeur
# - Configuration mémoire
# - État interfaces réseau
```

### Métriques Système Clés
- **Charge CPU** : Utilisation globale système
- **Utilisation Mémoire** : RAM système et hugepages
- **Information Réseau** : État interfaces et statistiques
- **Températures** : Températures CPU si disponibles

## Surveillance Performance

### Métriques en Temps Réel
```bash
# Surveillance continue système
watch -n 1 "show system"

# Surveillance mémoire
watch -n 1 "show memory"

# Surveillance lcores
watch -n 1 "show lcore"
```

### Collecte Statistiques
Le collecteur de statistiques (`stat-collector`) rassemble métriques :
- Configuration automatique via `set worker lthread stat-collector`
- Collecte périodique de métriques système
- Agrégation données performance

## Optimisation Performance

### Surveillance Ressources
1. **Mémoire** : Surveiller utilisation pools pour éviter épuisement
2. **CPU** : Équilibrer charge entre lcores
3. **Anneaux** : Éviter débordements avec dimensionnement approprié
4. **Réseau** : Surveiller statistiques ports pour goulots

### Bonnes Pratiques
- Surveiller régulièrement métriques clés
- Configurer seuils alertes pour ressources critiques
- Utiliser surveillance continue pendant tests charge
- Ajuster configuration basée sur métriques observées

## Scripts de Surveillance

### Surveillance Automatisée
```bash
#!/bin/bash
# Script surveillance système sdplane

while true; do
    echo "=== Système ==="
    show system
    echo "=== Mémoire ==="
    show memory
    echo "=== lcores ==="
    show lcore
    sleep 5
done
```

### Collecte Logs
```bash
# Rediriger sortie vers fichier log
{
    echo "Timestamp: $(date)"
    show system
    show memory
    show lcore
} >> /var/log/sdplane-monitoring.log
```

## Alertes et Seuils

### Seuils Recommandés
- **Utilisation Mémoire** : < 80% pool occupé
- **Charge CPU** : < 90% utilisation lcore
- **Débordements Anneaux** : Zéro débordements tolérable
- **Erreurs Ports** : Taux erreurs < 0.01%

### Surveillance Proactive
- Configurer surveillance automatique métriques critiques
- Implémenter alertes basées seuils
- Logs réguliers pour analyse tendances
- Corrélation métriques avec charge application

## Dépannage

### Problèmes Performance Courants
1. **Utilisation mémoire élevée** : Vérifier tailles pools
2. **Charge CPU élevée** : Redistribuer workers
3. **Débordements anneaux** : Augmenter tailles anneaux
4. **Contentions ressources** : Optimiser affectations

### Outils Diagnostic
```bash
# Diagnostics système complets
show version    # Version logiciel
show system     # État système
show memory     # Utilisation mémoire
show lcore      # Configuration CPU
show ring       # État anneaux
```

## Emplacement de Définition

Ces commandes sont définies dans :
- `sdplane/system_cmd.c` - Commandes information système
- `sdplane/memory_cmd.c` - Commandes surveillance mémoire
- `sdplane/lcore_cmd.c` - Commandes lcore

## Sujets Associés

- [Gestion Workers](worker-management.md) - Configuration workers et surveillance
- [Gestion Ports](port-management.md) - Surveillance ports et statistiques
- [Information Threads](thread-information.md) - Détails surveillance threads
- [RIB & Routage](routing.md) - Surveillance état système dans RIB