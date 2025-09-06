<div align="center">
<img src="../sdplane-logo.png" alt="sdplane-oss Logo" width="160">
</div>

# sdplane-oss (Software Data Plane)

"สภาพแวดล้อมการพัฒนา DPDK-dock" ที่ประกอบด้วย interactive shell ที่สามารถควบคุมการทำงานของ DPDK thread และสภาพแวดล้อมการทำงานของ DPDK thread (sd-plane)

**Language:** [English](../README.md) | [日本語](README.ja.md) | [Français](README.fr.md) | [中文](README.zh.md) | [Deutsch](README.de.md) | [Italiano](README.it.md) | [한국어](README.ko.md) | **ไทย** | [Español](README.es.md)

## คุณสมบัติ

- **การประมวลผลแพ็กเกตประสิทธิภาพสูง**:
  การประมวลผลแพ็กเกตแบบ zero-copy ใน user space โดยใช้ DPDK
- **การส่งต่อ Layer 2/3**:
  การส่งต่อ L2 และ L3 แบบรวมด้วยการสนับสนุน ACL, LPM และ FIB
- **การสร้างแพ็กเกต**:
  ตัวสร้างแพ็กเกตแบบบูรณาการสำหรับการทดสอบและ benchmarking
- **การเสมือนเครือข่าย**:
  การสนับสนุน TAP interface และฟังก์ชั่น VLAN switching
- **การจัดการ CLI**:
  อินเทอร์เฟซบรรทัดคำสั่งแบบโต้ตอบสำหรับการกำหนดค่าและติดตาม
- **Multi-threading**:
  โมเดล threading แบบร่วมมือกับ worker ต่อ core

### สถาปัตยกรรม
- **แอปพลิเคชันหลัก**: ตรรกะเราเตอร์กลางและการเริ่มต้น
- **โมดูล DPDK**: การส่งต่อ L2/L3 และการสร้างแพ็กเกต
- **ระบบ CLI**: อินเทอร์เฟซบรรทัดคำสั่งพร้อมการเติมคำและความช่วยเหลือ
- **Threading**: multitasking แบบร่วมมือตาม lthread
- **การเสมือน**: TAP interface และ virtual switching

## ระบบที่รองรับ

### ความต้องการซอฟต์แวร์
- **OS**:
  Ubuntu 24.04 LTS (รองรับในปัจจุบัน)
- **NIC**:
  [ไดรเวอร์](https://doc.dpdk.org/guides/nics/) | [NIC ที่รองรับ](https://core.dpdk.org/supported/)
- **หน่วยความจำ**:
  ต้องการการสนับสนุน hugepage
- **CPU**:
  แนะนำโปรเซสเซอร์แบบหลายคอร์

### แพลตฟอร์มฮาร์ดแวร์เป้าหมาย

โครงการนี้ได้รับการทดสอบบน:
- **Topton (N305/N100)**: Mini-PC พร้อม NIC 10G
- **Partaker (J3160)**: Mini-PC พร้อม NIC 1G
- **Intel Generic PC**: พร้อม Intel x520 / Mellanox ConnectX5
- **CPU อื่น ๆ**: ควรทำงานได้กับโปรเซสเซอร์ AMD, ARM ฯลฯ

## 1. การติดตั้ง Dependencies

### Dependencies

sdplane-oss ต้องการส่วนประกอบต่อไปนี้:
- **lthread** (yasuhironet/lthread): การทำ threading แบบร่วมมือที่มีน้ำหนักเบา
- **liburcu-qsbr**: ไลบรารี RCU ใน userspace  
- **libpcap**: ไลบรารีการจับแพ็กเกต
- **DPDK 23.11.1**: Data Plane Development Kit

### ติดตั้งแพ็กเกจ debian dependencies ของ sdplane

```bash
sudo apt update && sudo apt install liburcu-dev libpcap-dev
```

### ติดตั้งเครื่องมือ Build และข้อกำหนดเบื้องต้นของ DPDK

```bash
sudo apt install build-essential cmake autotools-dev autoconf automake \
                 libtool pkg-config python3 python3-pip meson ninja-build \
                 python3-pyelftools libnuma-dev pkgconf
```

### ติดตั้ง lthread

```bash
git clone https://github.com/yasuhironet/lthread
cd lthread
cmake .
make
sudo make install
cd ..
```

### ติดตั้ง DPDK 23.11.1

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

## 2. การเริ่มต้นอย่างรวดเร็วด้วยแพ็กเกจ Debian สำหรับ Intel Core i3-n305/Celeron j3160

สำหรับ Intel Core i3-n305/Celeron j3160 สามารถติดตั้งอย่างรวดเร็วด้วยแพ็กเกจ Debian ได้

ดาวน์โหลดและติดตั้งแพ็กเกจ Debian ที่คอมไพล์ไว้แล้ว:

```bash
# ดาวน์โหลดแพ็กเกจล่าสุดสำหรับ n305
wget https://www.yasuhironet.net/download/n305/sdplane_0.1.4-36_amd64.deb
wget https://www.yasuhironet.net/download/n305/sdplane-dbgsym_0.1.4-36_amd64.ddeb

# หรือสำหรับ j3160
wget https://www.yasuhironet.net/download/j3160/sdplane_0.1.4-35_amd64.deb
wget https://www.yasuhironet.net/download/j3160/sdplane-dbgsym_0.1.4-35_amd64.ddeb

# ติดตั้งแพ็กเกจ
sudo apt install ./sdplane_0.1.4-*_amd64.deb
sudo apt install ./sdplane-dbgsym_0.1.4-*_amd64.ddeb
```

**หมายเหตุ**: ตรวจสอบ [ดาวน์โหลด yasuhironet.net](https://www.yasuhironet.net/download/) สำหรับเวอร์ชันแพ็กเกจล่าสุด

ไปที่ 5. การกำหนดค่าระบบ

## 3. Build จาก Source Code

**โดยทั่วไปแล้วให้ปฏิบัติตามขั้นตอนนี้**

### ติดตั้งแพ็กเกจ Ubuntu ที่จำเป็น

#### สำหรับ Build จาก Source Code
```bash
# เครื่องมือ build หลัก
sudo apt install build-essential cmake autotools-dev autoconf automake libtool pkg-config

# ข้อกำหนดเบื้องต้นของ DPDK
sudo apt install python3 python3-pip meson ninja-build python3-pyelftools libnuma-dev pkgconf
```

#### แพ็กเกจเสริม
```bash
sudo apt install etckeeper tig bridge-utils \
                 iptables-persistent fail2ban dmidecode screen ripgrep
```

### Build sdplane-oss จาก Source Code

```bash
# clone repository
git clone https://github.com/kait-cronos/sdplane-oss
cd sdplane-oss

# สร้างไฟล์ build
sh autogen.sh

# กำหนดค่าและ build
mkdir build
cd build
CFLAGS="-g -O0" sh ../configure
make
```

## 4. การสร้างและติดตั้งแพ็กเกจ Debian ของ sdplane-oss

### ติดตั้งแพ็กเกจที่จำเป็น
```bash
sudo apt install build-essential cmake devscripts debhelper
```

### Build แพ็กเกจ Debian ของ sdplane-oss
```bash
# ก่อนอื่นให้แน่ใจว่าเริ่มต้นในพื้นที่ทำงานที่สะอาด
(cd build && make distclean)
make distclean

# build แพ็กเกจ Debian จาก source code
bash build-debian.sh

# ติดตั้งแพ็กเกจที่สร้างขึ้น (จะถูกสร้างใน directory หลัก)
sudo apt install ../sdplane_*.deb
```

## 5. การกำหนดค่าระบบ

- **Hugepages**: กำหนดค่า hugepage ของระบบสำหรับ DPDK
- **เครือข่าย**: ใช้ netplan สำหรับการกำหนดค่า network interface
- **Firewall**: จำเป็นต้องมีพอร์ต telnet 9882/tcp สำหรับ CLI

**⚠️ CLI ไม่มีการตรวจสอบสิทธิ์ แนะนำให้อนุญาตการเชื่อมต่อจาก localhost เท่านั้น ⚠️**

### กำหนดค่า Hugepages
```bash
# แก้ไขการกำหนดค่า GRUB
sudo vi /etc/default/grub

# เพิ่ม hugepages ลงในพารามิเตอร์ GRUB_CMDLINE_LINUX
# ตัวอย่างการเพิ่ม hugepages=1024:
GRUB_CMDLINE_LINUX="hugepages=1024"

# อัพเดต GRUB
sudo update-grub

# รีสตาร์ทระบบ
sudo reboot

# ตรวจสอบ hugepages หลังจากรีสตาร์ท
cat /proc/meminfo | grep -E "^HugePages|^Hugepagesize"
```

### การติดตั้งโมดูล Kernel IGB ของ DPDK (เสริม)

หาก NIC ของคุณไม่ทำงานกับ vfio-pci ให้ติดตั้ง igb_uio

```bash
git clone http://dpdk.org/git/dpdk-kmods
cd dpdk-kmods/linux/igb_uio
make
sudo make install
cd ../../..

# โมดูลจะถูกติดตั้งใน /lib/modules/$(uname -r)/extra/igb_uio.ko
```

## 6. การกำหนดค่า sdplane

### ไฟล์การกำหนดค่า

sdplane ใช้ไฟล์การกำหนดค่าเพื่อกำหนดพฤติกรรมการเริ่มต้นและสภาพแวดล้อมการทำงาน

#### ตัวอย่างการกำหนดค่า OS (`etc/`)
- `systemd/sdplane.service`: ไฟล์ service ของ systemd
- `netplan/01-netcfg.yaml`: การกำหนดค่าเครือข่ายด้วย netplan

#### ตัวอย่างการกำหนดค่าแอปพลิเคชัน (`example-config/`)
- ไฟล์การกำหนดค่าตัวอย่างสำหรับแอปพลิเคชันต่างๆ
- สคริปต์เริ่มต้นและโปรไฟล์การกำหนดค่า

## 7. รันแอปพลิเคชันโดยใช้ sdplane-oss

```bash
# การทำงานพื้นฐาน
sudo ./sdplane/sdplane

# รันด้วยไฟล์การกำหนดค่า
sudo ./sdplane/sdplane -f /path/to/config-file

# เชื่อมต่อกับ CLI (จากเทอร์มินัลอื่น)
telnet localhost 9882

# shell แบบโต้ตอบพร้อมใช้งานแล้ว
sdplane> help
sdplane> show version
```

## เคล็ดลับ

### จำเป็นต้องมี IOMMU เมื่อใช้ vfio-pci เป็นไดรเวอร์ NIC

ความสามารถในการทำ virtualization ต้องเปิดใช้งาน:
- Intel: Intel VT-d
- AMD: AMD IOMMU / AMD-V

ตัวเลือกเหล่านี้ต้องเปิดใช้งานในการตั้งค่า BIOS
การกำหนดค่า GRUB อาจจำเป็นต้องเปลี่ยนแปลงด้วย:

```conf
# /etc/default/grub
GRUB_CMDLINE_LINUX="iommu=pt intel_iommu=on"
```

ใช้การเปลี่ยนแปลง:
```bash
sudo update-grub
sudo reboot
```

### การกำหนดค่าเพื่อโหลดโมดูล kernel Linux vfio-pci แบบถาวร

```bash
# สร้างไฟล์การกำหนดค่าสำหรับการโหลดอัตโนมัติ
sudo tee /etc/modules-load.d/vfio-pci.conf > /dev/null <<EOF
vfio-pci
EOF
```

### สำหรับ Mellanox ConnectX Series

จำเป็นต้องติดตั้งไดรเวอร์จากลิงก์ต่อไปนี้:

https://network.nvidia.com/products/ethernet-drivers/linux/mlnx_en/

ระหว่างการติดตั้ง ให้รัน `./install --dpdk`
**ตัวเลือก `--dpdk` เป็นสิ่งบังคับ**

ให้แสดงความเห็นการตั้งค่าต่อไปนี้ใน sdplane.conf เนื่องจากไม่จำเป็น:

```conf
#set device {pcie-id} driver unbind
#set device {pcie-id} driver {driver-name} driver_override  
#set device {pcie-id} driver {driver-name} bind
```

สำหรับ NIC ของ Mellanox คุณจำเป็นต้องรันคำสั่ง `update port status` เพื่ออัพเดตข้อมูลพอร์ต

### วิธีตรวจสอบหมายเลขบัส PCIe

คุณสามารถใช้คำสั่ง dpdk-devbind.py ใน DPDK เพื่อตรวจสอบหมายเลขบัส PCIe ของ NIC:

```bash
# แสดงสถานะอุปกรณ์เครือข่าย  
dpdk-devbind.py -s

# ตัวอย่างผลลัพธ์:
Network devices using kernel driver
===================================
0000:04:00.0 'NetXtreme BCM5720 Gigabit Ethernet PCIe 165f' numa_node=0 if=eno8303 drv=tg3 unused= *Active*
0000:b1:00.0 'MT27800 Family [ConnectX-5] 1017' numa_node=1 if=enp177s0np0 drv=mlx5_core unused= *Active*
```

### ลำดับของ worker thread ในไฟล์การกำหนดค่า

หากคุณกำหนดค่า worker `rib-manager`, `neigh-manager` และ `netlink-thread` พวกมันต้องถูกกำหนดค่าในลำดับนี้หากใช้งาน

### การเริ่มต้น DPDK

ควรเรียกคำสั่งเดียวที่เรียก `rte_eal_init()` จากไฟล์การกำหนดค่า ฟังก์ชัน `rte_eal_init()` จะถูกเรียกโดยคำสั่งเช่น `rte_eal_init`, `pktgen init`, `l2fwd init` และ `l3fwd init`

## คู่มือผู้ใช้ (Manual)

คู่มือผู้ใช้ที่ครอบคลุมและการอ้างอิงคำสั่งมีให้:

- [คู่มือผู้ใช้](manual/th/README.md) - ภาพรวมที่สมบูรณ์และการจำแนกคำสั่ง

**คู่มือแอปพลิเคชัน:**
- [แอปพลิเคชัน L2 Repeater](manual/th/l2-repeater-application.md) - การส่งต่อแพ็กเกต Layer 2 อย่างง่ายพร้อม MAC learning
- [แอปพลิเคชัน Enhanced Repeater](manual/th/enhanced-repeater-application.md) - การสวิตช์ที่รับรู้ VLAN พร้อม TAP interface  
- [แอปพลิเคชันตัวสร้างแพ็กเกต](manual/th/packet-generator-application.md) - การสร้างทราฟฟิกประสิทธิภาพสูงและการทดสอบ

**คู่มือการกำหนดค่า:**
- [RIB & Routing](manual/th/routing.md) - ฟังก์ชั่น RIB และ routing
- [Enhanced Repeater](manual/th/enhanced-repeater.md) - การกำหนดค่า enhanced repeater
- [การจัดการพอร์ต & สถิติ](manual/th/port-management.md) - การจัดการพอร์ต DPDK และสถิติ
- [การจัดการ Worker & lcore & ข้อมูลเธรด](manual/th/worker-lcore-thread-management.md) - การจัดการ worker threads, lcore และข้อมูลเธรด
- [ข้อมูลระบบ & การติดตาม](manual/th/system-monitoring.md) - ข้อมูลระบบและการติดตาม
- [การจัดการอุปกรณ์](manual/th/device-management.md) - การจัดการอุปกรณ์และไดรเวอร์

**คู่มือนักพัฒนา:**
- [คู่มือการผสานรวม DPDK](manual/th/dpdk-integration-guide.md) - วิธีผสานรวมแอปพลิเคชัน DPDK
- [Debug & Logging](manual/th/debug-logging.md) - ฟังก์ชั่น debug และ logging
- [การกำหนดค่า Queue](manual/th/queue-configuration.md) - การกำหนดค่าและจัดการ queue
- [TAP Interface](manual/th/tap-interface.md) - การจัดการ TAP interface
- [การจัดการ VTY & Shell](manual/th/vty-shell.md) - การจัดการ VTY และ shell
- [การจัดการ lthread](manual/th/lthread-management.md) - การจัดการ thread แบบร่วมมือที่มีน้ำหนักเบา
- [การสร้างแพ็กเกต](manual/th/packet-generation.md) - การสร้างแพ็กเกตโดยใช้ PKTGEN

## คู่มือนักพัฒนา

### คู่มือการผสานรวม
- [คู่มือการผสานรวมแอปพลิเคชัน DPDK](manual/th/dpdk-integration-guide.md) - วิธีผสานรวมแอปพลิเคชัน DPDK ของคุณกับ sdplane-oss โดยใช้วิธี DPDK-dock

### เอกสาร
- เอกสารนักพัฒนาทั้งหมดอยู่ใน `doc/`
- คู่มือการผสานรวมและตัวอย่างอยู่ใน `doc/manual/th/`

### รูปแบบโค้ด

โครงการนี้ปฏิบัติตาม GNU Coding Standards เพื่อตรวจสอบและจัดรูปแบบโค้ด:

```bash
# ตรวจสอบรูปแบบ (ต้องการ clang-format 18.1.3+)
./style/check_gnu_style.sh check

# จัดรูปแบบโค้ดอัตโนมัติ
./style/check_gnu_style.sh update  

# แสดงความแตกต่าง
./style/check_gnu_style.sh diff
```

ติดตั้งเครื่องมือที่จำเป็น:
```bash
# สำหรับ Ubuntu 24.04
sudo apt install clang-format-18
```

## ใบอนุญาต

โครงการนี้อยู่ภายใต้ใบอนุญาต Apache 2.0 - ดูไฟล์ [LICENSE](LICENSE) สำหรับรายละเอียด

## ติดต่อ

- GitHub: https://github.com/kait-cronos/sdplane-oss  
- Issues: https://github.com/kait-cronos/sdplane-oss/issues

## การซื้ออุปกรณ์ประเมิน

อุปกรณ์ประเมินอาจรวมคุณสมบัติเพิ่มเติมและการปรับเปลี่ยนซอฟต์แวร์

สำหรับคำถามเกี่ยวกับอุปกรณ์ประเมิน ติดต่อเราผ่าน GitHub Issues หรือทางอีเมลโดยตรง