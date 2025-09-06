# Guide Utilisateur sdplane-oss

**Language:** [English](../README.md) | [Japanese](../ja/README.md) | **Français**

Ce guide fournit une référence complète pour utiliser sdplane-oss, un routeur logiciel haute performance construit sur DPDK.

## Vue d'ensemble

sdplane-oss est un environnement de développement DPDK-dock offrant :
- Traitement de paquets haute performance via DPDK
- Transmission Layer 2/3 avec support VLAN
- Génération de paquets intégrée pour tests
- Interface ligne de commande interactive
- Support de virtualisation réseau avec interfaces TAP

## Organisation des Commandes

### Commandes de Configuration Système
- **Gestion des Ports** - Configuration des ports DPDK, état des liens et statistiques
- **Gestion des Workers** - Affectation des threads workers et configuration lcore
- **Configuration des Files** - Configuration des files RX/TX et gestion
- **Gestion des Périphériques** - Configuration des pilotes et liaison PCIe

### Applications de Traitement de Paquets
- **Répéteur L2** - Transmission simple Layer 2 avec apprentissage MAC
- **Répéteur Amélioré** - Commutation VLAN avec interfaces TAP
- **Générateur de Paquets** - Génération de trafic et outils de benchmarking

### Surveillance et Débogage
- **Information Système** - État système et informations de performance
- **RIB et Routage** - Base d'informations de routage et gestion des ressources
- **Débogage et Journalisation** - Sortie de débogage et contrôles de journalisation

### Interfaces et Virtualisation
- **Interface TAP** - Intégration avec la pile réseau du noyau Linux
- **VTY et Shell** - Interface terminal et gestion des sessions
- **Threading** - Gestion des threads lthread et surveillance

## Guides Spécialisés

### Guides d'Application
Guides détaillés pour chaque application de traitement de paquets :
- Applications L2/L3 forwarding
- Génération et test de paquets
- Commutation VLAN et interfaces TAP

### Guides de Configuration
Instructions de configuration détaillées pour :
- Configuration des ports DPDK et des files
- Affectation des workers et optimisation des performances
- Intégration réseau et interfaces TAP

### Guide d'Intégration Développeur
- Intégration d'applications DPDK via l'approche DPDK-dock
- Modèles de threading coopératif
- Accès thread-safe aux données avec RCU

## Démarrage Rapide

1. **Installation** - Installez les dépendances et compilez sdplane
2. **Configuration** - Configurez hugepages et liez les NICs
3. **Démarrage** - Lancez sdplane et connectez-vous au CLI
4. **Configuration** - Configurez les ports, workers et applications
5. **Surveillance** - Utilisez les commandes d'état et statistiques

Pour des instructions d'installation détaillées, voir le [README principal](../../README.fr.md).

## Support

Pour questions et support :
- **Email** : sdplane [at] nwlab.org
- **Documentation** : Guides complets dans doc/manual/fr/
- **Code Source** : Disponible sous licence MIT

## Navigation des Documents

Utilisez les liens de navigation pour accéder aux guides spécifiques :
- **Guides d'Application** - Applications de traitement de paquets
- **Guides de Configuration** - Configuration système et réseau  
- **Guide Développeur** - Intégration et développement d'applications