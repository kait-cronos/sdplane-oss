# การกำหนดค่าระบบ

**Language:** [English](../en/system-configuration.md) | [Japanese](../ja/system-configuration.md) | [Français](../fr/system-configuration.md) | [中文](../zh/system-configuration.md) | [Deutsch](../de/system-configuration.md) | [Italiano](../it/system-configuration.md) | [한국어](../ko/system-configuration.md) | **ไทย** | [Español](../es/system-configuration.md)

- **Hugepages**: กำหนดค่า hugepages ระบบสำหรับ DPDK
- **เครือข่าย**: ใช้ netplan สำหรับการกำหนดค่าอินเทอร์เฟซเครือข่าย
- **Firewall**: ต้องการพอร์ต telnet 9882/tcp สำหรับ CLI

**⚠️ CLI ไม่มีการยืนยันตัวตน แนะนำให้อนุญาตการเชื่อมต่อจาก localhost เท่านั้น ⚠️**

## กำหนดค่า Hugepages
```bash
# แก้ไขการกำหนดค่า GRUB
sudo vi /etc/default/grub

# เพิ่มบรรทัดใดบรรทัดหนึ่งต่อไปนี้:
# สำหรับ hugepages ขนาด 2MB (1536 หน้า = ~3GB):
GRUB_CMDLINE_LINUX="hugepages=1536"

# หรือสำหรับ hugepages ขนาด 1GB (8 หน้า = 8GB):
GRUB_CMDLINE_LINUX="default_hugepagesz=1G hugepagesz=1G hugepages=8"

# อัปเดต GRUB และรีบูต
sudo update-grub
sudo reboot
```

## ติดตั้งโมดูลเคอร์เนล DPDK IGB (ทางเลือก)

สำหรับ NIC ที่ไม่ทำงานกับ vfio-pci ให้ติดตั้ง igb_uio:

```bash
git clone http://dpdk.org/git/dpdk-kmods
cd dpdk-kmods/linux/igb_uio
make
sudo mkdir -p /lib/modules/`uname -r`/extra/dpdk/
sudo cp igb_uio.ko /lib/modules/`uname -r`/extra/dpdk/
echo igb_uio | sudo tee /etc/modules-load.d/igb_uio.conf
```

## กำหนดค่าไดรเวอร์ DPDK UIO

DPDK ต้องการไดรเวอร์ User I/O (UIO) เพื่อเข้าถึงอินเทอร์เฟซเครือข่ายจากแอปพลิเคชันผู้ใช้

### NIC มาตรฐาน

สำหรับการ์ดเครือข่ายมาตรฐานส่วนใหญ่ คุณมีตัวเลือกไดรเวอร์ UIO สามตัว:

1. **vfio-pci** (แนะนำ)
   - ตัวเลือกที่ปลอดภัยและทันสมัยที่สุด
   - ต้องการการสนับสนุน IOMMU (Intel VT-d หรือ AMD-Vi)
   - ไม่ต้องคอมไพล์โมดูลเคอร์เนลเพิ่มเติม

2. **uio_pci_generic**
   - ไดรเวอร์ UIO ทั่วไปที่รวมอยู่ในเคอร์เนล Linux
   - ไม่ต้องติดตั้งเพิ่มเติม
   - ฟังก์ชันจำกัดเมื่อเทียบกับ vfio-pci

3. **igb_uio**
   - ไดรเวอร์ UIO เฉพาะสำหรับ DPDK
   - ต้องคอมไพล์และติดตั้งด้วยตนเอง (ดูด้านบน)
   - ให้คุณสมบัติเพิ่มเติมสำหรับฮาร์ดแวร์รุ่นเก่า

### NIC ที่มีไดรเวอร์แบบแยก

NIC บางตัว เช่น **ชุด Mellanox ConnectX** มีไดรเวอร์แบบแยกที่อนุญาตให้พอร์ตทางกายภาพเดียวกันถูกใช้พร้อมกันโดยเครือข่ายเคอร์เนลและแอปพลิเคชัน DPDK สำหรับ NIC เหล่านี้:

- ไม่ต้องกำหนดค่าไดรเวอร์ UIO
- NIC สามารถยังคงผูกกับไดรเวอร์เคอร์เนลได้
- แอปพลิเคชัน DPDK สามารถเข้าถึงฮาร์ดแวร์ได้โดยตรง

### กำหนดค่าไดรเวอร์ UIO

เพื่อกำหนดค่าไดรเวอร์ UIO คุณต้อง:

1. **โหลดโมดูลไดรเวอร์**:
```bash
# สำหรับ vfio-pci (ต้องเปิดใช้งาน IOMMU ใน BIOS/UEFI และเคอร์เนล)
sudo modprobe vfio-pci

# สำหรับ uio_pci_generic
sudo modprobe uio_pci_generic

# สำหรับ igb_uio (หลังการติดตั้ง)
sudo modprobe igb_uio
```

2. **ทำให้ไดรเวอร์โหลดเมื่อบูต** โดยสร้างไฟล์กำหนดค่าใน `/etc/modules-load.d/`:
```bash
# ตัวอย่างสำหรับ vfio-pci
echo "vfio-pci" | sudo tee /etc/modules-load.d/vfio-pci.conf

# ตัวอย่างสำหรับ uio_pci_generic
echo "uio_pci_generic" | sudo tee /etc/modules-load.d/uio_pci_generic.conf

# ตัวอย่างสำหรับ igb_uio
echo "igb_uio" | sudo tee /etc/modules-load.d/igb_uio.conf
```

3. **ผูก NIC กับไดรเวอร์ UIO** โดยใช้สคริปต์ `dpdk-devbind.py` ของ DPDK:
```bash
# ยกเลิกการผูกจากไดรเวอร์เคอร์เนลและผูกกับไดรเวอร์ UIO
sudo dpdk-devbind.py --bind=vfio-pci 0000:01:00.0
sudo dpdk-devbind.py --bind=uio_pci_generic 0000:01:00.1
sudo dpdk-devbind.py --bind=igb_uio 0000:01:00.2
```

**หมายเหตุ**: แทนที่ `0000:01:00.0` ด้วยที่อยู่อุปกรณ์ PCI จริงของคุณ ใช้ `lspci` หรือ `dpdk-devbind.py --status` เพื่อระบุ NIC ของคุณ

**ทางเลือก**: แทนที่จะใช้ `dpdk-devbind.py` ด้วยตนเอง คุณสามารถกำหนดค่าการผูกอุปกรณ์ในไฟล์ `sdplane.conf` ของคุณโดยใช้คำสั่ง `set device XX:XX.X driver ...` ที่จุดเริ่มต้นของไฟล์กำหนดค่า สิ่งนี้ช่วยให้ sdplane จัดการการผูกอุปกรณ์โดยอัตโนมัติเมื่อเริ่มต้น