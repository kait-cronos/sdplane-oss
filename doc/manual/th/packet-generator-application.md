# แอปพลิเคชัน Packet Generator

Created by Keith Wiles @ Intel 2010-2025 from https://github.com/pktgen/Pktgen-DPDK

**Language:** [English](../en/packet-generator-application.md) | [日本語](../ja/packet-generator-application.md) | [Français](../fr/packet-generator-application.md) | [中文](../zh/packet-generator-application.md) | [Deutsch](../de/packet-generator-application.md) | [Italiano](../it/packet-generator-application.md) | [한국어](../ko/packet-generator-application.md) | **ไทย**

แอปพลิเคชัน Packet Generator ให้ความสามารถในการสร้างและส่งแพ็กเก็ตประสิทธิภาพสูงสำหรับการทดสอบเครือข่าย การวัดประสิทธิภาพ และการตรวจสอบ

## ภาพรวม

Packet Generator (PKTGEN) เป็นเครื่องมือที่มีประสิทธิภาพสำหรับ:
- **การทดสอบประสิทธิภาพ**: การวัด throughput, latency, และ packet loss
- **การทดสอบภาระ**: การสร้างทราฟฟิกจำนวนมากเพื่อทดสอบความสามารถของระบบ
- **การตรวจสอบการทำงาน**: การตรวจสอบการทำงานของเครือข่าย
- **การพัฒนา**: การทดสอบและการตรวจสอบฟีเจอร์ใหม่

## ฟีเจอร์หลัก

### การสร้างแพ็กเก็ตประสิทธิภาพสูง
- **ความเร็วสูง**: สร้างแพ็กเก็ตได้หลายล้านแพ็กเก็ตต่อวินาที
- **การใช้ DPDK**: ใช้ประโยชน์จาก zero-copy packet processing
- **Multi-core Support**: รองรับการประมวลผลแบบขนาน
- **Burst Mode**: การส่งแพ็กเก็ตแบบ burst เพื่อประสิทธิภาพสูงสุด

### รูปแบบแพ็กเก็ตที่หลากหลาย
- **Ethernet Frames**: เฟรม Ethernet พื้นฐาน
- **IP Packets**: แพ็กเก็ต IPv4 และ IPv6
- **UDP/TCP Packets**: แพ็กเก็ต UDP และ TCP
- **VLAN Tagged**: แพ็กเก็ตที่มี VLAN tag
- **Custom Patterns**: รูปแบบข้อมูลที่กำหนดเอง

### การควบคุมการส่ง
- **Rate Control**: การควบคุมอัตราการส่งแพ็กเก็ต
- **Packet Size**: การกำหนดขนาดแพ็กเก็ต
- **Duration Control**: การควบคุมระยะเวลาการส่ง
- **Pattern Control**: การควบคุมรูปแบบข้อมูล

## การตั้งค่า

### การตั้งค่าพื้นฐาน
```bash
# ตั้งค่าพารามิเตอร์ PKTGEN
set argv-list 0 "-c 0x3 -n 4 --socket-mem 1024"

# เริ่มต้น PKTGEN
pktgen init argv-list 0

# ตั้งค่า PKTGEN worker
set worker lcore 1 pktgen
start worker lcore 1
```

### การตั้งค่าขั้นสูง
```bash
# การตั้งค่าหน่วยความจำและ hugepage
set argv-list 0 "-c 0x7 -n 4"
set argv-list 1 "--socket-mem 2048"
set argv-list 2 "--huge-dir /mnt/huge"

# เริ่มต้นด้วยการตั้งค่าที่ปรับแต่ง
pktgen init argv-list 0
```

## การทำงาน

### การเริ่มต้นการสร้างแพ็กเก็ต
```bash
# เริ่มการสร้างแพ็กเก็ตที่พอร์ตเฉพาะ
pktgen do start 0

# เริ่มการสร้างแพ็กเก็ตที่ทุกพอร์ต
pktgen do start all
```

### การหยุดการสร้างแพ็กเก็ต
```bash
# หยุดการสร้างแพ็กเก็ตที่พอร์ตเฉพาะ
pktgen do stop 0

# หยุดการสร้างแพ็กเก็ตที่ทุกพอร์ต
pktgen do stop all
```

### การตรวจสอบสถานะ
```bash
# แสดงสถานะ PKTGEN
show pktgen

# แสดงสถิติพอร์ต
show port statistics pps
show port statistics total
```

## การตั้งค่าแพ็กเก็ต

### รูปแบบแพ็กเก็ตพื้นฐาน
PKTGEN สามารถสร้างแพ็กเก็ตหลายรูปแบบ:
- **Fixed Size**: แพ็กเก็ตขนาดคงที่
- **Random Size**: แพ็กเก็ตขนาดสุ่ม
- **Pattern Data**: ข้อมูลรูปแบบเฉพาะ
- **Sequence Numbers**: หมายเลขลำดับ

### การควบคุมอัตรา
- **PPS (Packets Per Second)**: ควบคุมจำนวนแพ็กเก็ตต่อวินาที
- **BPS (Bits Per Second)**: ควบคุมจำนวนบิตต่อวินาที
- **Percentage**: ควบคุมเป็นเปอร์เซ็นต์ของความเร็วสูงสุด

## การตรวจสอบประสิทธิภาพ

### การวัดผลการทำงาน
```bash
# วัด throughput
show port statistics pps
show port statistics Bps

# วัดจำนวนแพ็กเก็ตรวม
show port statistics total
show port statistics total-bytes
```

### การวิเคราะห์
```bash
# วิเคราะห์การใช้งาน CPU
show thread counter
show loop-count pktgen pps

# วิเคราะห์การใช้งานหน่วยความจำ
show mempool
```

## กรณีการใช้งาน

### การทดสอบ Throughput
```bash
# การตั้งค่าสำหรับการทดสอบ throughput สูงสุด
set argv-list 0 "-c 0xF -n 4 --socket-mem 4096"
pktgen init argv-list 0

# เริ่มการทดสอบ
pktgen do start all

# ตรวจสอบผลลัพธ์
show port statistics pps
```

### การทดสอบ Latency
```bash
# การตั้งค่าสำหรับการทดสอบ latency
# ใช้แพ็กเก็ตขนาดเล็กและอัตราต่ำ
pktgen do start 0

# วิเคราะห์ latency ผ่านเครื่องมือภายนอก
```

### การทดสอบ Load
```bash
# การสร้างภาระจำลองสำหรับการทดสอบความเสียชย์
pktgen do start all

# ตรวจสอบสถิติระหว่างการทดสอบ
show port statistics
```

## การปรับแต่งประสิทธิภาพ

### การจัดการ CPU
```bash
# กำหนด PKTGEN ให้กับ core เฉพาะ
set worker lcore 1 pktgen
set worker lcore 2 pktgen  # สำหรับการประมวลผลขนาน
```

### การจัดการหน่วยความจำ
```bash
# เพิ่ม hugepage memory
set argv-list 1 "--socket-mem 4096"

# ปรับแต่ง memory pool
set mempool
```

### การตั้งค่าพอร์ต
```bash
# ปรับแต่ง descriptor rings
set port all nrxdesc 2048
set port all ntxdesc 2048

# เปิด promiscuous mode
set port all promiscuous enable
```

## การแก้ไขปัญหา

### ปัญหาทั่วไป
1. **PKTGEN ไม่เริ่มต้น**
   ```bash
   show pktgen
   show worker
   show mempool
   ```

2. **ประสิทธิภาพต่ำ**
   ```bash
   show thread counter
   show port statistics
   ```

3. **หน่วยความจำไม่เพียงพอ**
   ```bash
   show mempool
   # เพิ่ม --socket-mem ใน argv-list
   ```

### การดีบัก
```bash
# เปิดใช้งานการดีบัก
debug sdplane pktgen

# ตรวจสอบ log
show debugging sdplane
```

## ข้อจำกัด

- **หน่วยความจำ**: ต้องการหน่วยความจำเพียงพอสำหรับ packet buffer
- **CPU**: ต้องการ CPU core เพียงพอสำหรับการสร้างแพ็กเก็ตความเร็วสูง
- **พอร์ต**: ประสิทธิภาพขึ้นอยู่กับความสามารถของ network interface
- **การตั้งค่า**: ต้องการการปรับแต่งระดับต่ำสำหรับประสิทธิภาพสูงสุด

## แอปพลิเคชันที่เกี่ยวข้อง

- **L2 Repeater**: สำหรับการทดสอบการส่งต่อ L2
- **Enhanced Repeater**: สำหรับการทดสอบ VLAN
- **L3 Forwarding**: สำหรับการทดสอบการ routing

สำหรับข้อมูลเพิ่มเติมเกี่ยวกับการใช้งาน PKTGEN ดู [คำสั่งการสร้างแพ็กเก็ต](packet-generation.md)