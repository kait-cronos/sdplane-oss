# Gestion des Ports & Statistiques

**Language:** [English](../port-management.md) | [Japanese](../ja/port-management.md) | **Français**

Commandes pour la gestion des ports DPDK et la surveillance des statistiques.

## Liste des Commandes

### show_port - Afficher les Informations de Port
```
show port <port-id|all>
```

Affiche les informations de configuration et d'état pour les ports DPDK spécifiés.

**Paramètres :**
- `<port-id>` : ID du port spécifique (0-7)
- `all` : Tous les ports disponibles

**Exemples :**
```bash
show port 0
show port all
```

### show_port_statistics - Afficher les Statistiques de Port
```
show port statistics <port-id|all>
```

Affiche les statistiques détaillées de trafic pour les ports DPDK.

**Exemples :**
```bash
show port statistics 0
show port statistics all
```

### start_port - Démarrer Port
```
start port <port-id|all>
```

Démarre les ports DPDK spécifiés pour traitement de paquets.

**Exemples :**
```bash
start port 0
start port all
```

### stop_port - Arrêter Port
```
stop port <port-id|all>
```

Arrête les ports DPDK spécifiés.

**Exemples :**
```bash
stop port 0
stop port all
```

### set_port_dev_configure - Configuration Périphérique Port
```
set port <port-id|all> dev-configure <nrxq> <ntxq>
```

Configure le nombre de files RX et TX pour les ports.

**Paramètres :**
- `<nrxq>` : Nombre de files de réception
- `<ntxq>` : Nombre de files de transmission

**Exemple :**
```bash
set port all dev-configure 1 4
```

### set_port_nrxdesc - Définir Descripteurs RX
```
set port <port-id|all> nrxdesc <nombre>
```

Définit le nombre de descripteurs RX par file.

**Exemple :**
```bash
set port all nrxdesc 1024
```

### set_port_ntxdesc - Définir Descripteurs TX
```
set port <port-id|all> ntxdesc <nombre>
```

Définit le nombre de descripteurs TX par file.

**Exemple :**
```bash
set port all ntxdesc 1024
```

### set_port_promiscuous - Mode Promiscuous
```
set port <port-id|all> promiscuous <enable|disable>
```

Active ou désactive le mode promiscuous pour les ports.

**Exemples :**
```bash
set port all promiscuous enable
set port 0 promiscuous disable
```

### update_port_status - Mettre à Jour État Port
```
update port status
```

Met à jour manuellement les informations d'état des ports (utile pour NICs Mellanox).

## Gestion des Ports

### Cycle de Vie des Ports
1. **Initialisation** : Configuration via `rte_eal_init`
2. **Configuration** : Définir files RX/TX avec `dev-configure`
3. **Configuration Descripteurs** : Définir tailles tampons RX/TX
4. **Démarrage** : Activer ports avec `start port`
5. **Opération** : Traitement actif de paquets
6. **Arrêt** : Désactiver ports avec `stop port`

### Configuration Typique des Ports
```bash
# Arrêter tous les ports
stop port all

# Configurer files et descripteurs
set port all dev-configure 1 4
set port all nrxdesc 1024
set port all ntxdesc 1024

# Activer mode promiscuous si nécessaire
set port all promiscuous enable

# Démarrer tous les ports
start port all
```

## Surveillance des Statistiques

### Métriques de Port
- **Paquets RX/TX** : Compteurs de paquets reçus et transmis
- **Octets RX/TX** : Compteurs d'octets de trafic
- **Erreurs** : Erreurs de réception, transmission et suppression
- **État Lien** : Statut lien, vitesse et duplex

### Surveillance en Temps Réel
```bash
# Surveiller toutes les statistiques de ports
watch -n 1 "show port statistics all"

# Surveillance port spécifique
show port statistics 0
```

## Configuration Avancée

### Optimisation de Performance
- **Tailles Descripteurs** : Équilibrer utilisation mémoire et performance
- **Configuration Files** : Optimiser nombre de files pour charge de travail
- **Mode Promiscuous** : Activer seulement si nécessaire

### Configuration NUMA
- Aligner ports sur nœuds NUMA locaux
- Utiliser mémoire locale pour tampons
- Assigner workers aux cœurs appropriés

## Dépannage

### Problèmes Courants de Ports
1. **Port ne démarre pas** : Vérifier configuration périphérique et pilotes
2. **Performance faible** : Vérifier configuration files et descripteurs
3. **Erreurs de lien** : Vérifier câblage et compatibilité

### Commandes de Diagnostic
```bash
# Vérifier état des ports
show port all

# Surveiller erreurs
show port statistics all

# Mettre à jour état (pour Mellanox)
update port status
```

## Emplacement de Définition

Ces commandes sont définies dans :
- `sdplane/dpdk_port_cmd.c` - Implémentations commandes de port
- `sdplane/rib.h` - Structures de données de port

## Sujets Associés

- [Gestion Workers](worker-management.md) - Configuration workers et affectation ports
- [Configuration Files](queue-configuration.md) - Configuration détaillée des files
- [RIB & Routage](routing.md) - Informations état des ports dans RIB