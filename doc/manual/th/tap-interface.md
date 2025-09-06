# อินเทอร์เฟซ TAP

**ภาษา / Language:** [English](../tap-interface.md) | [日本語](../ja/tap-interface.md) | [Français](../fr/tap-interface.md) | [中文](../zh/tap-interface.md) | [Deutsch](../de/tap-interface.md) | [Italiano](../it/tap-interface.md) | [한국어](../ko/tap-interface.md) | **ไทย**

คำสั่งที่จัดการอินเทอร์เฟซ TAP

## รายการคำสั่ง

### set_tap_capture_ifname - การตั้งค่าชื่ออินเทอร์เฟซ TAP Capture
```
set tap capture ifname <WORD>
```

ตั้งค่าชื่ออินเทอร์เฟซที่จะใช้ในฟังก์ชัน TAP capture

**พารามิเตอร์:**
- `<WORD>` - ชื่ออินเทอร์เฟซ

**ตัวอย่างการใช้งาน:**
```bash
# ตั้งค่าอินเทอร์เฟซ tap0
set tap capture ifname tap0

# ตั้งค่าอินเทอร์เฟซ tap1
set tap capture ifname tap1
```

### set_tap_capture_persistent - การเปิดใช้งานการคงอยู่ของ TAP Capture
```
set tap capture persistent
```

เปิดใช้งานการคงอยู่ของ TAP capture

**ตัวอย่างการใช้งาน:**
```bash
# เปิดใช้งานการคงอยู่
set tap capture persistent
```

### no_tap_capture_persistent - การปิดใช้งานการคงอยู่ของ TAP Capture
```
no tap capture persistent
```

ปิดใช้งานการคงอยู่ของ TAP capture

**ตัวอย่างการใช้งาน:**
```bash
# ปิดใช้งานการคงอยู่
no tap capture persistent
```

### unset_tap_capture_persistent - การลบการตั้งค่าการคงอยู่ของ TAP Capture
```
unset tap capture persistent
```

ลบการตั้งค่าการคงอยู่ของ TAP capture

**ตัวอย่างการใช้งาน:**
```bash
# ลบการตั้งค่าการคงอยู่
unset tap capture persistent
```

## ภาพรวมของอินเทอร์เฟซ TAP

### อินเทอร์เฟซ TAP คืออะไร
อินเทอร์เฟซ TAP (Network TAP) เป็นอินเทอร์เฟซเครือข่ายเสมือนที่ใช้สำหรับการตรวจสอบและทดสอบการรับส่งข้อมูลเครือข่าย

### ฟังก์ชันหลัก
- **Packet Capture** - การจับแพ็กเก็ตการรับส่งข้อมูลเครือข่าย
- **Packet Injection** - การแทรกแพ็กเก็ตทดสอบ
- **ฟังก์ชัน Bridge** - การสร้าง bridge ระหว่างเครือข่ายต่างๆ
- **ฟังก์ชันตรวจสอบ** - การตรวจสอบและวิเคราะห์การรับส่งข้อมูล

### การใช้งานใน sdplane
- **การดีบัก** - การดีบัก packet flow
- **การทดสอบ** - การทดสอบฟังก์ชันเครือข่าย
- **การตรวจสอบ** - การตรวจสอบการรับส่งข้อมูล
- **การพัฒนา** - การพัฒนาและทดสอบฟีเจอร์ใหม่

## การตั้งค่าอินเทอร์เฟซ TAP

### ขั้นตอนการตั้งค่าพื้นฐาน
1. **การสร้างอินเทอร์เฟซ TAP**
```bash
# สร้างอินเทอร์เฟซ TAP ในระดับระบบ
sudo ip tuntap add tap0 mode tap
sudo ip link set tap0 up
```

2. **การตั้งค่าใน sdplane**
```bash
# ตั้งค่าชื่ออินเทอร์เฟซ TAP capture
set tap capture ifname tap0

# เปิดใช้งานการคงอยู่
set tap capture persistent
```

3. **การตั้งค่า TAP handler worker**
```bash
# ตั้งค่า TAP handler worker
set worker lcore 2 tap-handler
start worker lcore 2
```

### ตัวอย่างการตั้งค่า

#### การตั้งค่า TAP พื้นฐาน
```bash
# การตั้งค่าอินเทอร์เฟซ TAP
set tap capture ifname tap0
set tap capture persistent

# การตั้งค่า Worker
set worker lcore 2 tap-handler
start worker lcore 2

# การตรวจสอบการตั้งค่า
show worker
```

#### การตั้งค่าอินเทอร์เฟซ TAP หลายตัว
```bash
# ตั้งค่าอินเทอร์เฟซ TAP หลายตัว
set tap capture ifname tap0
set tap capture ifname tap1

# เปิดใช้งานการคงอยู่
set tap capture persistent
```

## ฟังก์ชันการคงอยู่

### การคงอยู่คืออะไร
เมื่อเปิดใช้งานฟังก์ชันการคงอยู่ การตั้งค่าอินเทอร์เฟซ TAP จะถูกเก็บไว้แม้หลังจากรีสตาร์ทระบบ

### ข้อดีของการคงอยู่
- **การเก็บการตั้งค่า** - การตั้งค่ายังคงใช้ได้หลังจากรีสตาร์ท
- **การฟื้นฟูอัตโนมัติ** - การฟื้นฟูอัตโนมัติจากความล้มเหลวของระบบ
- **ประสิทธิภาพการดำเนินงาน** - การลดการตั้งค่าด้วยมือ

### การตั้งค่าการคงอยู่
```bash
# เปิดใช้งานการคงอยู่
set tap capture persistent

# ปิดใช้งานการคงอยู่
no tap capture persistent

# ลบการตั้งค่าการคงอยู่
unset tap capture persistent
```

## ตัวอย่างการใช้งาน

### การใช้งานเพื่อการดีบัก
```bash
# การตั้งค่าอินเทอร์เฟซ TAP สำหรับการดีบัก
set tap capture ifname debug-tap
set tap capture persistent

# ตั้งค่า TAP handler worker
set worker lcore 3 tap-handler
start worker lcore 3

# เริ่มต้น packet capture
tcpdump -i debug-tap
```

### การใช้งานเพื่อการทดสอบ
```bash
# การตั้งค่าอินเทอร์เฟซ TAP สำหรับการทดสอบ
set tap capture ifname test-tap
set tap capture persistent

# การเตรียมการแทรกแพ็กเก็ตทดสอบ
set worker lcore 4 tap-handler
start worker lcore 4
```

## การตรวจสอบและการจัดการ

### การตรวจสอบสถานะอินเทอร์เฟซ TAP
```bash
# การตรวจสอบสถานะ Worker
show worker

# การตรวจสอบข้อมูลเธรด
show thread

# การตรวจสอบในระดับระบบ
ip link show tap0
```

### การตรวจสอบการรับส่งข้อมูล
```bash
# การตรวจสอบด้วย tcpdump
tcpdump -i tap0

# การตรวจสอบด้วย Wireshark
wireshark -i tap0
```

## การแก้ไขปัญหา

### เมื่ออินเทอร์เฟซ TAP ไม่ถูกสร้าง
1. การตรวจสอบในระดับระบบ
```bash
# การตรวจสอบการมีอยู่ของอินเทอร์เฟซ TAP
ip link show tap0

# การตรวจสอบสิทธิ์
sudo ip tuntap add tap0 mode tap
```

2. การตรวจสอบใน sdplane
```bash
# การตรวจสอบการตั้งค่า
show worker

# การตรวจสอบสถานะ Worker
show thread
```

### เมื่อแพ็กเก็ตไม่ถูกจับ
1. การตรวจสอบสถานะอินเทอร์เฟซ
```bash
ip link show tap0
```

2. การตรวจสอบสถานะ Worker
```bash
show worker
```

3. การรีสตาร์ท TAP handler
```bash
restart worker lcore 2
```

### เมื่อการคงอยู่ไม่ทำงาน
1. การตรวจสอบการตั้งค่าการคงอยู่
```bash
# การตรวจสอบการตั้งค่าปัจจุบัน (ตรวจสอบด้วยคำสั่ง show)
show worker
```

2. การตรวจสอบการตั้งค่าระบบ
```bash
# การตรวจสอบการตั้งค่าในระดับระบบ
systemctl status sdplane
```

## ฟังก์ชันขั้นสูง

### ความร่วมมือกับ VLAN
อินเทอร์เฟซ TAP สามารถใช้ร่วมกับฟังก์ชัน VLAN:
```bash
# ความร่วมมือกับ VLAN switch worker
set worker lcore 5 vlan-switch
start worker lcore 5
```

### ฟังก์ชัน Bridge
การใช้อินเทอร์เฟซ TAP หลายตัวร่วมกันเป็น bridge:
```bash
# การตั้งค่าอินเทอร์เฟซ TAP หลายตัว
set tap capture ifname tap0
set tap capture ifname tap1
```

## ตำแหน่งการกำหนด

คำสั่งเหล่านี้ถูกกำหนดในไฟล์ต่อไปนี้:
- `sdplane/tap_cmd.c`

## รายการที่เกี่ยวข้อง

- [การจัดการ Worker และ lcore](worker-management.md)
- [การจัดการ VTY และเชลล์](vty-shell.md)
- [การดีบักและล็อก](debug-logging.md)