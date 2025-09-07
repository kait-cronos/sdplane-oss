# สร้างและติดตั้งแพ็คเกจ Debian

**Language:** [English](../en/build-debian-package.md) | [Japanese](../ja/build-debian-package.md) | [Français](../fr/build-debian-package.md) | [中文](../zh/build-debian-package.md) | [Deutsch](../de/build-debian-package.md) | [Italiano](../it/build-debian-package.md) | [한국어](../ko/build-debian-package.md) | **ไทย** | [Español](../es/build-debian-package.md)

## ติดตั้งแพ็คเกจที่จำเป็น
```bash
sudo apt install build-essential cmake devscripts debhelper
```

## สร้างแพ็คเกจ Debian ของ sdplane-oss
```bash
# ทำความสะอาดพื้นที่ทำงานก่อน
(cd build && make distclean)
make distclean

# สร้างแพ็คเกจ Debian จากซอร์สโค้ด
bash build-debian.sh

# ติดตั้งแพ็คเกจที่สร้างขึ้น (จะอยู่ในไดเรกทอรีหลัก)
sudo apt install ../sdplane_*.deb
```