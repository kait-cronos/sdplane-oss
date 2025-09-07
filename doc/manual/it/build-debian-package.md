# Creazione e Installazione del Pacchetto Debian

**Language:** [English](../en/build-debian-package.md) | [Japanese](../ja/build-debian-package.md) | [Français](../fr/build-debian-package.md) | [中文](../zh/build-debian-package.md) | [Deutsch](../de/build-debian-package.md) | **Italiano** | [한국어](../ko/build-debian-package.md) | [ไทย](../th/build-debian-package.md) | [Español](../es/build-debian-package.md)

## Installazione dei pacchetti prerequisiti
```bash
sudo apt install build-essential cmake devscripts debhelper
```

## Compilazione del Pacchetto Debian sdplane-oss
```bash
# Prima assicurarsi di iniziare in uno spazio pulito
(cd build && make distclean)
make distclean

# Compilare pacchetto Debian dal codice sorgente
bash build-debian.sh

# Installare il pacchetto generato (sarà prodotto nella directory parent)
sudo apt install ../sdplane_*.deb
```