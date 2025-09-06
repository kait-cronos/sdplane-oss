# Gestion des Périphériques

**Language:** [English](../device-management.md) | [日本語](../ja/device-management.md) | **Français** | [中文](../zh/device-management.md)

Commandes pour la gestion des périphériques PCIe et la configuration des pilotes.

## Vue d'ensemble

La gestion des périphériques dans sdplane concerne principalement la liaison des cartes réseau PCIe aux pilotes DPDK appropriés :

- **Liaison Périphériques** : Attacher NICs aux pilotes DPDK (vfio-pci, igb_uio)
- **Configuration Pilotes** : Configurer pilotes pour performance optimale
- **Gestion PCIe** : Identification et gestion périphériques PCIe
- **Hot-plug** : Support ajout/suppression à chaud (limité)

## Commandes de Périphérique

### set_device_driver_bind - Lier Périphérique
```
set device <pcie-id> driver <pilote> bind
```

Lie un périphérique PCIe à un pilote spécifique.

**Paramètres :**
- `<pcie-id>` : Identifiant PCIe (format : XX:XX.X)
- `<pilote>` : Nom du pilote (vfio-pci, igb_uio, etc.)

**Exemples :**
```bash
set device 03:00.0 driver vfio-pci bind
set device 04:00.1 driver igb_uio bind
```

### set_device_driver_unbind - Délier Périphérique  
```
set device <pcie-id> driver unbind
```

Délie un périphérique de son pilote actuel.

**Exemple :**
```bash
set device 03:00.0 driver unbind
```

### set_device_driver_override - Override Pilote
```
set device <pcie-id> driver <pilote> driver_override
```

Configure override de pilote pour périphérique spécifique.

**Exemple :**
```bash
set device 03:00.0 driver vfio-pci driver_override
```

## Pilotes DPDK

### vfio-pci (Recommandé)
Pilote moderne avec support IOMMU :

**Avantages :**
- Sécurité renforcée avec IOMMU
- Isolation mémoire appropriée
- Support hotplug amélioré
- Compatibility large gamme NICs

**Configuration :**
```bash
# Activer IOMMU dans GRUB
# /etc/default/grub
GRUB_CMDLINE_LINUX="iommu=pt intel_iommu=on"

# Charger module vfio-pci
sudo modprobe vfio-pci

# Lier périphérique
set device 03:00.0 driver vfio-pci bind
```

### igb_uio (Alternatif)
Pilote DPDK traditionnel :

**Utilisation :**
- NICs non compatibles vfio-pci
- Environnements sans IOMMU
- Applications legacy

**Installation :**
```bash
# Compiler et installer igb_uio
git clone http://dpdk.org/git/dpdk-kmods
cd dpdk-kmods/linux/igb_uio
make
sudo make install

# Charger module
sudo modprobe igb_uio

# Utiliser dans sdplane
set device 03:00.0 driver igb_uio bind
```

## Identification Périphériques

### lspci - Lister Périphériques PCIe
```bash
# Lister tous périphériques réseau
lspci | grep -i ethernet

# Information détaillée
lspci -vv -s 03:00.0

# Avec IDs numériques
lspci -nn | grep -i ethernet
```

### dpdk-devbind.py - Outil DPDK
```bash
# Afficher état périphériques réseau
dpdk-devbind.py -s

# Afficher périphériques par catégorie
dpdk-devbind.py --status-dev net

# Informations détaillées
dpdk-devbind.py --status
```

### Exemple Sortie dpdk-devbind.py
```
Network devices using kernel driver
===================================
0000:03:00.0 'NetXtreme BCM5720 Gigabit Ethernet PCIe 165f' numa_node=0 if=eno1 drv=tg3 unused=vfio-pci 
0000:04:00.0 'MT27800 Family [ConnectX-5] 1017' numa_node=1 if=enp4s0 drv=mlx5_core unused=vfio-pci
```

## Configuration Périphériques

### Séquence Configuration Standard
```bash
# 1. Identifier périphériques disponibles
lspci | grep -i ethernet

# 2. Vérifier état actuel
dpdk-devbind.py -s

# 3. Délier du pilote kernel (si nécessaire)
set device 03:00.0 driver unbind

# 4. Configurer override pilote
set device 03:00.0 driver vfio-pci driver_override

# 5. Lier au pilote DPDK
set device 03:00.0 driver vfio-pci bind
```

### Configuration Multi-Port
```bash
# Configuration plusieurs NICs
set device 03:00.0 driver vfio-pci bind
set device 03:00.1 driver vfio-pci bind
set device 04:00.0 driver vfio-pci bind
set device 04:00.1 driver vfio-pci bind
```

## Cas Spéciaux

### NICs Mellanox ConnectX
Mellanox nécessite configuration pilote spéciale :

```bash
# Installation pilote Mellanox
wget https://network.nvidia.com/products/ethernet-drivers/linux/mlnx_en/
./install --dpdk

# Configuration sdplane (PAS de liaison manuelle)
# Commenter dans configuration :
# set device XX:XX.X driver unbind
# set device XX:XX.X driver mlx5_core driver_override  
# set device XX:XX.X driver mlx5_core bind

# Mise à jour état ports après configuration
update port status
```

### NICs Intel
Configuration standard vfio-pci :

```bash
# NICs Intel avec vfio-pci
set device 03:00.0 driver vfio-pci bind
set device 03:00.1 driver vfio-pci bind
```

### NICs Virtualisées
Dans environnements virtuels :

```bash
# Vérifier support IOMMU dans VM
dmesg | grep -i iommu

# Utiliser igb_uio si vfio-pci indisponible
set device 03:00.0 driver igb_uio bind
```

## Dépannage

### Problèmes Liaison Communs
1. **Pilote inexistant** : Vérifier modules chargés
2. **IOMMU désactivé** : Activer dans BIOS/GRUB
3. **Permissions insuffisantes** : Exécuter avec privilèges appropriés
4. **Périphérique en utilisation** : Délier pilote actuel d'abord

### Diagnostic
```bash
# Vérifier modules pilotes chargés
lsmod | grep -E "(vfio_pci|igb_uio)"

# Vérifier support IOMMU
dmesg | grep -i iommu

# État périphériques détaillé
dpdk-devbind.py --status

# Logs noyau pour erreurs liaison
dmesg | tail -20
```

### Résolution Problèmes
```bash
# Réinitialiser liaison périphérique
set device 03:00.0 driver unbind
echo "03:00.0" > /sys/bus/pci/drivers_probe

# Forcer rechargement pilote
sudo rmmod vfio_pci
sudo modprobe vfio_pci

# Vérifier permissions périphériques
ls -la /dev/vfio/
```

## Sécurité et Permissions

### IOMMU et Isolation
vfio-pci fournit isolation mémoire :
- **Protection DMA** : Périphériques isolés via IOMMU
- **Sécurité Renforcée** : Prévention accès mémoire non autorisés
- **Containers** : Support isolation containers/VMs

### Permissions Requises
```bash
# sdplane nécessite privilèges root
sudo ./sdplane/sdplane

# Ou configuration capabilities spécifiques
sudo setcap cap_sys_admin+ep ./sdplane/sdplane
```

### Groupes VFIO
Gestion groupes IOMMU :
```bash
# Lister groupes VFIO
ls /sys/kernel/iommu_groups/

# Information groupe périphérique
ls /sys/kernel/iommu_groups/*/devices/ | grep 03:00.0
```

## Automation et Scripts

### Script Configuration Périphériques
```bash
#!/bin/bash
# Configuration automatique périphériques sdplane

DEVICES=("03:00.0" "03:00.1" "04:00.0" "04:00.1")
DRIVER="vfio-pci"

for device in "${DEVICES[@]}"; do
    echo "Configuration $device avec $DRIVER"
    
    # Délier pilote actuel
    set device $device driver unbind
    
    # Override pilote
    set device $device driver $DRIVER driver_override
    
    # Lier nouveau pilote
    set device $device driver $DRIVER bind
done
```

### Vérification Post-Configuration
```bash
#!/bin/bash
# Vérifier configuration périphériques

echo "État périphériques réseau :"
dpdk-devbind.py -s

echo "Périphériques liés à vfio-pci :"
lspci -k | grep -A 2 -i ethernet | grep vfio-pci
```

## Performance et Optimisation

### Configuration NUMA
Optimiser périphériques selon topologie NUMA :

```bash
# Identifier nœud NUMA périphérique
cat /sys/class/pci_bus/0000:03/device/numa_node

# Affecter workers aux cœurs appropriés
set worker lcore 1 enhanced-repeater  # lcore sur même nœud NUMA
```

### Paramètres Pilote
Certains pilotes supportent paramètres ajustables :

```bash
# Paramètres vfio-pci
echo Y > /sys/module/vfio/parameters/enable_unsafe_noiommu_mode

# Paramètres igb_uio (si utilisé)
echo 2048 > /sys/module/igb_uio/parameters/wc_activate
```

## Emplacement de Définition

Gestion périphériques définie dans :
- `sdplane/device_cmd.c` - Commandes gestion périphériques
- Scripts système - Automation liaison périphériques

## Sujets Associés

- [Gestion Ports](port-management.md) - Configuration ports après liaison périphériques
- [Installation](../../README.fr.md) - Configuration système pour périphériques DPDK
- [Surveillance Système](system-monitoring.md) - Surveillance état périphériques
- [Debug & Logging](debug-logging.md) - Debug problèmes périphériques