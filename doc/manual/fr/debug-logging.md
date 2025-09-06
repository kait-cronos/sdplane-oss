# Débogage & Journalisation

**Language:** [English](../debug-logging.md) | [日本語](../ja/debug-logging.md) | **Français** | [中文](../zh/debug-logging.md) | [Deutsch](../de/debug-logging.md) | [Italiano](../it/debug-logging.md) | [한국어](../ko/debug-logging.md) | [ไทย](../th/debug-logging.md) | [Español](../es/debug-logging.md)

Fonctions de débogage et contrôles de journalisation pour le développement et le dépannage.

## Commandes Debug

### debug sdplane - Debug Système
```
debug sdplane <composant>
```

Active la journalisation debug pour composants système spécifiques.

**Composants disponibles :**
- `l2-repeater` : Débogage répéteur L2
- `enhanced-repeater` : Débogage répéteur amélioré
- `vlan-switch` : Débogage commutateur VLAN
- `pktgen` : Débogage générateur de paquets
- `rib` : Débogage RIB
- `fdb-change` : Changements base données transmission
- `tap` : Interfaces TAP
- `worker` : Gestion workers

**Exemples :**
```bash
debug sdplane l2-repeater
debug sdplane enhanced-repeater
debug sdplane rib
debug sdplane fdb-change
```

## Niveaux de Journalisation

### Contrôle Verbosité
La journalisation debug fournit différents niveaux de détail :

- **ERROR** : Erreurs critiques seulement
- **WARN** : Avertissements et erreurs
- **INFO** : Messages informatifs généraux
- **DEBUG** : Informations détaillées debug
- **TRACE** : Journalisation trace très détaillée

### Configuration Journalisation
```bash
# Activer journalisation détaillée pour composant spécifique
debug sdplane enhanced-repeater

# Combinaisons multiples pour diagnostics complets
debug sdplane rib
debug sdplane fdb-change
debug sdplane tap
```

## Composants Debug

### L2 Repeater Debug
```bash
debug sdplane l2-repeater
```
**Informations journalisées :**
- Flux transmission de paquets
- Décisions transmission port-à-port
- Statistiques traitement paquets
- Détection erreurs transmission

### Enhanced Repeater Debug
```bash
debug sdplane enhanced-repeater
debug sdplane vlan-switch
```
**Informations journalisées :**
- Traitement étiquettes VLAN
- Décisions commutation virtuelle
- Création/suppression interfaces TAP
- Flux transmission multi-port

### RIB Debug
```bash
debug sdplane rib
```
**Informations journalisées :**
- Mises à jour configuration RIB
- Changements état ports
- Synchronisation données inter-threads
- Opérations RCU

### Debug Base Données Transmission
```bash
debug sdplane fdb-change
```
**Informations journalisées :**
- Apprentissage adresses MAC
- Expiration entrées FDB
- Mises à jour tables transmission
- Changements topologie réseau

### Debug Interface TAP
```bash
debug sdplane tap
```
**Informations journalisées :**
- Création/destruction interfaces TAP
- Transfert paquets kernel-userspace
- Configuration interface TAP
- Erreurs opérations TAP

### Debug Worker
```bash
debug sdplane worker
```
**Informations journalisées :**
- Démarrage/arrêt workers
- Affectations lcore
- Performance threads
- Gestion cycle vie workers

## Techniques Débogage

### Débogage Flux Paquets
Pour tracer flux paquets à travers système :

```bash
# Activer débogage complet transmission
debug sdplane l2-repeater
debug sdplane enhanced-repeater
debug sdplane fdb-change

# Surveiller journaux pendant test trafic
tail -f /var/log/sdplane.log
```

### Débogage Performance
Pour analyser problèmes performance :

```bash
# Debug système et workers
debug sdplane worker
debug sdplane rib

# Surveiller métriques avec debug actif
show worker
show thread counter
```

### Débogage Configuration
Pour résoudre problèmes configuration :

```bash
# Debug composants configuration
debug sdplane rib
debug sdplane tap

# Vérifier état après changements configuration
show rib
show vswitch_rib
```

## Analyse Journaux

### Format Messages Debug
Messages debug incluent généralement :
- **Timestamp** : Horodatage précis
- **Composant** : Source message (l2-repeater, rib, etc.)
- **Niveau** : Niveau gravité (ERROR, WARN, INFO, DEBUG)
- **Message** : Détails événement ou état

### Filtrage Journaux
```bash
# Filtrer par composant
grep "l2-repeater" /var/log/sdplane.log

# Filtrer par niveau
grep "ERROR" /var/log/sdplane.log

# Surveillance temps réel
tail -f /var/log/sdplane.log | grep "enhanced-repeater"
```

## Débogage Avancé

### Debug Multi-Composant
Pour problèmes complexes impliquant plusieurs composants :

```bash
# Activer debug multiple
debug sdplane enhanced-repeater
debug sdplane rib
debug sdplane fdb-change
debug sdplane tap

# Analyser journaux corrélés
journalctl -f -u sdplane
```

### Debug Performance Détaillé
Pour analyse performance approfondie :

```bash
# Debug avec métriques détaillées
debug sdplane worker

# Surveillance continue performance
watch -n 1 "show thread counter"

# Corrélation avec journaux debug
tail -f /var/log/sdplane.log &
show worker
```

### Debug Réseau
Pour diagnostiquer problèmes réseau :

```bash
# Debug transmission et interfaces
debug sdplane enhanced-repeater
debug sdplane tap

# Capture paquets simultanée
tcpdump -i rif2031 &  # Interface TAP
# Analyser journaux debug pendant capture
```

## Optimisation Debug

### Impact Performance
La journalisation debug impacte performance :
- **Surcharge CPU** : Traitement messages debug
- **I/O Disque** : Écriture fichiers journaux
- **Latence** : Délais supplémentaires traitement

### Utilisation Sélective
```bash
# Activer debug seulement pour composants nécessaires
debug sdplane enhanced-repeater  # Spécifique seulement

# Éviter debug généralisé en production
# debug sdplane worker  # Éviter si non nécessaire
```

### Rotation Journaux
Configuration appropriée rotation journaux :
```bash
# Configuration logrotate pour journaux sdplane
/etc/logrotate.d/sdplane:
/var/log/sdplane.log {
    daily
    rotate 7
    compress
    missingok
    create 0644 root root
}
```

## Dépannage Common

### Pas de Messages Debug
1. **Vérifier activation** : Confirmer commandes debug exécutées
2. **Vérifier destination logs** : Localiser fichiers journaux appropriés
3. **Permissions fichiers** : S'assurer permissions écriture appropriées

### Trop de Messages Debug
1. **Désactiver debug non nécessaire** : Limiter aux composants pertinents
2. **Ajuster niveaux journalisation** : Utiliser niveaux moins verbeux
3. **Rotation journaux** : Configurer nettoyage automatique

### Performance Dégradée
1. **Limiter debug actif** : Désactiver debug non critique
2. **Surveillance ressources** : Vérifier usage CPU/disque
3. **Debug sélectif** : Activer seulement pendant investigations

## Outils Externes

### Intégration Systemd
```bash
# Journaux via systemd
journalctl -u sdplane -f

# Filtrage par priorité
journalctl -u sdplane -p err
```

### Outils Surveillance
- **htop** : Surveillance CPU threads
- **iotop** : Surveillance I/O disque
- **tcpdump** : Capture paquets réseau
- **wireshark** : Analyse paquets

## Emplacement de Définition

Fonctions debug définies dans :
- `lib/sdplane/debug.c` - Infrastructure debug
- Composants individuels - Implémentations debug spécifiques

## Sujets Associés

- [Information Système](system-monitoring.md) - Surveillance système
- [Gestion Workers](worker-management.md) - Debug workers
- [Interface TAP](tap-interface.md) - Debug interfaces TAP
- [RIB & Routage](routing.md) - Debug données système