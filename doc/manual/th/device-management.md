# การจัดการอุปกรณ์

**Language:** [English](../en/device-management.md) | [日本語](../ja/device-management.md) | [Français](../fr/device-management.md) | [中文](../zh/device-management.md) | [Deutsch](../de/device-management.md) | [Italiano](../it/device-management.md) | [한국어](../ko/device-management.md) | **ไทย**

คำสั่งที่จัดการอุปกรณ์ DPDK และไดรเวอร์

## รายการคำสั่ง

### show_devices - การแสดงข้อมูลอุปกรณ์
```
show devices
```

แสดงข้อมูลของอุปกรณ์ที่ใช้ได้ในระบบ

**ตัวอย่างการใช้งาน:**
```bash
show devices
```

คำสั่งนี้จะแสดงข้อมูลต่อไปนี้:
- ชื่ออุปกรณ์
- ประเภทอุปกรณ์
- ไดรเวอร์ปัจจุบัน
- สถานะอุปกรณ์
- PCI address

### set_device_driver - การตั้งค่าไดรเวอร์อุปกรณ์
```
set device <WORD> driver (ixgbe|igb|igc|uio_pci_generic|igb_uio|vfio-pci|unbound) (|bind|driver_override)
```

ตั้งค่าไดรเวอร์ให้กับอุปกรณ์ที่ระบุ

**พารามิเตอร์:**
- `<WORD>` - ชื่ออุปกรณ์หรือ PCI address
- ประเภทไดรเวอร์:
  - `ixgbe` - ไดรเวอร์ Intel 10GbE ixgbe
  - `igb` - ไดรเวอร์ Intel 1GbE igb
  - `igc` - ไดรเวอร์ Intel 2.5GbE igc
  - `uio_pci_generic` - ไดรเวอร์ UIO ทั่วไป
  - `igb_uio` - ไดรเวอร์ DPDK UIO
  - `vfio-pci` - ไดรเวอร์ VFIO PCI
  - `unbound` - ลบไดรเวอร์
- โมดการทำงาน:
  - `bind` - bind ไดรเวอร์
  - `driver_override` - override ไดรเวอร์

**ตัวอย่างการใช้งาน:**
```bash
# bind อุปกรณ์กับไดรเวอร์ vfio-pci
set device 0000:01:00.0 driver vfio-pci bind

# ตั้งค่าอุปกรณ์ให้ใช้ไดรเวอร์ igb_uio
set device eth0 driver igb_uio

# ลบไดรเวอร์จากอุปกรณ์
set device 0000:01:00.0 driver unbound
```

## ภาพรวมของการจัดการอุปกรณ์

### การจัดการอุปกรณ์ DPDK
ใน DPDK จะใช้ไดรเวอร์เฉพาะเพื่อใช้งานอุปกรณ์เครือข่ายอย่างมีประสิทธิภาพ

### ประเภทของไดรเวอร์

#### Network Driver
- **ixgbe** - สำหรับ Intel 10GbE network card
- **igb** - สำหรับ Intel 1GbE network card
- **igc** - สำหรับ Intel 2.5GbE network card

#### UIO (Userspace I/O) Driver
- **uio_pci_generic** - ไดรเวอร์ UIO ทั่วไป
- **igb_uio** - ไดรเวอร์ UIO เฉพาะ DPDK

#### VFIO (Virtual Function I/O) Driver
- **vfio-pci** - I/O ประสิทธิภาพสูงในสภาพแวดล้อมเสมือน

#### การตั้งค่าพิเศษ
- **unbound** - ลบไดรเวอร์และปิดใช้งานอุปกรณ์

## ขั้นตอนการตั้งค่าอุปกรณ์

### ขั้นตอนการตั้งค่าพื้นฐาน
1. **ตรวจสอบอุปกรณ์**
```bash
show devices
```

2. **ตั้งค่าไดรเวอร์**
```bash
set device <device> driver <driver> bind
```

3. **ตรวจสอบการตั้งค่า**
```bash
show devices
```

4. **ตั้งค่าพอร์ต**
```bash
show port
```

### ตัวอย่างการตั้งค่า

#### การตั้งค่า Intel 10GbE Card
```bash
# ตรวจสอบอุปกรณ์
show devices

# bind ไดรเวอร์ ixgbe
set device 0000:01:00.0 driver ixgbe bind
set device 0000:01:00.1 driver ixgbe bind

# ตรวจสอบการตั้งค่า
show devices
show port
```

#### การตั้งค่า DPDK UIO Driver
```bash
# ตรวจสอบอุปกรณ์
show devices

# bind ไดรเวอร์ igb_uio
set device 0000:02:00.0 driver igb_uio bind
set device 0000:02:00.1 driver igb_uio bind

# ตรวจสอบการตั้งค่า
show devices
show port
```

#### การตั้งค่า VFIO (สภาพแวดล้อมเสมือน)
```bash
# ตรวจสอบอุปกรณ์
show devices

# bind ไดรเวอร์ vfio-pci
set device 0000:03:00.0 driver vfio-pci bind
set device 0000:03:00.1 driver vfio-pci bind

# ตรวจสอบการตั้งค่า
show devices
show port
```

## หลักเกณฑ์การเลือกไดรเวอร์

### ixgbe (Intel 10GbE)
- **การใช้งาน**: Intel 10GbE network card
- **ข้อดี**: ประสิทธิภาพสูง เสถียร
- **เงื่อนไข**: ต้องมี Intel 10GbE card

### igb (Intel 1GbE)
- **การใช้งาน**: Intel 1GbE network card
- **ข้อดี**: ความเข้ากันได้กว้าง เสถียร
- **เงื่อนไข**: ต้องมี Intel 1GbE card

### igc (Intel 2.5GbE)
- **การใช้งาน**: Intel 2.5GbE network card
- **ข้อดี**: ประสิทธิภาพระดับกลาง มาตรฐานใหม่
- **เงื่อนไข**: ต้องมี Intel 2.5GbE card

### uio_pci_generic
- **การใช้งาน**: อุปกรณ์ทั่วไป
- **ข้อดี**: ความเข้ากันได้กว้าง
- **ข้อเสี**: จำกัดฟังก์ชันบางอย่าง

### igb_uio
- **การใช้งาน**: สภาพแวดล้อมเฉพาะ DPDK
- **ข้อดี**: ปรับแต่งสำหรับ DPDK
- **ข้อเสี**: ต้องติดตั้งแยกต่างหาก

### vfio-pci
- **การใช้งาน**: สภาพแวดล้อมเสมือน เน้นความปลอดภัย
- **ข้อดี**: ความปลอดภัย รองรับการเสมือน
- **เงื่อนไข**: ต้องเปิดใช้งาน IOMMU

## การแก้ไขปัญหา

### เมื่ออุปกรณ์ไม่ถูกจดจำ
1. ตรวจสอบอุปกรณ์
```bash
show devices
```

2. ตรวจสอบในระดับระบบ
```bash
lspci | grep Ethernet
```

3. ตรวจสอบ kernel module
```bash
lsmod | grep uio
lsmod | grep vfio
```

### เมื่อ bind ไดรเวอร์ล้มเหลว
1. ตรวจสอบไดรเวอร์ปัจจุบัน
```bash
show devices
```

2. ลบไดรเวอร์ที่มีอยู่
```bash
set device <device> driver unbound
```

3. bind ไดรเวอร์เป้าหมาย
```bash
set device <device> driver <target_driver> bind
```

### เมื่อไม่สามารถใช้พอร์ตได้
1. ตรวจสอบสถานะอุปกรณ์
```bash
show devices
show port
```

2. bind ไดรเวอร์ใหม่
```bash
set device <device> driver unbound
set device <device> driver <driver> bind
```

3. ตรวจสอบการตั้งค่าพอร์ต
```bash
show port
update port status
```

## ฟังก์ชันขั้นสูง

### Driver Override
```bash
# การใช้งาน driver override
set device <device> driver <driver> driver_override
```

### การตั้งค่าอุปกรณ์หลายตัวพร้อมกัน
```bash
# ตั้งค่าอุปกรณ์หลายตัวตามลำดับ
set device 0000:01:00.0 driver vfio-pci bind
set device 0000:01:00.1 driver vfio-pci bind
set device 0000:02:00.0 driver vfio-pci bind
set device 0000:02:00.1 driver vfio-pci bind
```

## ข้อพิจารณาด้านความปลอดภัย

### ข้อควรระวังเมื่อใช้ VFIO
- ต้องเปิดใช้งาน IOMMU
- การตั้งค่า security group
- การตั้งค่าสิทธิ์ที่เหมาะสม

### ข้อควรระวังเมื่อใช้ UIO
- ต้องมีสิทธิ์ root
- การเข้าใจความเสี่ยงด้านความปลอดภัย
- การควบคุมการเข้าถึงที่เหมาะสม

## การบูรณาการระบบ

### การทำงานร่วมกับ systemd service
```bash
# การตั้งค่าอัตโนมัติด้วย systemd service
# ตั้งค่าใน /etc/systemd/system/sdplane.service
```

### การตั้งค่าอัตโนมัติเมื่อเริ่มต้นระบบ
```bash
# การตั้งค่าใน startup script
# /etc/init.d/sdplane หรือ systemd unit file
```

## ตำแหน่งการกำหนด

คำสั่งเหล่านี้ถูกกำหนดในไฟล์ต่อไปนี้:
- `sdplane/dpdk_devbind.c`

## รายการที่เกี่ยวข้อง

- [การจัดการพอร์ตและสถิติ](port-management.md)
- [การจัดการ Worker และ lcore](worker-lcore-thread-management.md)
- [ข้อมูลระบบและการตรวจสอบ](system-monitoring.md)