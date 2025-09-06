# Configuration du Répéteur Amélioré

**Language:** [English](../enhanced-repeater.md) | [Japanese](../ja/enhanced-repeater.md) | **Français**

Le répéteur amélioré fournit des capacités de commutation VLAN avancées avec interfaces TAP pour routage L3 et capture de paquets. Il supporte la création de commutateurs virtuels, la liaison de ports DPDK avec étiquetage VLAN, les interfaces router pour intégration noyau, et les interfaces capture pour surveillance.

## Commandes de Commutateur Virtuel

### set vswitch
```
set vswitch <1-4094>
```
**Description** : Crée un commutateur virtuel avec l'ID VLAN spécifié

**Paramètres** :
- `<1-4094>` : ID VLAN pour le commutateur virtuel

**Exemple** :
```
set vswitch 2031
set vswitch 2032
```

### delete vswitch
```
delete vswitch <0-3>
```
**Description** : Supprime un commutateur virtuel par ID

**Paramètres** :
- `<0-3>` : ID du commutateur virtuel à supprimer

**Exemple** :
```
delete vswitch 0
```

### show vswitch_rib
```
show vswitch_rib
```
**Description** : Affiche les informations RIB des commutateurs virtuels incluant configuration et état

## Commandes de Lien de Commutateur Virtuel

### set vswitch-link
```
set vswitch-link vswitch <0-3> port <0-7> tag <0-4094>
```
**Description** : Lie un port DPDK à un commutateur virtuel avec configuration d'étiquetage VLAN

**Paramètres** :
- `vswitch <0-3>` : ID du commutateur virtuel (0-3)
- `port <0-7>` : ID du port DPDK (0-7)
- `tag <0-4094>` : ID d'étiquette VLAN (0: natif/non-étiqueté, 1-4094: VLAN étiqueté)

**Exemples** :
```
# Lier le port 0 au vswitch 0 avec l'étiquette VLAN 2031
set vswitch-link vswitch 0 port 0 tag 2031

# Lier le port 0 au vswitch 1 avec natif/non-étiqueté
set vswitch-link vswitch 1 port 0 tag 0
```

### delete vswitch-link
```
delete vswitch-link <0-7>
```
**Description** : Supprime un lien de commutateur virtuel par ID

**Paramètres** :
- `<0-7>` : ID du lien de commutateur virtuel

### show vswitch-link
```
show vswitch-link
```
**Description** : Affiche toutes les configurations de liens de commutateurs virtuels

## Commandes d'Interface Router

### set vswitch router-if
```
set vswitch <1-4094> router-if <WORD>
```
**Description** : Crée une interface router sur le commutateur virtuel spécifié pour connectivité L3

**Paramètres** :
- `<1-4094>` : ID VLAN du commutateur virtuel
- `<WORD>` : Nom de l'interface TAP

**Exemples** :
```
set vswitch 2031 router-if rif2031
set vswitch 2032 router-if rif2032
```

### no set vswitch router-if
```
no set vswitch <1-4094> router-if
```
**Description** : Supprime l'interface router du commutateur virtuel spécifié

**Paramètres** :
- `<1-4094>` : ID VLAN du commutateur virtuel

### show rib vswitch router-if
```
show rib vswitch router-if
```
**Description** : Affiche les configurations d'interfaces router incluant adresses MAC, adresses IP et état des interfaces

## Commandes d'Interface Capture

### set vswitch capture-if
```
set vswitch <1-4094> capture-if <WORD>
```
**Description** : Crée une interface capture sur le commutateur virtuel spécifié pour surveillance de paquets

**Paramètres** :
- `<1-4094>` : ID VLAN du commutateur virtuel
- `<WORD>` : Nom de l'interface TAP

**Exemples** :
```
set vswitch 2031 capture-if cif2031
set vswitch 2032 capture-if cif2032
```

### no set vswitch capture-if
```
no set vswitch <1-4094> capture-if
```
**Description** : Supprime l'interface capture du commutateur virtuel spécifié

**Paramètres** :
- `<1-4094>` : ID VLAN du commutateur virtuel

### show rib vswitch capture-if
```
show rib vswitch capture-if
```
**Description** : Affiche les configurations d'interfaces capture

## Fonctionnalités de Traitement VLAN

Le répéteur amélioré effectue un traitement VLAN sophistiqué :

- **Translation VLAN** : Modifie les ID VLAN basés sur la configuration vswitch-link
- **Suppression VLAN** : Supprime les en-têtes VLAN quand l'étiquette est définie à 0 (natif)
- **Insertion VLAN** : Ajoute des en-têtes VLAN lors de transmission de paquets non-étiquetés vers des ports étiquetés
- **Split Horizon** : Empêche les boucles en ne retransmettant pas les paquets vers le port de réception

## Exemple de Configuration

```bash
# Créer des commutateurs virtuels
set vswitch 2031
set vswitch 2032

# Lier le port DPDK aux commutateurs virtuels avec étiquettes VLAN
set vswitch-link vswitch 0 port 0 tag 2031
set vswitch-link vswitch 1 port 0 tag 2032

# Créer des interfaces router pour traitement L3
set vswitch 2031 router-if rif2031
set vswitch 2032 router-if rif2032

# Créer des interfaces capture pour surveillance
set vswitch 2031 capture-if cif2031
set vswitch 2032 capture-if cif2032

# Configurer le worker pour utiliser enhanced-repeater
set worker lcore 1 enhanced-repeater
```

## Intégration avec les Interfaces TAP

Les interfaces router et capture créent des interfaces TAP qui s'intègrent avec la pile réseau du noyau Linux :

- **Interfaces Router** : Permettent le routage L3, l'adressage IP et le traitement réseau noyau
- **Interfaces Capture** : Activent la surveillance, l'analyse et le débogage de paquets
- **Tampons Anneau** : Utilisent des anneaux DPDK pour transfert efficace de paquets entre plan de données et noyau