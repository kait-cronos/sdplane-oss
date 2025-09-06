# คู่มือการบูรณาการ DPDK

**ภาษา / Language:** [English](../dpdk-integration-guide.md) | [日本語](../ja/dpdk-integration-guide.md) | [Français](../fr/dpdk-integration-guide.md) | [中文](../zh/dpdk-integration-guide.md) | [Deutsch](../de/dpdk-integration-guide.md) | [Italiano](../it/ดoใผ-integration-guide.md) | [한국어](../ko/dpdk-integration-guide.md) | **ไทย**

คู่มือนี้ให้ข้อมูลที่ครอบคลุมเกี่ยวกับการบูรณาการ DPDK (Data Plane Development Kit) กับ sdplane รวมถึงการติดตั้ง การตั้งค่า และการปรับแต่งประสิทธิภาพ

## ภาพรวม DPDK

### DPDK คืออะไร
DPDK (Data Plane Development Kit) เป็นชุด library และ driver สำหรับการประมวลผลแพ็กเก็ตความเร็วสูง:
- **User Space Processing**: การประมวลผลแพ็กเก็ตใน user space
- **Zero-copy**: การหลีกเลี่ยงการคัดลอกข้อมูลที่ไม่จำเป็น
- **Poll Mode Driver**: การหลีกเลี่ยง interrupt overhead
- **Lock-free**: การใช้ algorithm ที่ไม่ต้องใช้ lock

### ประโยชน์ของ DPDK
- **ประสิทธิภาพสูง**: การประมวลผลแพ็กเก็ตหลายล้านแพ็กเก็ตต่อวินาที
- **ความล่าช้าต่ำ**: การลด latency ในการประมวลผลแพ็กเก็ต
- **ความยืดหยุ่น**: การสนับสนุน application หลากหลายประเภท
- **การปรับขนาด**: การรองรับระบบ multi-core

## การติดตั้ง DPDK

### ข้อกำหนดระบบ
- **Operating System**: Linux (Ubuntu 20.04+ แนะนำ)
- **CPU**: Intel x86_64 หรือ ARM64
- **Memory**: หน่วยความจำเพียงพอสำหรับ hugepage
- **Network Cards**: NICs ที่รองรับ DPDK

### การติดตั้งจาก Package Manager
```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install dpdk dpdk-dev

# ตรวจสอบการติดตั้ง
dpdk-hugepages.py --show
```

### การติดตั้งจาก Source Code
```bash
# ดาวน์โหลด DPDK source
wget http://fast.dpdk.org/rel/dpdk-21.11.tar.xz
tar xf dpdk-21.11.tar.xz
cd dpdk-21.11

# สร้างและติดตั้ง
meson build
cd build
ninja
sudo ninja install
```

## การตั้งค่าระบบ

### การตั้งค่า Hugepages
Hugepages เป็นสิ่งจำเป็นสำหรับ DPDK:

```bash
# ตั้งค่า hugepages ชั่วคราว
echo 1024 | sudo tee /sys/kernel/mm/hugepages/hugepages-2048kB/nr_hugepages

# ตั้งค่า hugepages ถาวร
echo 'vm.nr_hugepages=1024' | sudo tee -a /etc/sysctl.conf

# สร้าง mount point
sudo mkdir -p /mnt/huge
sudo mount -t hugetlbfs nodev /mnt/huge

# เพิ่มใน /etc/fstab สำหรับการ mount อัตโนมัติ
echo 'nodev /mnt/huge hugetlbfs defaults 0 0' | sudo tee -a /etc/fstab
```

### การตั้งค่า IOMMU (สำหรับ VFIO)
```bash
# เพิ่มพารามิเตอร์ kernel
sudo vim /etc/default/grub
# เพิ่ม: GRUB_CMDLINE_LINUX="intel_iommu=on"

# อัปเดต GRUB
sudo update-grub
sudo reboot
```

### การโหลด Kernel Modules
```bash
# โหลด UIO modules
sudo modprobe uio
sudo modprobe uio_pci_generic

# หรือโหลด VFIO modules (แนะนำ)
sudo modprobe vfio-pci
```

## การตั้งค่าอุปกรณ์เครือข่าย

### การระบุอุปกรณ์
```bash
# แสดงรายการ PCI devices
lspci | grep Ethernet

# ใช้ dpdk-devbind.py
sudo dpdk-devbind.py --status
```

### การ bind ไดรเวอร์
```bash
# bind กับ vfio-pci (แนะนำ)
sudo dpdk-devbind.py --bind=vfio-pci 0000:01:00.0

# หรือ bind กับ uio_pci_generic
sudo dpdk-devbind.py --bind=uio_pci_generic 0000:01:00.0

# ตรวจสอบสถานะ
sudo dpdk-devbind.py --status
```

### การ unbind อุปกรณ์
```bash
# unbind จาก DPDK driver
sudo dpdk-devbind.py --unbind 0000:01:00.0

# bind กลับไปยัง kernel driver
sudo dpdk-devbind.py --bind=ixgbe 0000:01:00.0
```

## การบูรณาการกับ sdplane

### การเริ่มต้น DPDK ใน sdplane
```bash
# ตั้งค่าพารามิเตอร์ EAL
set rte_eal argv -c 0x7 -n 4 --socket-mem 1024

# เริ่มต้น DPDK
rte_eal_init

# ตั้งค่า memory pool
set mempool
```

### การตั้งค่าพอร์ต
```bash
# แสดงอุปกรณ์ที่ใช้ได้
show devices

# ตั้งค่าพอร์ต
set port all dev-configure 1 4
set port all nrxdesc 1024
set port all ntxdesc 1024

# เริ่มต้นพอร์ต
start port all
```

### การตั้งค่า Worker
```bash
# ตั้งค่า lcore worker
set worker lcore 1 l2fwd
start worker lcore 1

# ตรวจสอบสถานะ
show worker
show port
```

## การปรับแต่งประสิทธิภาพ

### การตั้งค่า CPU
```bash
# การจัดสรร CPU core
set rte_eal argv -c 0xF  # ใช้ core 0-3

# การตั้งค่า CPU affinity
taskset -c 1 ./sdplane/sdplane
```

### การปรับแต่งหน่วยความจำ
```bash
# เพิ่มขนาด hugepage
echo 2048 | sudo tee /sys/kernel/mm/hugepages/hugepages-2048kB/nr_hugepages

# ตั้งค่า socket memory
set rte_eal argv --socket-mem 2048,2048
```

### การปรับแต่ง NIC
```bash
# ปรับแต่ง descriptor rings
set port all nrxdesc 2048
set port all ntxdesc 2048

# ปรับแต่ง flow control
set port all flowcontrol rx off
set port all flowcontrol tx off
```

## การตรวจสอบประสิทธิภาพ

### การวัดผล Throughput
```bash
# ตรวจสอบสถิติพอร์ต
show port statistics pps
show port statistics Bps

# ตรวจสอบการใช้งาน CPU
show loop-count l2fwd pps
show thread counter
```

### การวิเคราะห์ขวดคอ
```bash
# ตรวจสอบ memory pool
show mempool

# ตรวจสอบ queue utilization
show thread qconf

# ตรวจสอบ packet drops
show port statistics
```

## การแก้ไขปัญหา

### ปัญหาทั่วไป

#### Hugepage ไม่เพียงพอ
```bash
# ตรวจสอบ hugepage
cat /proc/meminfo | grep Huge

# เพิ่ม hugepage
echo 2048 | sudo tee /sys/kernel/mm/hugepages/hugepages-2048kB/nr_hugepages
```

#### อุปกรณ์ bind ไม่ได้
```bash
# ตรวจสอบ IOMMU
dmesg | grep -i iommu

# ตรวจสอบ kernel modules
lsmod | grep vfio
lsmod | grep uio
```

#### ประสิทธิภาพต่ำ
```bash
# ตรวจสอบ CPU frequency scaling
cat /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor

# ตั้งค่า performance mode
echo performance | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor
```

### การดีบัก
```bash
# เปิดใช้งาน DPDK debugging
export RTE_LOG_LEVEL=8

# ใช้ debugging tools
gdb ./sdplane/sdplane
perf record -g ./sdplane/sdplane
```

## Best Practices

### การออกแบบแอปพลิเคชัน
- ใช้ polling แทน interrupt
- หลีกเลี่ยงการ system call ใน data path
- ใช้ lockless data structures
- ปรับแต่ง cache line alignment

### การจัดการทรัพยากร
- จัดสรร hugepage อย่างเหมาะสม
- ใช้ NUMA-aware memory allocation
- จัดการ CPU affinity
- ติดตั้ง real-time kernel หากจำเป็น

### การติดตั้งในระบบจริง
- ใช้ dedicated CPU core
- ปิดการรบกวนจาก OS
- ตั้งค่า network interrupt coalescing
- ใช้ SR-IOV เมื่อเป็นไปได้

## ความปลอดภัย

### การควบคุมการเข้าถึง
- รัน application ด้วยสิทธิ์ที่จำเป็น
- ใช้ user/group ที่เหมาะสม
- จำกัดการเข้าถึง hugepage directory

### การตั้งค่า VFIO
- เปิดใช้งาน IOMMU
- ใช้ VFIO groups อย่างถูกต้อง
- ตรวจสอบ device isolation

## การรองรับ Hardware

### NICs ที่รองรับ
- Intel 82599, X540, X550 (ixgbe)
- Intel X710, XL710 (i40e)  
- Intel E810 (ice)
- Mellanox ConnectX-4, ConnectX-5, ConnectX-6 (mlx5)

### การตั้งค่าเฉพาะ Hardware
```bash
# สำหรับ Intel NICs
set device <pci-addr> driver ixgbe bind

# สำหรับ Mellanox NICs
set device <pci-addr> driver mlx5_core bind
```

## การอ้างอิง

- [DPDK Official Documentation](https://doc.dpdk.org/)
- [DPDK Getting Started Guide](https://doc.dpdk.org/guides/linux_gsg/)
- [DPDK Programming Guide](https://doc.dpdk.org/guides/prog_guide/)
- [DPDK Sample Applications](https://doc.dpdk.org/guides/sample_app_ug/)

สำหรับข้อมูลเฉพาะเกี่ยวกับการใช้งาน DPDK ใน sdplane ดู:
- [การจัดการอุปกรณ์](device-management.md)
- [การจัดการพอร์ต](port-management.md)
- [การตั้งค่าคิว](queue-configuration.md)