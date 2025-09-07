# แอปพลิเคชัน Enhanced Repeater

**Language:** [English](../en/enhanced-repeater-application.md) | [日本語](../ja/enhanced-repeater-application.md) | [Français](../fr/enhanced-repeater-application.md) | [中文](../zh/enhanced-repeater-application.md) | [Deutsch](../de/enhanced-repeater-application.md) | [Italiano](../it/enhanced-repeater-application.md) | [한국어](../ko/enhanced-repeater-application.md) | **ไทย**

แอปพลิเคชัน Enhanced Repeater ให้การ switching Layer 2 ขั้นสูงที่รู้จัก VLAN พร้อมการบูรณาการ TAP interface สำหรับการ routing L3 และฟังก์ชันการจับแพ็กเก็ต

## ภาพรวม

Enhanced Repeater เป็นแอปพลิเคชันการส่งต่อ Layer 2 ที่ซับซ้อนซึ่งขยายการส่งต่อ L2 พื้นฐานด้วยฟีเจอร์ต่อไปนี้:
- **การ Switching ที่รู้จัก VLAN**: การรองรับ IEEE 802.1Q VLAN อย่างสมบูรณ์ (tagging/untagging)
- **การแยก Virtual Switch**: virtual switch หลายตัวที่มี forwarding table แยกต่างหาก
- **การบูรณาการ TAP Interface**: router interface และ capture interface สำหรับการบูรณาการ kernel
- **การประมวลผล VLAN ขั้นสูง**: ฟังก์ชันการแปลง แทรก และลบ VLAN

## สถาปัตยกรรม

### เฟรมเวิร์ก Virtual Switch
Enhanced Repeater ใช้สถาปัตยกรรม virtual switch:
- **VLAN หลายตัว**: รองรับหลาย VLAN domain (1-4094)
- **การรวม Port**: หลาย physical port ต่อ virtual switch
- **การส่งต่อที่แยก**: forwarding domain ที่เป็นอิสระสำหรับแต่ละ VLAN
- **การ Tagging ที่ยืดหยุ่น**: โมด native, tagged, และ translation สำหรับแต่ละพอร์ต

### การบูรณาการ TAP Interface
- **Router Interface**: การบูรณาการ kernel network stack สำหรับการประมวลผล L3
- **Capture Interface**: ฟังก์ชันการตรวจสอบและวิเคราะห์แพ็กเก็ต
- **Ring Buffer**: การส่งถ่ายแพ็กเก็ตที่มีประสิทธิภาพระหว่าง data plane และ kernel
- **แบบสองทิศทาง**: การประมวลผลแพ็กเก็ตทั้งทิศทางเข้าและออก

## ฟีเจอร์หลัก

### การประมวลผล VLAN
- **VLAN Tagging**: การเพิ่ม 802.1Q header ลงในเฟรม untagged
- **VLAN Untagging**: การลบ 802.1Q header จากเฟรม tagged
- **การแปลง VLAN**: การเปลี่ยน VLAN ID ระหว่างอินพุตและเอาต์พุต
- **Native VLAN**: การจัดการทราฟฟิก untagged บนพอร์ต trunk

### Virtual Switching
- **การเรียนรู้**: การเรียนรู้ MAC address อัตโนมัติสำหรับแต่ละ VLAN
- **การ Flooding**: การจัดการ unknown unicast และ broadcast ที่เหมาะสม
- **Split Horizon**: การป้องกันลูปภายใน virtual switch
- **Multi-domain**: forwarding table ที่เป็นอิสระสำหรับแต่ละ VLAN

### การประมวลผลแพ็กเก็ต
- **Zero-copy**: การประมวลผลแพ็กเก็ต DPDK ที่มีประสิทธิภาพด้วยโอเวอร์เฮดขั้นต่ำ
- **การประมวลผล Burst**: ปรับแต่งสำหรับ packet rate สูง
- **การจัดการ Header**: การแทรก/ลบ VLAN header ที่มีประสิทธิภาพ
- **การปรับแต่งการคัดลอก**: การคัดลอกแพ็กเก็ตแบบเลือกสำหรับ TAP interface

## การตั้งค่า

### การตั้งค่า Virtual Switch
สร้าง virtual switch สำหรับ VLAN domain ต่างๆ:

```bash
# สร้าง virtual switch ด้วย VLAN ID
set vswitch 2031
set vswitch 2032
```

### การลิงก์พอร์ตไปยัง Virtual Switch
ลิงก์พอร์ต DPDK ทางกายภาพไปยัง virtual switch:

```bash
# ลิงก์พอร์ต 0 ไปยัง virtual switch 0 ด้วย VLAN tag 2031
set vswitch-link vswitch 0 port 0 tag 2031

# ลิงก์พอร์ต 0 ไปยัง virtual switch 1 แบบ native/untagged
set vswitch-link vswitch 1 port 0 tag 0

# ตัวอย่างการแปลง VLAN
set vswitch-link vswitch 0 port 1 tag 2040
```

### Router Interface
สร้าง router interface สำหรับการเชื่อมต่อ L3:

```bash
# สร้าง router interface สำหรับการประมวลผล L3
set vswitch 2031 router-if rif2031
set vswitch 2032 router-if rif2032
```

### Capture Interface
ตั้งค่า capture interface สำหรับการตรวจสอบ:

```bash
# สร้าง capture interface สำหรับการตรวจสอบแพ็กเก็ต
set vswitch 2031 capture-if cif2031
set vswitch 2032 capture-if cif2032
```

### การตั้งค่า Worker
กำหนด enhanced repeater ไปยัง lcore:

```bash
# ตั้งค่า enhanced repeater worker
set worker lcore 1 enhanced-repeater

# ตั้งค่าการจัดสรรคิว
set thread 1 port 0 queue 0
set thread 1 port 1 queue 0

# เปิดใช้งาน promiscuous mode
set port all promiscuous enable
```

## การทำงาน

### การประมวลผล VLAN
Enhanced Repeater ทำการประมวลผล VLAN ที่ซับซ้อน:
- **การแปลง Tag**: เปลี่ยน VLAN ID ตามการตั้งค่า vswitch-link
- **การลบ Tag**: ลบ VLAN header สำหรับทราฟฟิกที่ไปยังพอร์ต native
- **การเพิ่ม Tag**: เพิ่ม VLAN header สำหรับทราฟฟิก native ที่ไปยังพอร์ต tagged
- **การเก็บรักษา**: ส่งผ่าน VLAN tag ที่มีอยู่สำหรับทราฟฟิก tagged-to-tagged

### การเรียนรู้และการส่งต่อ
- **การเรียนรู้ MAC**: เรียนรู้ MAC address แยกสำหรับแต่ละ VLAN domain
- **การส่งต่อ Unicast**: ส่งต่อตาม MAC table ที่เรียนรู้
- **การ Flooding**: flood unknown unicast และ broadcast ภายใน VLAN
- **การแยก VLAN**: รักษาการแยก VLAN ที่เข้มงวด

## การตรวจสอบและการดีบัก

### สถิติ Virtual Switch
```bash
# แสดงข้อมูล virtual switch
show vswitch_rib

# แสดงการตั้งค่า vswitch link
show vswitch-link

# แสดง router interface
show rib vswitch router-if

# แสดง capture interface  
show rib vswitch capture-if
```

### การดีบัก
```bash
# เปิดใช้งานการดีบัก enhanced repeater
debug sdplane enhanced-repeater

# การดีบัก VLAN processing
debug sdplane vlan-processing

# การดีบัก FDB (Forwarding Database)
debug sdplane fdb-change
```

## กรณีการใช้งาน

### การ Switching VLAN ขั้นสูง
- การแยก VLAN ที่สมบูรณ์
- การแปลง VLAN ID
- การจัดการพอร์ต trunk และ access

### การรวมเข้ากับ Kernel
- การ routing L3 ผ่าน router interface
- การตรวจสอบแพ็กเก็ตผ่าน capture interface
- การบูรณาการกับ Linux network stack

### การตรวจสอบเครือข่าย
- การจับแพ็กเก็ตแบบ selective ต่อ VLAN
- การวิเคราะห์ทราฟฟิกในระดับ VLAN
- การ troubleshooting เครือข่าย

## ข้อจำกัด

- **จำนวน Virtual Switch**: สูงสุด 4 virtual switch พร้อมกัน
- **จำนวน VLAN Link**: สูงสุด 8 vswitch link
- **ความซับซ้อนของการตั้งค่า**: ต้องการการตั้งค่าที่ระมัดระวังสำหรับ VLAN topology ที่ซับซ้อน
- **ผลกระทบต่อประสิทธิภาพ**: การประมวลผล VLAN อาจส่งผลกระทบต่อประสิทธิภาพเมื่อเทียบกับ L2 repeater พื้นฐาน

## แอปพลิเคชันที่เกี่ยวข้อง

- **L2 Repeater**: เวอร์ชันพื้นฐานสำหรับการส่งต่อ L2 อย่างง่าย
- **L3 Forwarding**: การส่งต่อ Layer 3 ขั้นสูง
- **VLAN Switch**: การ switching ที่เน้น VLAN โดยเฉพาะ

สำหรับการตั้งค่าที่ละเอียด ดูเอกสาร [Enhanced Repeater Configuration](enhanced-repeater.md)