# แอปพลิเคชัน L2 Repeater

**ภาษา / Language:** [English](../l2-repeater-application.md) | [日本語](../ja/l2-repeater-application.md) | [Français](../fr/l2-repeater-application.md) | [中文](../zh/l2-repeater-application.md) | [Deutsch](../de/l2-repeater-application.md) | [Italiano](../it/l2-repeater-application.md) | [한국어](../ko/l2-repeater-application.md) | **ไทย**

แอปพลิเคชัน L2 Repeater ให้ฟังก์ชันการส่งต่อแพ็กเก็ต Layer 2 อย่างง่ายระหว่างพอร์ต DPDK พร้อมฟังก์ชันการ repeat ระหว่างพอร์ตพื้นฐาน

## ภาพรวม

L2 Repeater เป็นแอปพลิเคชันการส่งต่อ Layer 2 ที่ง่ายต่อการใช้งานและมีฟังก์ชันต่อไปนี้:
- การส่งต่อแพ็กเก็ตระหว่างพอร์ต DPDK ที่จับคู่กัน (การ repeat ระหว่างพอร์ตแบบง่าย)
- ฟังก์ชันการ repeat แพ็กเก็ตพื้นฐานที่ไม่มีการเรียนรู้ MAC address
- ฟังก์ชันการอัปเดต MAC address แบบเลือกได้ (การเปลี่ยน source MAC address)
- การทำงานประสิทธิภาพสูงด้วยการประมวลผลแพ็กเก็ต zero-copy ของ DPDK

## ฟีเจอร์หลัก

### การส่งต่อ Layer 2
- **การ Repeat ระหว่างพอร์ต**: การส่งต่อแพ็กเก็ตอย่างง่ายระหว่างคู่พอร์ตที่ตั้งค่าไว้ล่วงหน้า
- **ไม่มีการเรียนรู้ MAC**: การ repeat แพ็กเก็ตโดยตรงโดยไม่สร้างตารางการส่งต่อ
- **การส่งต่อแบบโปร่งใส**: ส่งต่อแพ็กเก็ตทั้งหมดโดยไม่เกี่ยวกับ destination MAC
- **การจับคู่พอร์ต**: การตั้งค่าการส่งต่อแบบพอร์ตต่อพอร์ตคงที่

### ลักษณะประสิทธิภาพ
- **การประมวลผล Zero-copy**: การใช้การประมวลผลแพ็กเก็ตที่มีประสิทธิภาพของ DPDK
- **การประมวลผล Burst**: การประมวลผลแพ็กเก็ตแบบ burst เพื่อ throughput ที่เหมาะสม
- **ความล่าช้าต่ำ**: โอเวอร์เฮดการประมวลผลขั้นต่ำเพื่อการส่งต่อที่รวดเร็ว
- **รองรับ Multi-core**: การทำงานบน lcore เฉพาะสำหรับการ scaling

## การตั้งค่า

### การตั้งค่าพื้นฐาน
L2 Repeater ถูกตั้งค่าผ่านระบบการตั้งค่าหลักของ sdplane:

```bash
# ตั้งค่าประเภท worker เป็น L2 repeater
set worker lcore 1 l2-repeater

# ตั้งค่าพอร์ตและคิว
set thread 1 port 0 queue 0  
set thread 1 port 1 queue 0

# เปิดใช้งาน promiscuous mode สำหรับการเรียนรู้
set port all promiscuous enable
```

### ตัวอย่างไฟล์การตั้งค่า
สำหรับตัวอย่างการตั้งค่าที่สมบูรณ์ ดู [`example-config/sdplane_l2_repeater.conf`](../../example-config/sdplane_l2_repeater.conf):

```bash
# การ bind อุปกรณ์
set device 02:00.0 driver vfio-pci bind
set device 03:00.0 driver vfio-pci bind

# การเริ่มต้น DPDK
set rte_eal argv -c 0x7
rte_eal_init

# Background worker (ต้องเริ่มก่อนการตั้งค่าคิว)
set worker lthread stat-collector
set worker lthread rib-manager
set worker lthread netlink-thread

set mempool

# การตั้งค่าคิว (ต้องทำหลังจากเริ่ม rib-manager)
set thread 1 port 0 queue 0
set thread 1 port 1 queue 0

# การตั้งค่าพอร์ต
stop port all
set port all dev-configure 1 4
set port all nrxdesc 1024
set port all ntxdesc 1024

# การตั้งค่า Worker
set worker lcore 1 l2-repeater
set worker lcore 2 tap-handler
set port all promiscuous enable
start port all

# รอให้พอร์ตเริ่มต้น
sleep 5

# เริ่ม Worker
start worker lcore all
```

## การทำงาน

### การส่งต่อแบบ Broadcast
L2 Repeater ส่งต่อแพ็กเก็ตไปยังพอร์ตที่ใช้งานอยู่ทั้งหมด:
- **การทำงานแบบ Broadcast**: ส่งต่อแพ็กเก็ตทั้งหมดที่รับจากพอร์ตไปยังพอร์ตที่ใช้งานอยู่ทั้งหมด
- **Split Horizon**: ไม่ส่งแพ็กเก็ตกลับไปยังพอร์ตที่รับมา
- **ไม่มีการกรอง**: ส่งต่อแพ็กเก็ตทุกประเภท (unicast, broadcast, multicast)

### พฤติกรรมการส่งต่อ
- **การส่งต่อทราฟฟิกทั้งหมด**: repeat แพ็กเก็ตทั้งหมดโดยไม่เกี่ยวกับ destination MAC
- **Broadcast ทุกพอร์ต**: ส่งแพ็กเก็ตไปยังพอร์ตที่ใช้งานอยู่ทั้งหมดยกเว้นพอร์ตที่รับเข้ามา
- **โปร่งใส**: ไม่เปลี่ยนแปลงเนื้อหาแพ็กเก็ต (เว้นแต่เปิดใช้งานการอัปเดต MAC)
- **การจำลอง Multi-port**: สร้างสำเนาแพ็กเก็ตสำหรับแต่ละพอร์ตปลายทาง

### การอัปเดต MAC Address
เมื่อเปิดใช้งาน L2 Repeater สามารถเปลี่ยน MAC address ของแพ็กเก็ต:
- **การอัปเดต Source MAC**: เปลี่ยน source MAC เป็น MAC ของพอร์ตเอาต์พุต
- **Transparent Bridging**: รักษา MAC address เดิม (ค่าเริ่มต้น)

## การปรับแต่งประสิทธิภาพ

### การตั้งค่าบัฟเฟอร์
```bash
# ปรับจำนวน descriptor ให้เหมาะกับ workload
set port all nrxdesc 2048  # เพิ่มสำหรับ packet rate สูง
set port all ntxdesc 2048  # เพิ่มสำหรับ buffering
```

### การกำหนด Worker
```bash
# กำหนด lcore เฉพาะสำหรับการส่งต่อ L2
set worker lcore 1 l2-repeater  # กำหนดให้กับ core เฉพาะ
set worker lcore 2 tap-handler  # แยกการประมวลผล TAP
```

### การกำหนดขนาด Memory Pool
Memory pool ต้องได้รับการกำหนดขนาดที่เหมาะสมสำหรับทราฟฟิกที่คาดหวัง:
- พิจารณา packet rate และข้อกำหนด buffer
- คำนึงถึง burst size และการจัดเก็บแพ็กเก็ตชั่วคราว

## การตรวจสอบและการดีบัก

### สถิติพอร์ต
```bash
# แสดงสถิติการส่งต่อ
show port statistics all

# ตรวจสอบพอร์ตเฉพาะ
show port statistics 0
show port statistics 1
```

### คำสั่งดีบัก
```bash
# เปิดใช้งานการดีบัก L2 repeater
debug sdplane l2-repeater

# การดีบัก VLAN switch (ประเภท worker ทางเลือก)
debug sdplane vlan-switch

# การดีบัก sdplane ทั่วไป
debug sdplane fdb-change
debug sdplane rib
```

## กรณีการใช้งาน

### Hub-type Repeater
- จำลองทราฟฟิกไปยังพอร์ตที่เชื่อมต่อทั้งหมด
- การ repeat Layer 2 แบบโปร่งใส
- ฟังก์ชัน hub พื้นฐานโดยไม่มีการเรียนรู้

### Port Mirroring/Repeating
- การ mirror ทราฟฟิกระหว่างพอร์ต
- การตรวจสอบและวิเคราะห์เครือข่าย
- การจำลองแพ็กเก็ตอย่างง่าย

### การทดสอบประสิทธิภาพ
- การวัดประสิทธิภาพการส่งต่อ
- baseline สำหรับ benchmark การส่งต่อ L2
- การตรวจสอบการตั้งค่าพอร์ต DPDK

## ข้อจำกัด

- **ไม่มีการประมวลผล VLAN**: การ repeat L2 อย่างง่ายโดยไม่รู้จัก VLAN
- **ไม่มีการเรียนรู้ MAC**: การส่งต่อ broadcast ทุกพอร์ตโดยไม่มีการเรียนรู้ address
- **ไม่รองรับ STP**: ไม่มีการใช้งาน Spanning Tree Protocol
- **ไม่มีการกรอง**: ส่งต่อแพ็กเก็ตทั้งหมดโดยไม่เกี่ยวกับปลายทาง

## แอปพลิเคชันที่เกี่ยวข้อง

- **Enhanced Repeater**: เวอร์ชันขั้นสูงพร้อมรองรับ VLAN และ TAP interface
- **L3 Forwarding**: ฟังก์ชัน Layer 3 routing
- **VLAN Switch**: ฟังก์ชัน switching ที่รู้จัก VLAN

สำหรับฟีเจอร์ Layer 2 ขั้นสูงรวมถึงการรองรับ VLAN ดูเอกสาร [Enhanced Repeater](enhanced-repeater.md)