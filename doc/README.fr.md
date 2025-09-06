<div align="center">
<img src="../sdplane-logo.png" alt="sdplane-oss Logo" width="160">
</div>

# sdplane-oss (Plan de Données Logiciel)

Un "Environnement de Développement DPDK-dock" constitué d'un shell interactif pouvant contrôler les opérations de threads DPDK et d'un environnement d'exécution de threads DPDK (sd-plane)

**Language:** [English](../README.md) | [Japanese](README.ja.md) | **Français**

## Caractéristiques

- **Traitement de Paquets Haute Performance** :
  Traitement de paquets zero-copy en espace utilisateur utilisant DPDK
- **Transmission Layer 2/3** :
  Transmission L2 et L3 avec support intégré ACL, LPM et FIB
- **Génération de Paquets** :
  Générateur de paquets intégré pour tests et benchmarks
- **Virtualisation Réseau** :
  Support d'interfaces TAP et fonctionnalités de commutation VLAN
- **Gestion CLI** :
  Interface ligne de commande interactive pour configuration et surveillance
- **Multi-threading** :
  Modèle de threading coopératif avec des workers par cœur

### Architecture
- **Application Principale** : Logique de routeur central et initialisation
- **Modules DPDK** : Transmission L2/L3 et génération de paquets
- **Système CLI** : Interface ligne de commande avec complétion et aide
- **Threading** : Multi-tâches coopératif basé sur lthread
- **Virtualisation** : Interfaces TAP et commutation virtuelle

## Systèmes Supportés

### Prérequis Logiciels
- **OS** :
  Ubuntu 24.04 LTS (actuellement supporté)
- **NIC** :
  [Pilotes](https://doc.dpdk.org/guides/nics/) | [NICs Supportées](https://core.dpdk.org/supported/)
- **Mémoire** :
  Support hugepage requis
- **CPU** :
  Processeur multi-cœur recommandé

### Plateformes Matérielles Cibles

Ce projet a été validé sur :
- **Topton (N305/N100)** : Mini-PC avec NICs 10G
- **Partaker (J3160)** : Mini-PC avec NICs 1G
- **PC Intel générique** : avec Intel x520 / Mellanox ConnectX5
- **Autres CPUs** : Devrait fonctionner avec AMD, ARM CPU, etc.

## 1. Installation des Dépendances

### Dépendances
- **liburcu-qsbr** : Bibliothèque RCU espace utilisateur
- **libpcap** : Bibliothèque capture de paquets
- **lthread** : [yasuhironet/lthread](https://github.com/yasuhironet/lthread) (threading coopératif léger)
- **DPDK** : Data Plane Development Kit

### Installation des paquets debian de dépendances sdplane
```bash
sudo apt install liburcu-dev libpcap-dev
```

### Installation des outils de build et paquets prérequis DPDK

```bash
# Outils de build essentiels
sudo apt install build-essential cmake autotools-dev autoconf automake libtool pkg-config

# Paquets prérequis DPDK
sudo apt install python3 python3-pip meson ninja-build python3-pyelftools libnuma-dev pkgconf
```

### Installation de lthread
```bash
# Installation de lthread
git clone https://github.com/yasuhironet/lthread
cd lthread
cmake .
make
sudo make install
```
### Installation de DPDK 23.11.1
```bash
# Téléchargement et extraction de DPDK
wget https://fast.dpdk.org/rel/dpdk-23.11.1.tar.xz
tar vxJf dpdk-23.11.1.tar.xz
cd dpdk-stable-23.11.1

# Build et installation de DPDK
meson setup build
cd build
ninja
sudo meson install
sudo ldconfig

# Vérification de l'installation
pkg-config --modversion libdpdk
```

### Installation optionnelle du module kernel IGB DPDK

Si votre NIC ne fonctionne pas avec vfio-pci, installez optionnellement igb_uio.

```bash
git clone http://dpdk.org/git/dpdk-kmods
cd dpdk-kmods/linux/igb_uio
make
sudo make install
```

### 2. Compilation et Installation de sdplane

```bash
# Génération des fichiers de build
./autogen.sh

# Configuration et compilation
mkdir build
cd build
CFLAGS="-g -O0" sh ../configure
make
```

### 3. Configuration

#### 3.1. Configuration des hugepages

```bash
# Allocation temporaire de hugepages
echo 1024 | sudo tee /sys/kernel/mm/hugepages/hugepages-2048kB/nr_hugepages

# Configuration permanente dans /etc/default/grub
GRUB_CMDLINE_LINUX="hugepages=1024"
```

#### 3.2. Liaison des NICs

```bash
# Vérification des NICs disponibles
lspci | grep -i ethernet

# Liaison à vfio-pci (exemple avec 03:00.0)
sudo modprobe vfio-pci
echo 03:00.0 | sudo tee /sys/bus/pci/devices/0000:03:00.0/driver/unbind
echo "8086 1563" | sudo tee /sys/bus/pci/drivers/vfio-pci/new_id
echo 03:00.0 | sudo tee /sys/bus/pci/drivers/vfio-pci/bind
```

### 4. Exécution

```bash
# Démarrage de sdplane
sudo ./sdplane/sdplane

# Connexion au CLI
telnet localhost 9882
```

## Conseils

### IOMMU est requis lors de l'utilisation de vfio-pci comme pilote NIC

- Intel: Intel VT-d
- AMD: AMD IOMMU / AMD-V

Ces options doivent être activées dans les paramètres BIOS.
La configuration GRUB peut aussi nécessiter une modification :

```conf
# /etc/default/grub
GRUB_CMDLINE_LINUX="iommu=pt intel_iommu=on"
```

```bash
sudo update-grub
sudo reboot
```

### Configuration pour charger définitivement le module kernel Linux vfio-pci

```conf
#/etc/modules-load.d/vfio-pci.conf
vfio-pci
```

### Pour les séries Mellanox ConnectX

L'installation du pilote est requise depuis le lien suivant :

https://network.nvidia.com/products/ethernet-drivers/linux/mlnx_en/

Lors de l'installation, exécutez `./install --dpdk`.
**L'option `--dpdk` est obligatoire.**

Commentez les paramètres suivants dans sdplane.conf car ils ne sont pas nécessaires :

```conf
#set device {pcie-id} driver unbound
#set device {pcie-id} driver {nom-pilote} driver_override
#set device {pcie-id} driver {nom-pilote} bind
```

Pour les NICs Mellanox, vous devez exécuter la commande `update port status` pour actualiser les informations de port.

### Comment vérifier les numéros de bus PCIe

Vous pouvez utiliser la commande dpdk-devbind.py dans DPDK pour vérifier les numéros de bus PCIe des NICs :

```bash
> dpdk-devbind.py -s     

Network devices using kernel driver
===================================
0000:04:00.0 'NetXtreme BCM5720 Gigabit Ethernet PCIe 165f' numa_node=0 if=eno8303 drv=tg3 unused= *Active*
0000:b1:00.0 'MT27800 Family [ConnectX-5] 1017' numa_node=1 if=enp177s0np0 drv=mlx5_core unused= *Active*
```

### Ordre des threads workers dans le fichier de configuration

Si vous configurez les workers `rib-manager`, `neigh-manager` et `netlink-thread`, ils doivent être configurés dans cet ordre s'ils sont utilisés.

### Initialisation DPDK

Une seule commande appelant `rte_eal_init()` doit être invoquée depuis le fichier de configuration. La fonction `rte_eal_init()` est appelée par des commandes telles que `rte_eal_init`, `pktgen init`, `l2fwd init` et `l3fwd init`.

## Guide Utilisateur (Manuel)

Des guides utilisateur complets et références de commandes sont disponibles :

- [Guide Utilisateur](manual/fr/README.md) - Aperçu complet et classification des commandes

**Guides d'Application :**
- [Application Répéteur L2](manual/fr/l2-repeater-application.md) - Transmission simple de paquets Layer 2 avec apprentissage MAC
- [Application Répéteur Amélioré](manual/fr/enhanced-repeater-application.md) - Commutation VLAN avec interfaces TAP
- [Application Générateur de Paquets](manual/fr/packet-generator-application.md) - Génération de trafic haute performance et tests

**Guides de Configuration :**
- [Gestion des Ports & Statistiques](manual/fr/port-management.md) - Gestion des ports DPDK et statistiques
- [Gestion des Workers & lcores](manual/fr/worker-management.md) - Gestion des threads workers et lcores
- [Débogage & Journalisation](manual/fr/debug-logging.md) - Fonctions de débogage et journalisation
- [Gestion VTY & Shell](manual/fr/vty-shell.md) - Gestion VTY et shell
- [Information Système & Surveillance](manual/fr/system-monitoring.md) - Information système et surveillance
- [RIB & Routage](manual/fr/routing.md) - Fonctions RIB et routage
- [Configuration des Files](manual/fr/queue-configuration.md) - Configuration et gestion des files
- [Génération de Paquets](manual/fr/packet-generation.md) - Génération de paquets utilisant PKTGEN
- [Information des Threads](manual/fr/thread-information.md) - Information et surveillance des threads
- [Interface TAP](manual/fr/tap-interface.md) - Gestion des interfaces TAP
- [Gestion lthread](manual/fr/lthread-management.md) - Gestion lthread
- [Gestion des Périphériques](manual/fr/device-management.md) - Gestion des périphériques et pilotes
- [Répéteur Amélioré](manual/fr/enhanced-repeater.md) - Commutation virtuelle, traitement VLAN et interfaces TAP

## Guide Développeur

### Guide d'Intégration
- [Guide d'Intégration d'Applications DPDK](manual/fr/dpdk-integration-guide.md) - Comment intégrer des applications DPDK dans sdplane via l'approche DPDK-dock

### Comment développer

Pour développer avec sdplane-oss, référez-vous au fichier suivant :
- [CLAUDE.md](../CLAUDE.md) - Instructions pour les développeurs et l'IA

### Vérification de Style de Code

```bash
# Vérification du style de code (nécessite clang-format 18.1.3+)
./style/check_gnu_style.sh check

# Affichage des différences de formatage
./style/check_gnu_style.sh diff

# Formatage automatique du code
./style/check_gnu_style.sh update
```

## Licence

Ce projet est sous licence MIT. Voir le fichier [LICENSE](../LICENSE) pour les détails de la licence.

## Contact

Pour questions, problèmes ou contributions, veuillez contacter : **sdplane [at] nwlab.org**

## Achat d'Équipement d'Évaluation

Un équipement d'évaluation avec des fonctionnalités supplémentaires et des modifications logicielles peut être disponible. Veuillez visiter notre page de vente pour plus d'informations :

**[https://www.rca.co.jp/sdplane/](https://www.rca.co.jp/sdplane/)**

*Note : La page de vente est actuellement disponible en japonais seulement.*