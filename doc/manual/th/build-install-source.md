# สร้างและติดตั้งจากซอร์สโค้ด

**Language:** [English](../en/build-install-source.md) | [Japanese](../ja/build-install-source.md) | [Français](../fr/build-install-source.md) | [中文](../zh/build-install-source.md) | [Deutsch](../de/build-install-source.md) | [Italiano](../it/build-install-source.md) | [한국어](../ko/build-install-source.md) | **ไทย** | [Español](../es/build-install-source.md)

**โดยทั่วไป โปรดปฏิบัติตามขั้นตอนนี้**

## ติดตั้งแพ็คเกจ Ubuntu ที่จำเป็น

### สำหรับการสร้างจากซอร์สโค้ด
```bash
# เครื่องมือสร้างที่จำเป็น
sudo apt install build-essential cmake autotools-dev autoconf automake libtool pkg-config

# ความต้องการเบื้องต้นสำหรับ DPDK
sudo apt install python3 python3-pip meson ninja-build python3-pyelftools libnuma-dev pkgconf
```

### แพ็คเกจเสริม
```bash
sudo apt install etckeeper tig bridge-utils \
                 iptables-persistent fail2ban dmidecode screen ripgrep
```

## สร้าง sdplane-oss จากซอร์สโค้ด

```bash
# โคลน repository
git clone https://github.com/kait-cronos/sdplane-oss
cd sdplane-oss

# สร้างไฟล์ build
sh autogen.sh

# กำหนดค่าและคอมไพล์
mkdir build
cd build
CFLAGS="-g -O0" sh ../configure
make

# ติดตั้งลงในระบบ
# จะติดตั้งไปที่ $prefix (ค่าเริ่มต้น: /usr/local/sbin)
sudo make install
```