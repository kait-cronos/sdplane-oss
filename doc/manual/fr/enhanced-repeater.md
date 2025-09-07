# Configuration du Répéteur Amélioré

**Language:** [English](../en/enhanced-repeater.md) | [日本語](../ja/enhanced-repeater.md) | **Français** | [中文](../zh/enhanced-repeater.md) | [Deutsch](../de/enhanced-repeater.md) | [Italiano](../it/enhanced-repeater.md) | [한국어](../ko/enhanced-repeater.md) | [ไทย](../th/enhanced-repeater.md) | [Español](../es/enhanced-repeater.md)

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

## Configuration RIB (Base d'Informations de Routage)

### Gestion des Interfaces Router

Une fois les interfaces router créées, elles peuvent être configurées avec des adresses IP via les commandes système Linux standard :

```bash
# Configuration IP pour les interfaces router
ip addr add 192.168.31.1/24 dev rif2031
ip addr add 192.168.32.1/24 dev rif2032

# Activation des interfaces
ip link set rif2031 up
ip link set rif2032 up
```

### Surveillance du RIB

```bash
# Afficher l'état détaillé des interfaces router
show rib vswitch router-if

# Afficher les informations du RIB des commutateurs virtuels
show vswitch_rib
```

La sortie inclut :
- Adresses MAC des interfaces
- Configuration IP active
- État des interfaces (up/down)
- Statistiques de trafic
- Informations de liaison VLAN

## Configuration VLAN Avancée

### Scénarios de Configuration VLAN

#### Scénario 1 : VLAN Tronqué (Trunk)
Configuration pour porter plusieurs VLANs sur un seul port physique :

```bash
# Créer plusieurs commutateurs virtuels
set vswitch 100
set vswitch 200
set vswitch 300

# Lier le même port physique avec différentes étiquettes VLAN
set vswitch-link vswitch 0 port 0 tag 100
set vswitch-link vswitch 1 port 0 tag 200
set vswitch-link vswitch 2 port 0 tag 300

# Créer des interfaces router pour chaque VLAN
set vswitch 100 router-if rif100
set vswitch 200 router-if rif200
set vswitch 300 router-if rif300
```

#### Scénario 2 : VLAN Natif (Access)
Configuration pour trafic non-étiqueté :

```bash
# Créer commutateur virtuel pour trafic natif
set vswitch 1

# Lier avec tag 0 pour trafic non-étiqueté
set vswitch-link vswitch 0 port 1 tag 0

# Interface router pour VLAN natif
set vswitch 1 router-if rif-native
```

#### Scénario 3 : Isolation VLAN
Configuration pour séparer complètement les domaines de diffusion :

```bash
# Créer VLANs isolés
set vswitch 10  # VLAN Management
set vswitch 20  # VLAN Utilisateurs
set vswitch 30  # VLAN Serveurs

# Ports séparés pour chaque VLAN
set vswitch-link vswitch 0 port 0 tag 10  # Port management
set vswitch-link vswitch 1 port 1 tag 20  # Port utilisateurs  
set vswitch-link vswitch 2 port 2 tag 30  # Port serveurs

# Interfaces router distinctes
set vswitch 10 router-if rif-mgmt
set vswitch 20 router-if rif-users
set vswitch 30 router-if rif-servers
```

### Configuration Multi-Ports

```bash
# Configuration avec multiples ports physiques
set vswitch 500

# Lier plusieurs ports au même commutateur virtuel
set vswitch-link vswitch 0 port 0 tag 500
set vswitch-link vswitch 0 port 1 tag 500
set vswitch-link vswitch 0 port 2 tag 500

# Interface router commune
set vswitch 500 router-if rif500

# Interfaces capture pour surveillance
set vswitch 500 capture-if cap500
```

## Diagnostics et Dépannage

### Commandes de Diagnostic

#### Vérification de l'État des Commutateurs Virtuels

```bash
# Afficher tous les commutateurs virtuels
show vswitch_rib

# Vérifier les liens de commutateurs virtuels
show vswitch-link

# État des interfaces router
show rib vswitch router-if

# État des interfaces capture
show rib vswitch capture-if
```

#### Surveillance du Trafic

```bash
# Utiliser tcpdump sur les interfaces TAP
tcpdump -i rif2031 -n
tcpdump -i cif2031 -n

# Surveillance avec filtres
tcpdump -i rif2031 'vlan 2031'
tcpdump -i cif2031 'icmp'
```

#### Vérification de la Connectivité

```bash
# Tester la connectivité L2
ping -I rif2031 192.168.31.2

# Traçage des paquets
tcpdump -i cif2031 'host 192.168.31.2' &
ping -I rif2031 192.168.31.2
```

### Problèmes Courants et Solutions

#### Problème : Interface TAP Non Créée

**Symptômes** :
- Erreur "interface not found"
- Commandes `ip` échouent

**Diagnostic** :
```bash
# Vérifier l'existence de l'interface
ip link show | grep rif

# Vérifier la configuration du commutateur virtuel
show vswitch_rib
```

**Solution** :
```bash
# Recréer l'interface router
no set vswitch 2031 router-if
set vswitch 2031 router-if rif2031
ip link set rif2031 up
```

#### Problème : Pas de Trafic VLAN

**Symptômes** :
- Paquets étiquetés ne sont pas transmis
- Isolation VLAN ne fonctionne pas

**Diagnostic** :
```bash
# Vérifier la configuration des liens
show vswitch-link

# Capturer le trafic sur l'interface physique
tcpdump -i <interface-physique> 'vlan'
```

**Solution** :
```bash
# Vérifier et corriger les étiquettes VLAN
delete vswitch-link <id-incorrect>
set vswitch-link vswitch <id> port <port> tag <vlan-correct>
```

#### Problème : Performance Dégradée

**Symptômes** :
- Latence élevée
- Perte de paquets
- Débit faible

**Diagnostic** :
```bash
# Vérifier les statistiques des interfaces
cat /proc/net/dev | grep rif
cat /proc/net/dev | grep cif

# Surveiller l'utilisation CPU des workers
top -p $(pgrep sdplane)
```

**Solution** :
```bash
# Optimiser la configuration des workers
set worker lcore 1 enhanced-repeater
set worker lcore 2 enhanced-repeater  # Ajouter plus de workers

# Ajuster les paramètres DPDK
# Augmenter la taille des tampons anneau si nécessaire
```

## Optimisation des Performances

### Configuration des Workers

```bash
# Utiliser plusieurs cœurs pour le traitement
set worker lcore 1 enhanced-repeater
set worker lcore 2 enhanced-repeater
set worker lcore 3 enhanced-repeater

# Vérifier la distribution des workers
show worker
```

### Optimisation Réseau

#### Paramètres Interface TAP

```bash
# Augmenter la taille de la file d'attente TX
ip link set rif2031 txqueuelen 2000

# Optimiser les paramètres de l'interface
ethtool -K rif2031 tso off gso off gro off lro off
```

#### Paramètres Système

```bash
# Augmenter les tampons réseau du noyau
echo 'net.core.rmem_max = 134217728' >> /etc/sysctl.conf
echo 'net.core.wmem_max = 134217728' >> /etc/sysctl.conf
echo 'net.core.netdev_max_backlog = 5000' >> /etc/sysctl.conf

# Appliquer les changements
sysctl -p
```

### Surveillance des Performances

```bash
# Surveiller les statistiques en temps réel
watch -n 1 'cat /proc/net/dev | grep -E "(rif|cif)"'

# Surveillance des interruptions
watch -n 1 'cat /proc/interrupts | grep -E "(eth|dpdk)"'

# Utilisation mémoire DPDK
cat /proc/meminfo | grep -E "(Hugepages|MemAvailable)"
```

## Configuration de Production

### Exemple de Configuration Complète

```bash
#!/bin/bash
# Script de configuration pour environnement de production

# Créer les commutateurs virtuels principaux
set vswitch 100   # VLAN Management
set vswitch 200   # VLAN DMZ
set vswitch 300   # VLAN Interne
set vswitch 400   # VLAN Invités

# Configuration des liens pour port tronqué
set vswitch-link vswitch 0 port 0 tag 100
set vswitch-link vswitch 1 port 0 tag 200
set vswitch-link vswitch 2 port 0 tag 300
set vswitch-link vswitch 3 port 0 tag 400

# Configuration des liens pour ports d'accès
set vswitch-link vswitch 0 port 1 tag 0    # Port natif management
set vswitch-link vswitch 1 port 2 tag 0    # Port natif DMZ

# Créer les interfaces router
set vswitch 100 router-if rif-mgmt
set vswitch 200 router-if rif-dmz
set vswitch 300 router-if rif-internal
set vswitch 400 router-if rif-guest

# Créer les interfaces capture pour surveillance
set vswitch 100 capture-if cap-mgmt
set vswitch 200 capture-if cap-dmz
set vswitch 300 capture-if cap-internal
set vswitch 400 capture-if cap-guest

# Configurer les workers
set worker lcore 1 enhanced-repeater
set worker lcore 2 enhanced-repeater
set worker lcore 3 enhanced-repeater
```

### Configuration IP Post-Déploiement

```bash
#!/bin/bash
# Configuration IP pour interfaces router

# VLAN Management (100)
ip addr add 10.0.100.1/24 dev rif-mgmt
ip link set rif-mgmt up

# VLAN DMZ (200)  
ip addr add 10.0.200.1/24 dev rif-dmz
ip link set rif-dmz up

# VLAN Interne (300)
ip addr add 10.0.300.1/24 dev rif-internal
ip link set rif-internal up

# VLAN Invités (400)
ip addr add 10.0.400.1/24 dev rif-guest
ip link set rif-guest up

# Configuration du routage inter-VLAN
echo 1 > /proc/sys/net/ipv4/ip_forward

# Règles de pare-feu (exemple avec iptables)
iptables -A FORWARD -i rif-mgmt -o rif-internal -j ACCEPT
iptables -A FORWARD -i rif-internal -o rif-mgmt -j ACCEPT
iptables -A FORWARD -i rif-dmz -o rif-internal -m state --state ESTABLISHED,RELATED -j ACCEPT
```

### Script de Surveillance

```bash
#!/bin/bash
# Script de surveillance pour répéteur amélioré

echo "=== État des Commutateurs Virtuels ==="
show vswitch_rib

echo "=== Configuration des Liens ==="
show vswitch-link

echo "=== Interfaces Router ==="
show rib vswitch router-if

echo "=== Interfaces Capture ==="
show rib vswitch capture-if

echo "=== Statistiques Interfaces TAP ==="
cat /proc/net/dev | grep -E "(rif|cap)" | while read line; do
    echo "$line"
done

echo "=== État des Workers ==="
show worker
```

## Intégration avec Outils Externes

### Surveillance avec SNMP

```bash
# Configuration SNMP pour interfaces TAP
# /etc/snmp/snmpd.conf
view systemview included .1.3.6.1.2.1.2.2.1.10
view systemview included .1.3.6.1.2.1.2.2.1.16

# Redémarrer le service SNMP
systemctl restart snmpd

# Test des requêtes SNMP
snmpwalk -v2c -c public localhost 1.3.6.1.2.1.2.2.1.2 | grep rif
```

### Intégration avec Prometheus

```bash
# Collecteur de métriques personnalisé
#!/bin/bash
# /usr/local/bin/sdplane-metrics.sh

echo "# HELP sdplane_interface_rx_packets Interface RX packets"
echo "# TYPE sdplane_interface_rx_packets counter"

cat /proc/net/dev | grep -E "(rif|cap)" | while read line; do
    iface=$(echo $line | cut -d: -f1 | tr -d ' ')
    rx_packets=$(echo $line | awk '{print $2}')
    echo "sdplane_interface_rx_packets{interface=\"$iface\"} $rx_packets"
done

echo "# HELP sdplane_interface_tx_packets Interface TX packets"  
echo "# TYPE sdplane_interface_tx_packets counter"

cat /proc/net/dev | grep -E "(rif|cap)" | while read line; do
    iface=$(echo $line | cut -d: -f1 | tr -d ' ')
    tx_packets=$(echo $line | awk '{print $10}')
    echo "sdplane_interface_tx_packets{interface=\"$iface\"} $tx_packets"
done
```

### Automatisation avec Ansible

```yaml
# playbook-sdplane.yml
---
- name: Configuration SDPlane Enhanced Repeater
  hosts: sdplane-nodes
  become: yes
  
  vars:
    vlans:
      - { id: 100, name: "mgmt", ip: "10.0.100.1/24" }
      - { id: 200, name: "dmz", ip: "10.0.200.1/24" }
      - { id: 300, name: "internal", ip: "10.0.300.1/24" }
  
  tasks:
    - name: Créer les commutateurs virtuels
      shell: "echo 'set vswitch {{ item.id }}' | nc -w1 localhost 9882"
      loop: "{{ vlans }}"
    
    - name: Configurer les liens vswitch
      shell: "echo 'set vswitch-link vswitch {{ loop.index0 }} port 0 tag {{ item.id }}' | nc -w1 localhost 9882"
      loop: "{{ vlans }}"
      loop_control:
        index_var: loop.index0
    
    - name: Créer les interfaces router
      shell: "echo 'set vswitch {{ item.id }} router-if rif-{{ item.name }}' | nc -w1 localhost 9882"
      loop: "{{ vlans }}"
    
    - name: Configurer les adresses IP
      shell: |
        ip addr add {{ item.ip }} dev rif-{{ item.name }}
        ip link set rif-{{ item.name }} up
      loop: "{{ vlans }}"
```

## Sécurité et Bonnes Pratiques

### Isolation Réseau

```bash
# Utiliser des espaces de noms réseau pour isolation
ip netns add vlan100
ip netns add vlan200

# Déplacer les interfaces vers les espaces de noms
ip link set rif-mgmt netns vlan100
ip link set rif-dmz netns vlan200

# Configuration dans l'espace de noms
ip netns exec vlan100 ip addr add 10.0.100.1/24 dev rif-mgmt
ip netns exec vlan100 ip link set rif-mgmt up
```

### Contrôle d'Accès

```bash
# Limiter l'accès aux interfaces de capture
chmod 600 /dev/net/tun
chown root:root /dev/net/tun

# Utiliser des groupes pour contrôler l'accès
groupadd sdplane-operators
usermod -a -G sdplane-operators monitoring-user

# Règles sudo spécifiques
echo '%sdplane-operators ALL=(root) NOPASSWD: /usr/local/bin/sdplane-stats.sh' >> /etc/sudoers
```

### Audit et Logging

```bash
# Configuration rsyslog pour SDPlane
echo 'local0.*    /var/log/sdplane.log' >> /etc/rsyslog.conf
systemctl restart rsyslog

# Script d'audit des changements
#!/bin/bash
# /usr/local/bin/sdplane-audit.sh

LOG_FILE="/var/log/sdplane-audit.log"
TIMESTAMP=$(date '+%Y-%m-%d %H:%M:%S')

# Enregistrer les changements de configuration
echo "[$TIMESTAMP] Configuration check:" >> $LOG_FILE
show vswitch_rib >> $LOG_FILE 2>&1
show vswitch-link >> $LOG_FILE 2>&1
echo "" >> $LOG_FILE
```

## Références et Documentation Avancée

### Commandes de Référence Rapide

```bash
# Gestion des commutateurs virtuels
set vswitch <1-4094>                          # Créer commutateur virtuel
delete vswitch <0-3>                          # Supprimer commutateur virtuel
show vswitch_rib                              # Afficher informations RIB

# Gestion des liens
set vswitch-link vswitch <0-3> port <0-7> tag <0-4094>  # Créer lien
delete vswitch-link <0-7>                     # Supprimer lien
show vswitch-link                             # Afficher liens

# Interfaces router
set vswitch <1-4094> router-if <WORD>         # Créer interface router
no set vswitch <1-4094> router-if             # Supprimer interface router
show rib vswitch router-if                    # Afficher interfaces router

# Interfaces capture  
set vswitch <1-4094> capture-if <WORD>        # Créer interface capture
no set vswitch <1-4094> capture-if            # Supprimer interface capture
show rib vswitch capture-if                   # Afficher interfaces capture

# Configuration des workers
set worker lcore <1-16> enhanced-repeater     # Assigner worker au repeater
```

### Limites et Considérations

- **Nombre maximum de commutateurs virtuels** : 4 (IDs 0-3)
- **Nombre maximum de ports DPDK** : 8 (IDs 0-7)
- **Plage des ID VLAN** : 1-4094 (conforme à la norme IEEE 802.1Q)
- **Nombre maximum de liens vswitch** : 8 (IDs 0-7)
- **Performance** : Dépend du nombre de cœurs CPU assignés comme workers

### Architecture Interne

```
┌─────────────────────────────────────────────────────────────┐
│                    Enhanced Repeater                        │
├─────────────────────────────────────────────────────────────┤
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐         │
│  │  VSwitch 0  │  │  VSwitch 1  │  │  VSwitch N  │         │
│  │  VLAN: 100  │  │  VLAN: 200  │  │  VLAN: XXX  │         │
│  └─────────────┘  └─────────────┘  └─────────────┘         │
├─────────────────────────────────────────────────────────────┤
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐         │
│  │    Port 0   │  │    Port 1   │  │    Port N   │         │
│  │   DPDK NIC  │  │   DPDK NIC  │  │   DPDK NIC  │         │
│  └─────────────┘  └─────────────┘  └─────────────┘         │
├─────────────────────────────────────────────────────────────┤
│  ┌─────────────┐  ┌─────────────┐                           │
│  │ Router IF   │  │ Capture IF  │                           │
│  │ (TAP/Kernel)│  │ (TAP/Monitor)│                          │
│  └─────────────┘  └─────────────┘                           │
└─────────────────────────────────────────────────────────────┘
```

Cette architecture permet un traitement VLAN haute performance avec intégration complète au système d'exploitation pour le routage L3 et la surveillance réseau.