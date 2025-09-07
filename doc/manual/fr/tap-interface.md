# Interface TAP

**Language:** [English](../en/tap-interface.md) | [日本語](../ja/tap-interface.md) | **Français** | [中文](../zh/tap-interface.md) | [Deutsch](../de/tap-interface.md) | [Italiano](../it/tap-interface.md) | [한국어](../ko/tap-interface.md) | [ไทย](../th/tap-interface.md) | [Español](../es/tap-interface.md)

Gestion des interfaces TAP pour intégration avec la pile réseau du noyau Linux.

## Vue d'ensemble

Les interfaces TAP permettent l'intégration entre le plan de données DPDK haute performance de sdplane et la pile réseau du noyau Linux :

- **Pont Kernel-Userspace** : Communication bidirectionnelle entre DPDK et noyau
- **Interfaces Virtuelles** : Interfaces réseau virtuelles visibles dans Linux
- **Intégration L3** : Support routage IP et applications réseau standard
- **Capture Paquets** : Surveillance et analyse trafic

## Types d'Interfaces TAP

### Interface Router (router-if)
Interfaces pour traitement Layer 3 et intégration noyau :
- **Routage IP** : Participation au routage système
- **Applications Réseau** : Accès via socket standard
- **Configuration Réseau** : Adressage IP et routes
- **Services Système** : SSH, HTTP, etc.

### Interface Capture (capture-if)  
Interfaces pour surveillance et analyse :
- **Capture Passive** : Copie paquets pour analyse
- **Surveillance Trafic** : Outils monitoring réseau
- **Debug Réseau** : Analyse problèmes connectivité
- **Logging** : Enregistrement activité réseau

## Configuration Interface TAP

### Création Interface Router
```bash
# Créer interface router sur commutateur virtuel
set vswitch <vlan-id> router-if <nom-interface>

# Exemples
set vswitch 2031 router-if rif2031
set vswitch 2032 router-if rif2032
```

### Création Interface Capture
```bash
# Créer interface capture sur commutateur virtuel
set vswitch <vlan-id> capture-if <nom-interface>

# Exemples  
set vswitch 2031 capture-if cif2031
set vswitch 2032 capture-if cif2032
```

### Suppression Interfaces
```bash
# Supprimer interface router
no set vswitch <vlan-id> router-if

# Supprimer interface capture
no set vswitch <vlan-id> capture-if
```

## Configuration Système

### Configuration Interface Router
Une fois interface TAP créée, configurer dans Linux :

```bash
# Activer interface
sudo ip link set rif2031 up

# Configurer adresse IP
sudo ip addr add 192.168.31.1/24 dev rif2031

# Ajouter routes si nécessaire
sudo ip route add 192.168.31.0/24 dev rif2031
```

### Configuration Interface Capture
```bash
# Activer interface capture
sudo ip link set cif2031 up

# Configurer pour capture (pas d'IP nécessaire)
sudo tcpdump -i cif2031 -w capture.pcap
```

## Architecture TAP

### Flux de Données Router
```
[DPDK Port] ←→ [Enhanced Repeater] ←→ [TAP Ring] ←→ [Router Interface] ←→ [Linux Kernel]
```

1. **Réception** : Paquets DPDK vers interface TAP
2. **Traitement Kernel** : Pile réseau Linux traite paquets  
3. **Transmission** : Réponses kernel vers DPDK
4. **Bidirectionnel** : Communication dans les deux sens

### Flux de Données Capture
```
[DPDK Port] → [Enhanced Repeater] → [Copy] → [TAP Ring] → [Capture Interface]
```

1. **Copie Paquets** : Duplication sélective paquets
2. **Interface Passive** : Pas de transmission retour
3. **Surveillance** : Outils analyse accèdent paquets

## Gestion Workers TAP

### TAP Handler Worker
Worker dédié pour gestion interfaces TAP :

```bash
# Configurer worker TAP handler
set worker lcore 2 tap-handler

# Pour traitement L3 spécialisé
set worker lcore 3 l3-tap-handler
```

### Fonctions Worker
- **Transfer Bidirectionnel** : Paquets DPDK ←→ TAP
- **Gestion Buffers** : Allocation/libération tampons
- **Statistiques** : Compteurs trafic TAP
- **Gestion Erreurs** : Récupération situations erreur

## Surveillance Interface TAP

### Commandes Statut
```bash
# Afficher configuration interfaces router
show rib vswitch router-if

# Afficher configuration interfaces capture  
show rib vswitch capture-if

# Statistiques système générales
show rib
```

### Statistiques Linux
```bash
# Statistiques interface TAP dans Linux
ip -s link show rif2031

# Trafic interface
cat /proc/net/dev | grep rif2031

# Routes utilisant interface
ip route show dev rif2031
```

## Cas d'Utilisation

### Routage Inter-VLAN
Configuration router-on-a-stick :

```bash
# Créer interfaces router pour VLANs multiples
set vswitch 2031 router-if rif2031
set vswitch 2032 router-if rif2032

# Configuration Linux
sudo ip addr add 192.168.31.1/24 dev rif2031
sudo ip addr add 192.168.32.1/24 dev rif2032

# Activer forwarding IP
echo 1 | sudo tee /proc/sys/net/ipv4/ip_forward
```

### Surveillance Réseau
```bash
# Interface capture pour surveillance
set vswitch 2031 capture-if cif2031

# Capture avec tcpdump
sudo tcpdump -i cif2031 -w network-capture.pcap

# Analyse avec Wireshark
wireshark -i cif2031
```

### Services Réseau
```bash
# Interface pour services SSH/HTTP
set vswitch 2031 router-if mgmt

# Configuration management
sudo ip addr add 192.168.100.1/24 dev mgmt

# Services écoutent sur interface TAP
sudo systemctl start sshd
```

## Performance TAP

### Optimisations
- **Taille Ring Buffer** : Ajuster tailles anneaux pour débit
- **Worker Dédiés** : lcores séparés pour TAP handler
- **Copying Sélectif** : Copier seulement trafic nécessaire
- **Batch Processing** : Traitement paquets par lots

### Considérations Performance
- **Surcharge Copie** : Impact copie paquets DPDK→TAP
- **Context Switch** : Coût passage userspace→kernel  
- **Bande Passante** : Limitation débit interface TAP
- **Latence** : Délai additionnel traitement kernel

## Dépannage

### Problèmes Courants
1. **Interface TAP non créée** : Vérifier permissions et support kernel
2. **Pas de connectivité** : Vérifier configuration IP et routes
3. **Performance dégradée** : Optimiser workers et buffers
4. **Perte paquets** : Ajuster tailles buffers et ring

### Diagnostic
```bash
# Vérifier création interfaces
ip link show | grep -E "(rif|cif)"

# Vérifier configuration sdplane
show rib vswitch router-if
show rib vswitch capture-if

# Logs système TAP
dmesg | grep tun
journalctl -u sdplane | grep -i tap
```

### Debug TAP
```bash
# Activer debug TAP
debug sdplane tap

# Surveiller trafic
tcpdump -i rif2031 -v

# Statistiques détaillées
cat /sys/class/net/rif2031/statistics/*
```

## Sécurité

### Permissions
Interface TAP nécessite privilèges appropriés :
```bash
# sdplane doit tourner avec privilèges
sudo ./sdplane/sdplane

# Ou configuration capabilities
sudo setcap cap_net_admin+ep ./sdplane/sdplane
```

### Isolation
- Interfaces TAP héritent sécurité réseau système
- Firewall rules s'appliquent aux interfaces TAP
- Isolation réseau via namespaces si nécessaire

### Bonnes Pratiques
- Limiter accès interfaces TAP aux services nécessaires
- Surveiller trafic pour activité suspecte  
- Configurer firewall approprié pour interfaces
- Rotation logs capture pour gestion espace

## Intégration Applications

### Applications Réseau Standard
```bash
# Services écoutent sur interfaces TAP
netstat -tuln | grep 192.168.31.1

# Applications peuvent utiliser interfaces normalement
ping -I rif2031 192.168.31.100
wget --bind-address=192.168.31.1 http://example.com
```

### Monitoring Tools
```bash
# Surveillance avec SNMP
snmpwalk -v2c -c public 192.168.31.1

# Monitoring bande passante
iftop -i rif2031
```

## Emplacement de Définition

Interfaces TAP définies dans :
- `sdplane/tap_handler.c` - Gestion interfaces TAP
- `sdplane/l3_tap_handler.c` - Traitement L3 spécialisé  
- `sdplane/rib.c` - Configuration interfaces dans RIB

## Sujets Associés

- [Répéteur Amélioré](enhanced-repeater.md) - Configuration commutateurs virtuels
- [Gestion Workers](worker-lcore-thread-management.md) - Configuration TAP handlers
- [RIB & Routage](routing.md) - Interfaces TAP dans RIB
- [Debug & Logging](debug-logging.md) - Debug interfaces TAP