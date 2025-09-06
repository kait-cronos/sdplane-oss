# Gestion VTY & Shell

**Language:** [English](../vty-shell.md) | [日本語](../ja/vty-shell.md) | **Français** | [中文](../zh/vty-shell.md) | [Deutsch](../de/vty-shell.md) | [Italiano](../it/vty-shell.md) | [한국어](../ko/vty-shell.md) | [ไทย](../th/vty-shell.md) | [Español](../es/vty-shell.md)

Gestion de l'interface terminal virtuel (VTY) et des sessions shell pour sdplane.

## Vue d'ensemble

Le système VTY/Shell de sdplane fournit une interface ligne de commande interactive pour configuration, surveillance et gestion du système :

- **Interface VTY** : Terminal virtuel pour accès distant
- **Shell Interactif** : Interface ligne de commande locale
- **Complétion Commandes** : Auto-complétion et aide contextuelle
- **Sessions Multiples** : Support connections utilisateurs multiples
- **Historique** : Historique commandes et navigation

## Accès Shell

### Shell Local
Accès direct via application sdplane :
```bash
# Démarrer sdplane (mode interactif par défaut)
sudo ./sdplane/sdplane

# Interface CLI disponible immédiatement
sdplane> show version
sdplane> help
```

### Shell Distant (VTY)
Accès via connexion telnet :
```bash
# Connexion VTY (port par défaut 9882)
telnet localhost 9882

# Ou depuis machine distante
telnet <sdplane-host> 9882
```

### Configuration Port VTY
```bash
# Configuration port d'écoute VTY (défaut : 9882)
# Modifiable dans fichier configuration ou code source
```

## Fonctionnalités Shell

### Complétion Commandes
- **Tab** : Complétion automatique commandes
- **Double Tab** : Affichage options disponibles
- **Complétion Contextuelle** : Suggère paramètres appropriés
- **Validation Syntaxe** : Vérification syntaxe en temps réel

### Navigation Historique
```bash
# Flèches haut/bas : Navigation historique
↑ / ↓ : Commandes précédentes/suivantes

# Ctrl+R : Recherche dans historique
# Ctrl+A : Début de ligne
# Ctrl+E : Fin de ligne
```

### Aide Intégrée
```bash
# Aide générale
help

# Aide commande spécifique
help show
help set worker

# Aide contextuelle avec ?
show ?
set worker ?
```

## Gestion Sessions

### Sessions Multiples
Le système VTY supporte sessions simultanées :
- **Sessions Locales** : Shell application principal
- **Sessions VTY** : Connexions telnet multiples
- **Isolation** : Sessions indépendantes avec contexte propre
- **Limite Connexions** : Configurable pour sécurité

### Information Session
```bash
# Afficher sessions actives
show sessions

# Information session courante
show whoami
```

### Déconnexion
```bash
# Quitter session
exit
quit

# Fermeture forcée
Ctrl+C
Ctrl+D
```

## Commandes Shell

### Commandes Système Shell
```bash
# Information système
show version        # Version sdplane
show system        # État système
show memory        # Utilisation mémoire

# Navigation aide
help               # Liste commandes disponibles
?                  # Aide contextuelle
list               # Liste commandes (si disponible)
```

### Commandes Configuration
```bash
# Configuration workers
set worker lcore <id> <type>
start worker lcore <id|all>
show worker

# Configuration ports
set port <id|all> <paramètre> <valeur>
show port <id|all>
```

### Commandes Debug
```bash
# Activation debug
debug sdplane <composant>

# Information détaillée
show rib
show thread information
show port statistics all
```

## Personnalisation Shell

### Configuration Prompt
Le prompt peut être personnalisé :
```
sdplane>           # Prompt par défaut
sdplane(config)>   # Mode configuration
sdplane#           # Mode privilégié (si implémenté)
```

### Configuration Historique
- **Taille Historique** : Nombre commandes mémorisées
- **Persistance** : Sauvegarde historique entre sessions
- **Partage** : Historique partagé ou par session

### Paramètres Terminal
```bash
# Configuration terminal via variables environnement
export TERM=xterm-256color
export COLUMNS=120
export LINES=40
```

## Mode Configuration

### Modes Opérationnels
Le shell peut avoir différents modes :

```bash
# Mode normal (lecture/affichage)
sdplane> show port all

# Mode configuration (modifications)
sdplane> configure
sdplane(config)> set worker lcore 1 l2-repeater
sdplane(config)> exit
```

### Validation Configuration
- **Vérification Syntaxe** : Validation avant application
- **Rollback** : Annulation changements en cas d'erreur
- **Confirmation** : Confirmation changements critiques

## Sécurité VTY

### Contrôle Accès
```bash
# Restriction accès par IP (configuration système)
# allow-host 192.168.1.0/24
# deny-host all

# Authentification (si implémentée)
# username admin password secret
```

### Limitations Sécurité
- **Pas d'authentification par défaut** : Accès libre via telnet
- **Trafic Non Chiffré** : Communications telnet en clair
- **Privilèges Système** : Sessions héritent privilèges sdplane

### Bonnes Pratiques
- Limiter accès VTY aux réseaux de confiance
- Utiliser firewall pour protéger port VTY
- Surveiller connexions actives
- Considérer tunnel SSH pour accès distant sécurisé

## Surveillance Shell

### Logs Sessions
```bash
# Logs connexions VTY
journalctl -u sdplane | grep -i vty

# Logs commandes (si activé)
tail -f /var/log/sdplane-shell.log
```

### Métriques Sessions
- **Connexions Actives** : Nombre sessions en cours
- **Historique Connexions** : Log connexions/déconnexions
- **Utilisation Commandes** : Statistiques utilisation
- **Erreurs** : Commandes échouées et syntaxe invalide

## Dépannage

### Problèmes Connexion VTY
1. **Port inaccessible** : Vérifier firewall et écoute port
2. **Connexion refusée** : Vérifier sdplane actif et port correct
3. **Déconnexions fréquentes** : Vérifier stabilité réseau
4. **Performance lente** : Vérifier charge système

### Diagnostic
```bash
# Vérifier port d'écoute VTY
netstat -tuln | grep 9882

# Test connexion locale
telnet localhost 9882

# Logs système
journalctl -u sdplane -f

# Processus sdplane actif
ps aux | grep sdplane
```

### Problèmes Shell
1. **Complétion non fonctionnelle** : Vérifier configuration terminal
2. **Historique perdu** : Vérifier permissions fichiers
3. **Aide manquante** : Vérifier installation complète
4. **Commandes non reconnues** : Vérifier version et modules

## Intégration Automatisation

### Scripts Automation
```bash
#!/bin/bash
# Script configuration automatique via VTY

{
    echo "set worker lcore 1 l2-repeater"
    echo "set worker lcore 2 enhanced-repeater" 
    echo "start worker lcore all"
    echo "show worker"
    echo "exit"
} | telnet localhost 9882
```

### Expect Scripts
```tcl
#!/usr/bin/expect
spawn telnet localhost 9882
expect "sdplane>"
send "show version\r"
expect "sdplane>"
send "exit\r"
```

### API Intégration
Pour automation avancée, considérer :
- Scripts expect pour interactions automatiques
- Parsing sortie pour extraction données
- Monitoring via scripts périodiques
- Intégration systèmes surveillance

## Configuration Avancée

### Paramètres VTY
Configuration dans code source ou fichier config :
```c
// Port d'écoute VTY
#define VTY_PORT 9882

// Nombre maximum connexions simultanées  
#define MAX_VTY_CONNECTIONS 10

// Timeout session inactive
#define VTY_TIMEOUT 1800
```

### Personnalisation Interface
- Messages d'accueil personnalisés
- Bannières de sécurité
- Configuration couleurs terminal
- Layouts personnalisés pour différents modes

## Emplacement de Définition

VTY/Shell défini dans :
- `sdplane/vty_shell.c` - Implémentation VTY
- `sdplane/console_shell.c` - Shell console
- `lib/sdplane/cmd*.c` - Définitions commandes

## Sujets Associés

- [Debug & Logging](debug-logging.md) - Debug via interface shell
- [Surveillance Système](system-monitoring.md) - Commandes surveillance
- [Gestion Workers](worker-management.md) - Configuration via shell
- [Gestion Ports](port-management.md) - Commandes port via shell