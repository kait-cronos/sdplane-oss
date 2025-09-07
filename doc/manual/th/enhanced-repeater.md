# การตั้งค่า Enhanced Repeater

**Language:** [English](../en/enhanced-repeater.md) | [日本語](../ja/enhanced-repeater.md) | [Français](../fr/enhanced-repeater.md) | [中文](../zh/enhanced-repeater.md) | [Deutsch](../de/enhanced-repeater.md) | [Italiano](../it/enhanced-repeater.md) | [한국어](../ko/enhanced-repeater.md) | **ไทย**

Enhanced Repeater ให้ฟังก์ชันการ switching VLAN ขั้นสูงพร้อมด้วย TAP interface สำหรับการ routing L3 และการจับแพ็กเก็ต รองรับการสร้าง virtual switch, การลิงก์พอร์ต DPDK ด้วย VLAN tagging, router interface สำหรับการรวมเข้ากับ kernel, และ capture interface สำหรับการตรวจสอบ

## คำสั่ง Virtual Switch

### set vswitch
```
set vswitch <1-4094>
```
**คำอธิบาย**: สร้าง virtual switch ด้วย VLAN ID ที่ระบุ

**พารามิเตอร์**:
- `<1-4094>`: VLAN ID ของ virtual switch

**ตัวอย่าง**:
```
set vswitch 2031
set vswitch 2032
```

### delete vswitch
```
delete vswitch <0-3>
```
**คำอธิบาย**: ลบ virtual switch ที่ระบุด้วย ID

**พารามิเตอร์**:
- `<0-3>`: ID ของ virtual switch ที่จะลบ

**ตัวอย่าง**:
```
delete vswitch 0
```

### show vswitch_rib
```
show vswitch_rib
```
**คำอธิบาย**: แสดงข้อมูล RIB ของ virtual switch รวมถึงการตั้งค่าและสถานะ

## คำสั่ง Virtual Switch Link

### set vswitch-link
```
set vswitch-link vswitch <0-3> port <0-7> tag <0-4094>
```
**คำอธิบาย**: ลิงก์พอร์ต DPDK กับ virtual switch ด้วยการตั้งค่า VLAN tagging

**พารามิเตอร์**:
- `vswitch <0-3>`: ID ของ virtual switch (0-3)
- `port <0-7>`: ID ของพอร์ต DPDK (0-7)  
- `tag <0-4094>`: ID ของ VLAN tag (0: native/untagged, 1-4094: tagged VLAN)

**ตัวอย่าง**:
```
# ลิงก์พอร์ต 0 กับ virtual switch 0 ด้วย VLAN tag 2031
set vswitch-link vswitch 0 port 0 tag 2031

# ลิงก์พอร์ต 0 กับ virtual switch 1 แบบ native/untagged
set vswitch-link vswitch 1 port 0 tag 0
```

### delete vswitch-link
```
delete vswitch-link <0-7>
```
**คำอธิบาย**: ลบ virtual switch link ที่ระบุด้วย ID

**พารามิเตอร์**:
- `<0-7>`: ID ของ virtual switch link

### show vswitch-link
```
show vswitch-link
```
**คำอธิบาย**: แสดงการตั้งค่า virtual switch link ทั้งหมด

## คำสั่ง Router Interface

### set vswitch router-if
```
set vswitch <1-4094> router-if <WORD>
```
**คำอธิบาย**: สร้าง router interface สำหรับการเชื่อมต่อ L3 กับ virtual switch ที่ระบุ

**พารามิเตอร์**:
- `<1-4094>`: VLAN ID ของ virtual switch
- `<WORD>`: ชื่อของ TAP interface

**ตัวอย่าง**:
```
set vswitch 2031 router-if rif2031
set vswitch 2032 router-if rif2032
```

### no set vswitch router-if
```
no set vswitch <1-4094> router-if
```
**คำอธิบาย**: ลบ router interface จาก virtual switch ที่ระบุ

**พารามิเตอร์**:
- `<1-4094>`: VLAN ID ของ virtual switch

### show rib vswitch router-if
```
show rib vswitch router-if
```
**คำอธิบาย**: แสดงการตั้งค่า router interface รวมถึง MAC address, IP address, และสถานะ interface

## คำสั่ง Capture Interface

### set vswitch capture-if
```
set vswitch <1-4094> capture-if <WORD>
```
**คำอธิบาย**: สร้าง capture interface สำหรับการตรวจสอบแพ็กเก็ตใน virtual switch ที่ระบุ

**พารามิเตอร์**:
- `<1-4094>`: VLAN ID ของ virtual switch
- `<WORD>`: ชื่อของ TAP interface

**ตัวอย่าง**:
```
set vswitch 2031 capture-if cif2031
set vswitch 2032 capture-if cif2032
```

### no set vswitch capture-if
```
no set vswitch <1-4094> capture-if
```
**คำอธิบาย**: ลบ capture interface จาก virtual switch ที่ระบุ

**พารามิเตอร์**:
- `<1-4094>`: VLAN ID ของ virtual switch

### show rib vswitch capture-if
```
show rib vswitch capture-if
```
**คำอธิบาย**: แสดงการตั้งค่า capture interface

## ฟังก์ชันการประมวลผล VLAN

Enhanced Repeater ทำการประมวลผล VLAN ขั้นสูง:

- **การแปลง VLAN**: เปลี่ยน VLAN ID ตามการตั้งค่า vswitch-link
- **การลบ VLAN**: ลบ VLAN header เมื่อ tag ถูกตั้งเป็น 0 (native)  
- **การแทรก VLAN**: เพิ่ม VLAN header เมื่อส่งต่อแพ็กเก็ต untagged ไปยังพอร์ต tagged
- **Split Horizon**: ป้องกันลูปโดยไม่ส่งแพ็กเก็ตกลับไปยังพอร์ตที่รับมา

## ตัวอย่างการตั้งค่า

```bash
# สร้าง virtual switch
set vswitch 2031
set vswitch 2032

# ลิงก์พอร์ต DPDK กับ virtual switch ด้วย VLAN tag
set vswitch-link vswitch 0 port 0 tag 2031
set vswitch-link vswitch 1 port 0 tag 2032

# สร้าง router interface สำหรับการประมวลผล L3
set vswitch 2031 router-if rif2031
set vswitch 2032 router-if rif2032

# สร้าง capture interface สำหรับการตรวจสอบ
set vswitch 2031 capture-if cif2031
set vswitch 2032 capture-if cif2032

# ตั้งค่า worker ให้ใช้ enhanced repeater
set worker lcore 1 enhanced-repeater
```

## การบูรณาการกับ TAP Interface

Router interface และ capture interface สร้าง TAP interface ที่บูรณาการกับ Linux kernel network stack:

- **Router Interface**: เปิดใช้งาน L3 routing, IP addressing, และการประมวลผลเครือข่าย kernel
- **Capture Interface**: เปิดใช้งานการตรวจสอบ วิเคราะห์ และดีบักแพ็กเก็ต
- **Ring Buffer**: ใช้ DPDK ring สำหรับการส่งถ่ายแพ็กเก็ตที่มีประสิทธิภาพระหว่าง data plane และ kernel