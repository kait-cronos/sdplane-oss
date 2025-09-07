# Configuration Système

**Language:** [English](../en/system-configuration.md) | [Japanese](../ja/system-configuration.md) | **Français**

- **Hugepages** : Configurer les hugepages système pour DPDK
- **Réseau** : Utiliser netplan pour la configuration des interfaces réseau
- **Firewall** : Le port telnet 9882/tcp est requis pour CLI

**⚠️ Le CLI n'a pas d'authentification. Il est recommandé d'autoriser les connexions seulement depuis localhost ⚠️**

## Configuration des Hugepages
```bash
# Éditer la configuration GRUB
sudo vi /etc/default/grub

# Ajouter une de ces lignes :
# Pour hugepages 2MB (1536 pages = ~3GB) :
GRUB_CMDLINE_LINUX="hugepages=1536"

# Ou pour hugepages 1GB (8 pages = 8GB) :
GRUB_CMDLINE_LINUX="default_hugepagesz=1G hugepagesz=1G hugepages=8"

# Mettre à jour GRUB et redémarrer
sudo update-grub
sudo reboot
```

## Installation du Module Kernel DPDK IGB (Optionnel)

Pour les NIC qui ne fonctionnent pas avec vfio-pci, installez optionnellement igb_uio :

```bash
git clone http://dpdk.org/git/dpdk-kmods
cd dpdk-kmods/linux/igb_uio
make
sudo mkdir -p /lib/modules/`uname -r`/extra/dpdk/
sudo cp igb_uio.ko /lib/modules/`uname -r`/extra/dpdk/
echo igb_uio | sudo tee /etc/modules-load.d/igb_uio.conf
```

## Configuration des Pilotes UIO DPDK

DPDK nécessite des pilotes I/O utilisateur (UIO) pour accéder aux interfaces réseau depuis les applications utilisateur.

### NIC Standard

Pour la plupart des cartes réseau standards, vous avez trois options de pilotes UIO :

1. **vfio-pci** (Recommandé)
   - Option la plus sécurisée et moderne
   - Nécessite le support IOMMU (Intel VT-d ou AMD-Vi)
   - Aucune compilation de module kernel supplémentaire nécessaire

2. **uio_pci_generic**
   - Pilote UIO générique inclus dans le kernel Linux
   - Aucune installation supplémentaire requise
   - Fonctionnalité limitée par rapport à vfio-pci

3. **igb_uio**
   - Pilote UIO spécifique à DPDK
   - Nécessite compilation et installation manuelles (voir ci-dessus)
   - Fournit des fonctionnalités supplémentaires pour matériel ancien

### NIC avec Pilotes Bifurqués

Certaines NIC, comme les **séries Mellanox ConnectX**, fournissent des pilotes bifurqués qui permettent d'utiliser le même port physique simultanément par le réseau kernel et les applications DPDK. Pour ces NIC :

- Aucune configuration de pilote UIO n'est nécessaire
- La NIC peut rester liée à son pilote kernel
- Les applications DPDK peuvent accéder directement au matériel

### Configuration des Pilotes UIO

Pour configurer les pilotes UIO, vous devez :

1. **Charger le module pilote** :
```bash
# Pour vfio-pci (nécessite IOMMU activé dans BIOS/UEFI et kernel)
sudo modprobe vfio-pci

# Pour uio_pci_generic
sudo modprobe uio_pci_generic

# Pour igb_uio (après installation)
sudo modprobe igb_uio
```

2. **Faire charger le pilote au démarrage** en créant un fichier de configuration dans `/etc/modules-load.d/` :
```bash
# Exemple pour vfio-pci
echo "vfio-pci" | sudo tee /etc/modules-load.d/vfio-pci.conf

# Exemple pour uio_pci_generic
echo "uio_pci_generic" | sudo tee /etc/modules-load.d/uio_pci_generic.conf

# Exemple pour igb_uio
echo "igb_uio" | sudo tee /etc/modules-load.d/igb_uio.conf
```

3. **Lier les NIC au pilote UIO** en utilisant le script `dpdk-devbind.py` de DPDK :
```bash
# Délier du pilote kernel et lier au pilote UIO
sudo dpdk-devbind.py --bind=vfio-pci 0000:01:00.0
sudo dpdk-devbind.py --bind=uio_pci_generic 0000:01:00.1
sudo dpdk-devbind.py --bind=igb_uio 0000:01:00.2
```

**Note** : Remplacez `0000:01:00.0` par votre adresse de périphérique PCI réelle. Utilisez `lspci` ou `dpdk-devbind.py --status` pour identifier vos NIC.

**Alternative** : Au lieu d'utiliser `dpdk-devbind.py` manuellement, vous pouvez configurer les liaisons de périphérique dans votre fichier `sdplane.conf` en utilisant les commandes `set device XX:XX.X driver ...` au début du fichier de configuration. Cela permet à sdplane de gérer automatiquement la liaison de périphérique au démarrage.