# ข้อมูลเธรด

**ภาษา / Language:** [English](../thread-information.md) | [日本語](../ja/thread-information.md) | [Français](../fr/thread-information.md) | [中文](../zh/thread-information.md) | [Deutsch](../de/thread-information.md) | [Italiano](../it/thread-information.md) | [한국어](../ko/thread-information.md) | **ไทย**

คำสั่งที่แสดงข้อมูลและตรวจสอบเธรด

## รายการคำสั่ง

### show_thread_cmd - การแสดงข้อมูลเธรด
```
show thread
```

แสดงสถานะและข้อมูลของเธรดปัจจุบัน

**ตัวอย่างการใช้งาน:**
```bash
show thread
```

คำสั่งนี้จะแสดงข้อมูลต่อไปนี้:
- Thread ID
- สถานะของเธรด
- งานที่กำลังทำงาน
- การใช้งาน CPU
- การใช้งานหน่วยความจำ

### show_thread_counter - การแสดงตัวนับเธรด
```
show thread counter
```

แสดงข้อมูลตัวนับของเธรด

**ตัวอย่างการใช้งาน:**
```bash
show thread counter
```

คำสั่งนี้จะแสดงข้อมูลต่อไปนี้:
- จำนวนแพ็กเก็ตที่ประมวลผล
- จำนวนครั้งที่ทำงาน
- จำนวนข้อผิดพลาด
- สถิติเวลาการประมวลผล

## ภาพรวมของระบบเธรด

### สถาปัตยกรรมเธรดของ sdplane
ใน sdplane จะใช้ประเภทเธรดต่อไปนี้:

#### 1. Worker Thread
- **L2 Forwarding** - การส่งต่อแพ็กเก็ต Layer 2
- **L3 Forwarding** - การส่งต่อแพ็กเก็ต Layer 3
- **TAP Handler** - การประมวลผล TAP interface
- **PKTGEN** - การสร้างแพ็กเก็ต

#### 2. Management Thread
- **RIB Manager** - การจัดการข้อมูล routing
- **Statistics Collector** - การรวบรวมข้อมูลสถิติ
- **Netlink Thread** - การประมวลผลการสื่อสาร Netlink

#### 3. System Thread
- **VTY Server** - การประมวลผลการเชื่อมต่อ VTY
- **Console** - การรับส่งข้อมูลคอนโซล

### ความสัมพันธ์กับ lthread
sdplane ใช้โมเดล cooperative threading:

- **lthread** - การใช้งาน lightweight thread
- **Cooperative Scheduling** - การควบคุมด้วยการ yield อย่างชัดเจน
- **ประสิทธิภาพสูง** - การลดโอเวอร์เฮดของการ context switch

## วิธีการอ่านข้อมูลเธรด

### รายการการแสดงผลพื้นฐาน
- **Thread ID** - ตัวระบุเธรด
- **Name** - ชื่อของเธรด
- **State** - สถานะของเธรด
- **lcore** - CPU core ที่กำลังทำงาน
- **Type** - ประเภทเธรด

### สถานะของเธรด
- **Running** - กำลังทำงาน
- **Ready** - พร้อมทำงาน
- **Blocked** - ถูกบล็อก
- **Terminated** - ยุติการทำงานแล้ว

### ข้อมูลตัวนับ
- **Packets** - จำนวนแพ็กเก็ตที่ประมวลผล
- **Loops** - จำนวนครั้งที่ทำลูป
- **Errors** - จำนวนข้อผิดพลาด
- **CPU Time** - เวลาใช้งาน CPU

## ตัวอย่างการใช้งาน

### การตรวจสอบพื้นฐาน
```bash
# แสดงข้อมูลเธรด
show thread

# แสดงตัวนับเธรด
show thread counter
```

### การตีความผลลัพธ์
```bash
# ตัวอย่างผลลัพธ์ show thread
Thread ID: 1
Name: l2fwd-worker
State: Running
lcore: 1
Type: L2FWD

Thread ID: 2
Name: rib-manager
State: Running
lcore: 2
Type: RIB_MANAGER
```

```bash
# ตัวอย่างผลลัพธ์ show thread counter
Thread ID: 1
Packets: 1000000
Loops: 5000000
Errors: 0
CPU Time: 123.45s
```

## การตรวจสอบและการแก้ไขปัญหา

### การตรวจสอบเป็นระยะ
```bash
# คำสั่งการตรวจสอบเป็นระยะ
show thread
show thread counter
```

### การวิเคราะห์ประสิทธิภาพ
```bash
# ข้อมูลที่เกี่ยวข้องกับประสิทธิภาพ
show thread counter
show loop-count l2fwd pps
show worker
```

### การแก้ไขปัญหา

#### เมื่อเธรดไม่ตอบสนอง
1. ตรวจสอบสถานะเธรด
```bash
show thread
```

2. ตรวจสอบสถานะ Worker
```bash
show worker
```

3. รีสตาร์ทหากจำเป็น
```bash
restart worker lcore 1
```

#### เมื่อประสิทธิภาพลดลง
1. ตรวจสอบข้อมูลตัวนับ
```bash
show thread counter
```

2. ตรวจสอบตัวนับลูป
```bash
show loop-count l2fwd pps
```

3. ตรวจสอบจำนวนข้อผิดพลาด
```bash
show thread counter
```

#### เมื่อการใช้งานหน่วยความจำมาก
1. ตรวจสอบข้อมูล memory pool
```bash
show mempool
```

2. ตรวจสอบข้อมูลเธรด
```bash
show thread
```

## การปรับปรุงเธรด

### การตั้งค่า CPU Affinity
- การจัดวาง Worker ใน lcore ที่เหมาะสม
- การจัดวางโดยพิจารณา NUMA node
- การกระจายการใช้งาน CPU อย่างเท่าเทียมกัน

### การปรับปรุงประสิทธิภาพหน่วยความจำ
- ขนาด memory pool ที่เหมาะสม
- การป้องกัน memory leak
- การปรับปรุงประสิทธิภาพของ cache

## ฟังก์ชันขั้นสูง

### การจัดการ lthread
```bash
# การตั้งค่า lthread worker
set worker lthread stat-collector
set worker lthread rib-manager
set worker lthread netlink-thread
```

### การใช้ประโยชน์จากข้อมูลสถิติ
- การตรวจสอบประสิทธิภาพ
- การวางแผนความจุ
- การตรวจจับสิ่งผิดปกติ

## ตำแหน่งการกำหนด

คำสั่งเหล่านี้ถูกกำหนดในไฟล์ต่อไปนี้:
- `sdplane/thread_info.c`

## รายการที่เกี่ยวข้อง

- [การจัดการ Worker และ lcore](worker-management.md)
- [การจัดการ lthread](lthread-management.md)
- [ข้อมูลระบบและการตรวจสอบ](system-monitoring.md)