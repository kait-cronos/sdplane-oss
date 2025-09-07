# Débogage & Journalisation

**Language:** [English](../en/debug-logging.md) | [日本語](../ja/debug-logging.md) | **Français** | [中文](../zh/debug-logging.md) | [Deutsch](../de/debug-logging.md) | [Italiano](../it/debug-logging.md) | [한국어](../ko/debug-logging.md) | [ไทย](../th/debug-logging.md) | [Español](../es/debug-logging.md)

Commandes pour contrôler les fonctions de débogage et de journalisation de sdplane.

## Liste des Commandes

### log_file - Configuration de Sortie vers Fichier Journal
```
log file <chemin-fichier>
```

Configure la journalisation pour la sortie vers un fichier.

**Paramètres :**
- `<chemin-fichier>` - Chemin vers le fichier de journal de sortie

**Exemples :**
```bash
# Sortie des journaux vers le fichier spécifié
log file /var/log/sdplane.log

# Fichier de journal de débogage
log file /tmp/sdplane-debug.log
```

### log_stdout - Configuration de Journal Sortie Standard
```
log stdout
```

Configure la journalisation pour la sortie vers la sortie standard.

**Exemples :**
```bash
# Afficher les journaux sur la sortie standard
log stdout
```

**Note :** `log file` et `log stdout` peuvent être configurés simultanément, et les journaux seront affichés aux deux destinations.

### debug - Configuration de Débogage
```
debug <catégorie> <cible>
```

Active la journalisation de débogage pour des cibles spécifiques dans la catégorie spécifiée.

**Catégories :**
- `sdplane` - Catégorie principale sdplane
- `zcmdsh` - Catégorie shell de commande

**Liste des Cibles sdplane :**
- `lthread` - Threads légers
- `console` - Console
- `tap-handler` - Gestionnaire TAP
- `l2fwd` - Transmission L2
- `l3fwd` - Transmission L3
- `vty-server` - Serveur VTY
- `vty-shell` - Shell VTY
- `stat-collector` - Collecteur de statistiques
- `packet` - Traitement de paquets
- `fdb` - FDB (Base de Données de Transmission)
- `fdb-change` - Changements FDB
- `rib` - RIB (Base d'Informations de Routage)
- `vswitch` - Commutateur virtuel
- `vlan-switch` - Commutateur VLAN
- `pktgen` - Générateur de paquets
- `enhanced-repeater` - Répéteur amélioré
- `netlink` - Interface Netlink
- `neighbor` - Gestion des voisins
- `all` - Toutes les cibles

**Exemples :**
```bash
# Activer le débogage pour des cibles spécifiques
debug sdplane rib
debug sdplane fdb-change
debug sdplane pktgen

# Activer tout le débogage sdplane
debug sdplane all

# Débogage de la catégorie zcmdsh
debug zcmdsh shell
debug zcmdsh command
```

### no debug - Désactivation de Débogage
```
no debug <catégorie> <cible>
```

Désactive la journalisation de débogage pour des cibles spécifiques dans la catégorie spécifiée.

**Exemples :**
```bash
# Désactiver le débogage pour des cibles spécifiques
no debug sdplane rib
no debug sdplane fdb-change

# Désactiver tout le débogage sdplane (recommandé)
no debug sdplane all

# Désactiver le débogage de la catégorie zcmdsh
no debug zcmdsh all
```

### show_debug_sdplane - Afficher les Informations de Débogage sdplane
```
show debugging sdplane
```

Affiche la configuration actuelle du débogage sdplane.

**Exemples :**
```bash
show debugging sdplane
```

Cette commande affiche les informations suivantes :
- Cibles de débogage actuellement activées
- Statut de débogage pour chaque cible
- Options de débogage disponibles

## Vue d'ensemble du Système de Débogage

Le système de débogage sdplane dispose des fonctionnalités suivantes :

### Débogage par Catégorie
- Les catégories de débogage sont séparées par différents modules fonctionnels
- Vous pouvez activer les journaux de débogage uniquement pour les fonctions nécessaires

### Contrôle par Cible
- Les messages de débogage sont classés par type de cible
- Vous pouvez afficher uniquement les informations nécessaires en définissant des cibles appropriées

### Configuration Dynamique
- La configuration de débogage peut être modifiée pendant que le système fonctionne
- Les cibles de débogage peuvent être ajustées sans redémarrage

## Utilisation

### 1. Configurer la Sortie des Journaux
```bash
# Configurer la sortie vers fichier journal (recommandé)
log file /var/log/sdplane.log

# Configurer la sortie standard
log stdout

# Activer les deux (pratique pour le débogage)
log file /var/log/sdplane.log
log stdout
```

### 2. Vérifier la Configuration de Débogage Actuelle
```bash
show debugging sdplane
```

### 3. Vérifier les Cibles de Débogage
Utilisez la commande `show debugging sdplane` pour vérifier les cibles disponibles et leur statut.

### 4. Modifier la Configuration de Débogage
```bash
# Activer le débogage pour des cibles spécifiques
debug sdplane rib
debug sdplane fdb-change

# Activer toutes les cibles en une fois
debug sdplane all
```

### 5. Vérifier les Journaux de Débogage
Les journaux de débogage sont envoyés vers les destinations configurées (fichier ou sortie standard).

## Dépannage

### Quand les Journaux de Débogage ne s'Affichent Pas
1. Vérifiez si la sortie des journaux est configurée (`log file` ou `log stdout`)
2. Vérifiez si les cibles de débogage sont correctement configurées (`debug sdplane <cible>`)
3. Vérifiez le statut de débogage actuel (`show debugging sdplane`)
4. Vérifiez l'espace disque du fichier journal et les permissions

### Gestion des Fichiers Journaux
```bash
# Vérifier la taille du fichier journal
ls -lh /var/log/sdplane.log

# Suivre le fichier journal
tail -f /var/log/sdplane.log

# Vérifier l'emplacement du fichier journal (exemple de fichier de configuration)
grep "log file" /etc/sdplane/sdplane.conf
```

### Impact sur les Performances
- L'activation des journaux de débogage peut impacter les performances
- Il est recommandé d'activer uniquement un débogage minimal dans les environnements de production
- Effectuez une rotation régulière des fichiers journaux pour éviter qu'ils deviennent trop volumineux

## Exemples de Configuration

### Configuration de Journal de Base
```bash
# Exemple de fichier de configuration (/etc/sdplane/sdplane.conf)
log file /var/log/sdplane.log
log stdout

# Activer le débogage au démarrage du système
debug sdplane rib
debug sdplane fdb-change
```

### Configuration de Débogage
```bash
# Configuration détaillée des journaux de débogage
log file /tmp/sdplane-debug.log
log stdout

# Activer tout le débogage des cibles (développement uniquement)
debug sdplane all

# Activer seulement des cibles spécifiques
debug sdplane rib
debug sdplane fdb-change
debug sdplane vswitch
```

### Configuration Environnement de Production
```bash
# Journalisation standard uniquement en production
log file /var/log/sdplane.log

# Activer seulement les cibles critiques si nécessaire
# debug sdplane fdb-change
# debug sdplane rib
```

### Opérations de Nettoyage de Débogage
```bash
# Désactiver tout le débogage
no debug sdplane all
no debug zcmdsh all
```

## Emplacement de Définition

Ces commandes sont définies dans le fichier suivant :
- `sdplane/debug_sdplane.c`

## Sujets Connexes

- [Informations Système & Surveillance](system-monitoring.md)
- [Gestion VTY & Shell](vty-shell.md)