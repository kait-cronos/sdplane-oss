# Application Répéteur Amélioré

**Language:** [English](../enhanced-repeater-application.md) | [日本語](../ja/enhanced-repeater-application.md) | **Français** | [中文](../zh/enhanced-repeater-application.md) | [Deutsch](../de/enhanced-repeater-application.md) | [Italiano](../it/enhanced-repeater-application.md) | [한국어](../ko/enhanced-repeater-application.md) | [ไทย](../th/enhanced-repeater-application.md) | [Español](../es/enhanced-repeater-application.md)

L'Application Répéteur Amélioré fournit une commutation Layer 2 VLAN-aware avancée avec intégration d'interfaces TAP pour les capacités de routage L3 et capture de paquets.

## Vue d'ensemble

Le Répéteur Amélioré est une application sophistiquée de transmission Layer 2 qui étend la transmission L2 de base avec :
- **Commutation VLAN-aware** : Support complet IEEE 802.1Q VLAN avec étiquetage/désétiquetage
- **Abstraction de Commutateur Virtuel** : Multiples commutateurs virtuels avec tables de transmission indépendantes
- **Intégration Interface TAP** : Interfaces router et capture pour intégration noyau
- **Traitement VLAN Avancé** : Capacités de translation, insertion et suppression VLAN

## Architecture

### Framework de Commutateur Virtuel
Le Répéteur Amélioré implémente une architecture de commutateur virtuel :
- **VLANs Multiples** : Support pour domaines VLAN multiples (1-4094)
- **Agrégation de Ports** : Multiples ports physiques par commutateur virtuel
- **Transmission Isolée** : Domaines de transmission indépendants par VLAN
- **Étiquetage Flexible** : Modes natif, étiqueté et translation par port

### Intégration Interface TAP
- **Interfaces Router** : Intégration pile réseau noyau pour traitement L3
- **Interfaces Capture** : Capacités de surveillance et analyse de paquets
- **Tampons Anneau** : Transfert efficace de paquets entre plan de données et noyau
- **Bidirectionnel** : Gestion de paquets en entrée et sortie

## Fonctionnalités Clés

### Traitement VLAN
- **Étiquetage VLAN** : Ajout d'en-têtes 802.1Q aux trames non étiquetées
- **Désétiquetage VLAN** : Suppression d'en-têtes 802.1Q des trames étiquetées
- **Translation VLAN** : Modification des IDs VLAN entre entrée et sortie
- **VLAN Natif** : Gestion du trafic non étiqueté sur ports trunk

### Commutation Virtuelle
- **Apprentissage** : Apprentissage automatique d'adresses MAC par VLAN
- **Inondation** : Gestion appropriée d'unicast inconnu et de diffusion
- **Horizon Divisé** : Prévention de boucles dans commutateurs virtuels
- **Multi-domaine** : Tables de transmission indépendantes par VLAN

### Traitement de Paquets
- **Zéro-copie** : Gestion efficace de paquets DPDK avec surcharge minimale
- **Traitement par Rafales** : Optimisé pour taux de paquets élevés
- **Manipulation d'En-tête** : Insertion/suppression efficace d'en-têtes VLAN
- **Optimisation de Copie** : Copie sélective de paquets pour interfaces TAP

## Configuration

### Configuration de Commutateur Virtuel
Créer des commutateurs virtuels pour différents domaines VLAN :

```bash
# Créer des commutateurs virtuels avec IDs VLAN
set vswitch 2031
set vswitch 2032
```

### Liaison Port vers VSwitch
Lier les ports DPDK physiques aux commutateurs virtuels :

```bash
# Lier port 0 au vswitch 0 avec étiquette VLAN 2031
set vswitch-link vswitch 0 port 0 tag 2031

# Lier port 0 au vswitch 1 avec natif/non-étiqueté
set vswitch-link vswitch 1 port 0 tag 0

# Exemple de translation VLAN
set vswitch-link vswitch 0 port 1 tag 2040
```

### Interfaces Router
Créer des interfaces router pour connectivité L3 :

```bash
# Créer des interfaces router pour traitement L3
set vswitch 2031 router-if rif2031
set vswitch 2032 router-if rif2032
```

### Interfaces Capture
Configurer des interfaces capture pour surveillance :

```bash
# Créer des interfaces capture pour surveillance de paquets
set vswitch 2031 capture-if cif2031
set vswitch 2032 capture-if cif2032
```

### Configuration Worker
Affecter le répéteur amélioré à un lcore :

```bash
# Définir worker répéteur amélioré
set worker lcore 1 enhanced-repeater

# Configurer affectations de files
set thread 1 port 0 queue 0
```

## Exemple de Configuration

### Configuration Complète
Voir [`example-config/sdplane_enhanced_repeater.conf`](../../example-config/sdplane_enhanced_repeater.conf) :

```bash
# Configuration périphérique
set device 03:00.0 driver vfio-pci bind

# Initialisation DPDK
set rte_eal argv -c 0x7
rte_eal_init
set mempool

# Configuration port
stop port all
set port all dev-configure 1 4
set port all nrxdesc 1024
set port all ntxdesc 1024

# Configuration worker
set worker lthread stat-collector
set worker lthread rib-manager
set worker lcore 1 enhanced-repeater
set worker lcore 2 l3-tap-handler

# Activer mode promiscuous
set port all promiscuous enable
start port all

# Configuration des files
set thread 1 port 0 queue 0

# Configuration commutateur virtuel
set vswitch 2031
set vswitch 2032

# Configuration lien
set vswitch-link vswitch 0 port 0 tag 2031
set vswitch-link vswitch 1 port 0 tag 2032

# Création d'interface
set vswitch 2031 router-if rif2031
set vswitch 2032 router-if rif2032
set vswitch 2031 capture-if cif2031
set vswitch 2032 capture-if cif2032

# Démarrer workers
start worker lcore all
```

## Détails de Traitement VLAN

### Modes d'Étiquetage

#### Mode Étiqueté
- **Entrée** : Accepte paquets avec étiquette VLAN spécifique
- **Sortie** : Maintient ou traduit l'étiquette VLAN
- **Cas d'usage** : Ports trunk, translation VLAN

#### Mode Natif (tag 0)
- **Entrée** : Accepte paquets non étiquetés
- **Sortie** : Supprime les en-têtes VLAN
- **Cas d'usage** : Ports d'accès, périphériques non-VLAN

#### Mode Translation
- **Fonction** : Change les IDs VLAN entre ports
- **Configuration** : Étiquettes différentes sur liens entrée/sortie
- **Cas d'usage** : Mappage VLAN, réseaux fournisseur de services

### Flux de Paquets

1. **Traitement d'Entrée** :
   - Recevoir paquet sur port DPDK
   - Déterminer VLAN basé sur étiquette ou configuration native
   - Rechercher commutateur virtuel de destination

2. **Recherche Commutateur Virtuel** :
   - Apprentissage et recherche d'adresses MAC
   - Déterminer ports de sortie dans domaine VLAN
   - Gérer inondation unicast/diffusion inconnu

3. **Traitement de Sortie** :
   - Manipulation d'en-tête VLAN par configuration port
   - Copie de paquet pour destinations multiples
   - Intégration interface TAP

4. **Gestion Interface TAP** :
   - Interface router : Traitement L3 noyau
   - Interface capture : Surveillance et analyse

## Surveillance et Gestion

### Commandes de Statut
```bash
# Afficher configuration commutateur virtuel
show vswitch_rib

# Afficher liens commutateur virtuel
show vswitch-link

# Voir interfaces router
show rib vswitch router-if

# Vérifier interfaces capture
show rib vswitch capture-if
```

### Statistiques et Performance
```bash
# Statistiques niveau port
show port statistics all

# Performance worker
show worker statistics

# Information thread
show thread information
```

### Commandes de Débogage
```bash
# Débogage répéteur amélioré
debug sdplane enhanced-repeater

# Débogage commutateur VLAN (type worker alternatif)
debug sdplane vlan-switch

# Débogage RIB et transmission
debug sdplane rib
debug sdplane fdb-change
```

## Cas d'Utilisation

### Agrégation VLAN
- Consolider VLANs multiples sur liens physiques uniques
- Fonctionnalité edge fournisseur
- Multiplexage de services

### Intégration L3
- Configurations router-on-a-stick
- Routage inter-VLAN via interfaces TAP
- Transmission hybride L2/L3

### Surveillance Réseau
- Capture de paquets par VLAN
- Analyse de trafic et débogage
- Surveillance de services

### Réseaux Fournisseur de Services
- Translation VLAN pour isolation client
- Réseaux multi-tenant
- Ingénierie de trafic

## Considérations de Performance

### Mise à l'Échelle
- **Affectation Worker** : Dédier lcores pour performance optimale
- **Configuration File** : Équilibrer affectations files entre cœurs
- **Pools Mémoire** : Dimensionner appropriément pour taux de paquets et besoins tampons

### Surcharge Traitement VLAN
- **Manipulation En-tête** : Surcharge minimale pour opérations VLAN
- **Copie Paquet** : Copie sélective seulement quand nécessaire
- **Optimisation Rafale** : Traiter paquets en rafales pour efficacité

### Performance Interface TAP
- **Dimensionnement Tampon Anneau** : Configurer tailles anneaux appropriées
- **Transmission Sélective** : Envoyer seulement paquets pertinents aux interfaces TAP
- **Intégration Noyau** : Considérer surcharge traitement noyau

## Dépannage

### Problèmes Courants
- **Discordance VLAN** : Vérifier configurations étiquettes correspondent à configuration réseau
- **Création interface TAP** : Assurer permissions appropriées et support noyau
- **Problèmes performance** : Vérifier affectations files et distribution workers

### Stratégies de Débogage
- **Activer journalisation débogage** : Utiliser commandes débogage pour flux détaillé paquets
- **Surveillance statistiques** : Surveiller statistiques ports et workers
- **Capture paquets** : Utiliser interfaces capture pour analyse trafic

## Documentation Associée

- [Commandes Répéteur Amélioré](enhanced-repeater.md) - Référence complète des commandes
- [Gestion Workers](worker-lcore-thread-management.md) - Détails configuration workers
- [Gestion Ports](port-management.md) - Configuration ports DPDK
- [Gestion Interface TAP](tap-interface.md) - Détails interfaces TAP