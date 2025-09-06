# การสร้างแพ็กเก็ต

**ภาษา / Language:** [English](../packet-generation.md) | [日本語](../ja/packet-generation.md) | [Français](../fr/packet-generation.md) | [中文](../zh/packet-generation.md) | [Deutsch](../de/packet-generation.md) | [Italiano](../it/packet-generation.md) | [한국어](../ko/packet-generation.md) | **ไทย**

คำสั่งฟังก์ชันการสร้างแพ็กเก็ตโดยใช้ PKTGEN (Packet Generator)

## รายการคำสั่ง

### show_pktgen - การแสดงข้อมูล PKTGEN
```
show pktgen
```

แสดงสถานะและการตั้งค่าปัจจุบันของ PKTGEN (Packet Generator)

**ตัวอย่างการใช้งาน:**
```bash
show pktgen
```

คำสั่งนี้จะแสดงข้อมูลต่อไปนี้:
- สถานะการเริ่มต้นของ PKTGEN
- พารามิเตอร์การตั้งค่าปัจจุบัน
- งานที่กำลังทำงาน
- ข้อมูลสถิติ

### pktgen_init - การเริ่มต้น PKTGEN
```
pktgen init argv-list <0-7>
```

เริ่มต้น PKTGEN โดยใช้ argv-list ที่ระบุ

**พารามิเตอร์:**
- `<0-7>` - ดัชนีของ argv-list ที่จะใช้

**ตัวอย่างการใช้งาน:**
```bash
# เริ่มต้น PKTGEN โดยใช้ argv-list 0
pktgen init argv-list 0

# เริ่มต้น PKTGEN โดยใช้ argv-list 2
pktgen init argv-list 2
```

### pktgen_do_start - การเริ่มต้น PKTGEN
```
pktgen do start (<0-7>|all)
```

เริ่มต้นการสร้างแพ็กเก็ตที่พอร์ตที่ระบุ

**เป้าหมาย:**
- `<0-7>` - หมายเลขพอร์ตที่ระบุ
- `all` - ทุกพอร์ต

**ตัวอย่างการใช้งาน:**
```bash
# เริ่มต้นการสร้างแพ็กเก็ตที่พอร์ต 0
pktgen do start 0

# เริ่มต้นการสร้างแพ็กเก็ตที่ทุกพอร์ต
pktgen do start all
```

### pktgen_do_stop - การหยุด PKTGEN
```
pktgen do stop (<0-7>|all)
```

หยุดการสร้างแพ็กเก็ตที่พอร์ตที่ระบุ

**เป้าหมาย:**
- `<0-7>` - หมายเลขพอร์ตที่ระบุ
- `all` - ทุกพอร์ต

**ตัวอย่างการใช้งาน:**
```bash
# หยุดการสร้างแพ็กเก็ตที่พอร์ต 1
pktgen do stop 1

# หยุดการสร้างแพ็กเก็ตที่ทุกพอร์ต
pktgen do stop all
```

## ภาพรวมของ PKTGEN

### PKTGEN คืออะไร
PKTGEN (Packet Generator) เป็นเครื่องมือสร้างแพ็กเก็ตสำหรับการทดสอบเครือข่าย ให้ฟังก์ชันต่อไปนี้:

- **การสร้างแพ็กเก็ตความเร็วสูง** - การสร้างแพ็กเก็ตประสิทธิภาพสูง
- **รูปแบบแพ็กเก็ตที่หลากหลาย** - รองรับโปรโตคอลต่างๆ
- **การตั้งค่าที่ยืดหยุ่น** - สามารถตั้งค่าแพ็กเก็ตได้อย่างละเอียด
- **ฟังก์ชันสถิติ** - ให้ข้อมูลสถิติที่ละเอียด

### การใช้งานหลัก
- **การทดสอบประสิทธิภาพเครือข่าย** - การวัด throughput และ latency
- **การทดสอบภาระ** - การทดสอบความทนทานต่อภาระของระบบ
- **การทดสอบฟังก์ชัน** - การตรวจสอบฟังก์ชันเครือข่าย
- **เบนช์มาร์ก** - การทดสอบเปรียบเทียบประสิทธิภาพ

## การตั้งค่า PKTGEN

### ขั้นตอนการตั้งค่าพื้นฐาน
1. **การตั้งค่า argv-list**
```bash
# ตั้งค่าพารามิเตอร์สำหรับ PKTGEN
set argv-list 0 "-c 0x3 -n 4"
set argv-list 1 "--socket-mem 1024"
set argv-list 2 "--huge-dir /mnt/huge"
```

2. **การเริ่มต้น PKTGEN**
```bash
pktgen init argv-list 0
```

3. **การตั้งค่า Worker**
```bash
set worker lcore 1 pktgen
start worker lcore 1
```

4. **การเริ่มต้นการสร้างแพ็กเก็ต**
```bash
pktgen do start 0
```

### พารามิเตอร์การตั้งค่า
ตัวอย่างพารามิเตอร์ที่สามารถตั้งค่าใน argv-list:

- **-c** - CPU mask
- **-n** - จำนวน memory channel
- **--socket-mem** - ขนาด socket memory
- **--huge-dir** - ไดเรกทอรี hugepage
- **--file-prefix** - file prefix

## ตัวอย่างการใช้งาน

### การสร้างแพ็กเก็ตพื้นฐาน
```bash
# การตั้งค่า
set argv-list 0 "-c 0x3 -n 4 --socket-mem 1024"

# การเริ่มต้น
pktgen init argv-list 0

# การตั้งค่า Worker
set worker lcore 1 pktgen
start worker lcore 1

# เริ่มต้นการสร้างแพ็กเก็ต
pktgen do start 0

# การตรวจสอบสถานะ
show pktgen

# หยุดการสร้างแพ็กเก็ต
pktgen do stop 0
```

### การสร้างที่หลายพอร์ต
```bash
# เริ่มต้นที่หลายพอร์ต
pktgen do start all

# การตรวจสอบสถานะ
show pktgen

# หยุดที่หลายพอร์ต
pktgen do stop all
```

## การตรวจสอบและสถิติ

### การตรวจสอบข้อมูลสถิติ
```bash
# แสดงสถิติ PKTGEN
show pktgen

# แสดงสถิติพอร์ต
show port statistics

# แสดงสถิติ Worker
show worker
```

### การตรวจสอบประสิทธิภาพ
```bash
# การตรวจสอบ PPS (Packets Per Second)
show port statistics pps

# การตรวจสอบจำนวนแพ็กเก็ตรวม
show port statistics total

# การตรวจสอบไบต์/วินาที
show port statistics Bps
```

## การแก้ไขปัญหา

### เมื่อ PKTGEN ไม่เริ่มต้น
1. ตรวจสอบสถานะการเริ่มต้น
```bash
show pktgen
```

2. ตรวจสอบสถานะ Worker
```bash
show worker
```

3. ตรวจสอบสถานะพอร์ต
```bash
show port
```

### เมื่อการสร้างแพ็กเก็ตไม่หยุด
1. หยุดอย่างชัดเจน
```bash
pktgen do stop all
```

2. รีสตาร์ท Worker
```bash
restart worker lcore 1
```

### เมื่อประสิทธิภาพต่ำ
1. ตรวจสอบการใช้งาน CPU
2. ตรวจสอบการตั้งค่าหน่วยความจำ
3. ตรวจสอบการตั้งค่าพอร์ต

## ฟังก์ชันขั้นสูง

### การตั้งค่ารูปแบบแพ็กเก็ต
ใน PKTGEN สามารถสร้างรูปแบบแพ็กเก็ตต่างๆ ได้:
- **Ethernet** - เฟรม Ethernet พื้นฐาน
- **IP** - แพ็กเก็ต IPv4/IPv6
- **UDP/TCP** - แพ็กเก็ต UDP/TCP
- **VLAN** - แพ็กเก็ตที่มี VLAN tag

### การควบคุมภาระ
- **การควบคุมอัตรา** - การควบคุมอัตราการสร้างแพ็กเก็ต
- **การควบคุม Burst** - การสร้างแพ็กเก็ต burst
- **การควบคุมขนาด** - การควบคุมขนาดแพ็กเก็ต

## ตำแหน่งการกำหนด

คำสั่งเหล่านี้ถูกกำหนดในไฟล์ต่อไปนี้:
- `sdplane/pktgen_cmd.c`

## รายการที่เกี่ยวข้อง

- [การจัดการ Worker และ lcore](worker-management.md)
- [การจัดการพอร์ตและสถิติ](port-management.md)
- [ข้อมูลระบบและการตรวจสอบ](system-monitoring.md)