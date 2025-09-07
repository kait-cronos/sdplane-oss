# การติดตั้ง Dependencies

**Language:** [English](../en/install-dependencies.md) | [日本語](../ja/install-dependencies.md) | [Français](../fr/install-dependencies.md) | [中文](../zh/install-dependencies.md) | [Deutsch](../de/install-dependencies.md) | [Italiano](../it/install-dependencies.md) | [한국어](../ko/install-dependencies.md) | **ไทย** | [Español](../es/install-dependencies.md)

## Dependencies

sdplane-oss ต้องการส่วนประกอบต่อไปนี้:
- **lthread** (yasuhironet/lthread): Lightweight cooperative threading
- **liburcu-qsbr**: Userspace RCU library  
- **libpcap**: Packet capture library
- **DPDK 23.11.1**: Data Plane Development Kit

## ติดตั้งแพ็คเกจ debian dependencies ของ sdplane

```bash
sudo apt update && sudo apt install liburcu-dev libpcap-dev
```

## ติดตั้งเครื่องมือสร้างและแพ็คเกจที่จำเป็นสำหรับ DPDK

```bash
sudo apt install build-essential cmake autotools-dev autoconf automake \
                 libtool pkg-config python3 python3-pip meson ninja-build \
                 python3-pyelftools libnuma-dev pkgconf
```

## ติดตั้ง lthread

```bash
git clone https://github.com/yasuhironet/lthread
cd lthread
cmake .
make
sudo make install
cd ..
```

## ติดตั้ง DPDK 23.11.1

```bash
# ดาวน์โหลด DPDK 23.11.1
wget https://fast.dpdk.org/rel/dpdk-23.11.1.tar.xz
tar xf dpdk-23.11.1.tar.xz
cd dpdk-23.11.1

# คอมไพล์และติดตั้ง DPDK
meson setup -Dprefix=/usr/local build
cd build
ninja install
cd ../..

# ตรวจสอบการติดตั้ง
pkg-config --modversion libdpdk
# ควรแสดง: 23.11.1
```