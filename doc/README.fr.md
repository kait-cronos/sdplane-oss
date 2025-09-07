<div align="center">
<img src="../sdplane-logo.png" alt="sdplane-oss Logo" width="160">
</div>

# sdplane-oss (Plan de Données Logiciel)

Un "Environnement de Développement DPDK-dock" constitué d'un shell interactif pouvant contrôler les opérations de threads DPDK et d'un environnement d'exécution de threads DPDK (sd-plane)

**Language:** [English](../README.md) | [日本語](README.ja.md) | **Français** | [中文](README.zh.md) | [Deutsch](README.de.md) | [Italiano](README.it.md) | [한국어](README.ko.md) | [ไทย](README.th.md) | [Español](README.es.md)

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
- **Topton (N305/N100)** : Mini-PC avec NICs 10G (testé)
- **Partaker (J3160)** : Mini-PC avec NICs 1G (testé)
- **PC Intel générique** : avec Intel x520 / Mellanox ConnectX5
- **Autres CPUs** : Devrait fonctionner avec CPU AMD, ARM, etc.

## 1. Installation des Dépendances

[Installation des Dépendances](manual/fr/install-dependencies.md)

## 2. Installation depuis un Package Debian Pré-compilé

Pour Intel Core i3-n305/Celeron j3160, installation rapide possible avec packages Debian.

Télécharger et installer le package Debian pré-compilé :

```bash
# Télécharger le package le plus récent pour n305
wget https://www.yasuhironet.net/download/n305/sdplane_0.1.4-36_amd64.deb
wget https://www.yasuhironet.net/download/n305/sdplane-dbgsym_0.1.4-36_amd64.ddeb

# ou pour j3160
wget https://www.yasuhironet.net/download/j3160/sdplane_0.1.4-35_amd64.deb
wget https://www.yasuhironet.net/download/j3160/sdplane-dbgsym_0.1.4-35_amd64.ddeb

# Installer le package
sudo apt install ./sdplane_0.1.4-*_amd64.deb
sudo apt install ./sdplane-dbgsym_0.1.4-*_amd64.ddeb
```

**Note** : L'utilisation de ce binaire pré-compilé sur d'autres CPUs peut causer SIGILL (Instruction Illégale). Dans ce cas, vous devez compiler par vous-même. Vérifier [téléchargements yasuhironet.net](https://www.yasuhironet.net/download/) pour la version de package la plus récente.

Passer à 5. Configuration Système.

## 3. Compilation et Installation depuis les Sources

[Compilation et Installation depuis les Sources](manual/fr/build-install-source.md)

## 4. Compilation et Installation du Package Debian

[Compilation et Installation du Package Debian](manual/fr/build-debian-package.md)

## 5. Configuration Système

[Configuration Système](manual/fr/system-configuration.md)

## 6. Configuration sdplane

### Fichiers de Configuration

sdplane utilise des fichiers de configuration pour définir le comportement au démarrage et l'environnement d'exécution.

#### Exemples de Configuration OS (`etc/`)
- `systemd/sdplane.service` : Fichier de service systemd
- `netplan/01-netcfg.yaml` : Configuration réseau avec netplan

#### Exemples de Configuration d'Application (`example-config/`)
- Fichiers de configuration exemple pour applications diverses
- Scripts de démarrage et profils de configuration

## 7. Exécuter des Applications utilisant sdplane-oss

```bash
# Exécution de base
sudo ./sdplane/sdplane

# Exécuter avec fichier de configuration
sudo ./sdplane/sdplane -f /path/to/config-file

# Connexion au CLI (depuis un autre terminal)
telnet localhost 9882

# Le shell interactif est maintenant disponible
sdplane> help
sdplane> show version
```

## Conseils

### IOMMU est requis lors de l'utilisation de vfio-pci comme pilote NIC

Les capacités de virtualisation doivent être activées :
- Intel: Intel VT-d
- AMD: AMD IOMMU / AMD-V

Ces options doivent être activées dans les paramètres BIOS.
La configuration GRUB peut aussi nécessiter une modification :

```conf
# /etc/default/grub
GRUB_CMDLINE_LINUX="iommu=pt intel_iommu=on"
```

Appliquer les modifications :
```bash
sudo update-grub
sudo reboot
```

### Configuration pour charger définitivement le module kernel Linux vfio-pci

```bash
# Créer fichier de configuration pour chargement automatique
sudo tee /etc/modules-load.d/vfio-pci.conf > /dev/null <<EOF
vfio-pci
EOF
```

### Pour les séries Mellanox ConnectX

L'installation du pilote est requise depuis le lien suivant :

https://network.nvidia.com/products/ethernet-drivers/linux/mlnx_en/

Lors de l'installation, exécutez `./install --dpdk`.
**L'option `--dpdk` est obligatoire.**

Commentez les paramètres suivants dans sdplane.conf car ils ne sont pas nécessaires :

```conf
#set device {pcie-id} driver unbind
#set device {pcie-id} driver {nom-pilote} driver_override  
#set device {pcie-id} driver {nom-pilote} bind
```

Pour les NICs Mellanox, vous devez exécuter la commande `update port status` pour actualiser les informations de port.

### Comment vérifier les numéros de bus PCIe

Vous pouvez utiliser la commande dpdk-devbind.py dans DPDK pour vérifier les numéros de bus PCIe des NICs :

```bash
# Afficher état des périphériques réseau  
dpdk-devbind.py -s

# Exemple de sortie :
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
- [RIB & Routage](manual/fr/routing.md) - Fonctions RIB et routage
- [Répéteur Amélioré](manual/fr/enhanced-repeater.md) - Configuration répéteur amélioré
- [Gestion des Ports & Statistiques](manual/fr/port-management.md) - Gestion des ports DPDK et statistiques
- [Gestion Workers & lcores & Informations Thread](manual/fr/worker-lcore-thread-management.md) - Gestion des threads workers, lcores et informations thread
- [Information Système & Surveillance](manual/fr/system-monitoring.md) - Information système et surveillance
- [Gestion des Périphériques](manual/fr/device-management.md) - Gestion des périphériques et pilotes

**Guides de Développeur :**
- [Guide d'Intégration DPDK](manual/fr/dpdk-integration-guide.md) - Comment intégrer applications DPDK
- [Débogage & Journalisation](manual/fr/debug-logging.md) - Fonctions de débogage et journalisation
- [Configuration des Files](manual/fr/queue-configuration.md) - Configuration et gestion des files
- [Interface TAP](manual/fr/tap-interface.md) - Gestion des interfaces TAP
- [Gestion VTY & Shell](manual/fr/vty-shell.md) - Gestion VTY et shell
- [Gestion lthread](manual/fr/lthread-management.md) - Gestion des threads légers coopératifs
- [Génération de Paquets](manual/fr/packet-generation.md) - Génération de paquets utilisant PKTGEN

## Guide Développeur

### Guide d'Intégration
- [Guide d'Intégration DPDK](manual/fr/dpdk-integration-guide.md) - Comment intégrer vos applications DPDK avec sdplane-oss

### Documentation
- Toute documentation de développeur est incluse dans `doc/`
- Les guides d'intégration et exemples sont dans `doc/manual/fr/`

### Style de Code

Ce projet suit GNU Coding Standards. Pour vérifier et formater le code :

```bash
# Vérifier le style (nécessite clang-format 18.1.3+)
./style/check_gnu_style.sh check

# Formater automatiquement le code
./style/check_gnu_style.sh update  

# Afficher les différences
./style/check_gnu_style.sh diff
```

Installer les outils requis :
```bash
# Pour Ubuntu 24.04
sudo apt install clang-format-18
```

## Licence

Ce projet est sous licence Apache 2.0 - voir le fichier [LICENSE](LICENSE) pour les détails.

## Contact

- GitHub: https://github.com/kait-cronos/sdplane-oss  
- Issues: https://github.com/kait-cronos/sdplane-oss/issues

## Achat d'Équipement d'Évaluation

L'équipement d'évaluation peut inclure des fonctionnalités supplémentaires et des modifications logicielles.

Pour les demandes de renseignements sur l'équipement d'évaluation, contactez-nous via GitHub Issues ou directement par e-mail.