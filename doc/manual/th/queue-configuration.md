# การตั้งค่าคิว

**ภาษา / Language:** [English](../queue-configuration.md) | [日本語](../ja/queue-configuration.md) | [Français](../fr/queue-configuration.md) | [中文](../zh/queue-configuration.md) | [Deutsch](../de/queue-configuration.md) | [Italiano](../it/queue-configuration.md) | [한국어](../ko/queue-configuration.md) | **ไทย**

คำสั่งที่ตั้งค่าและจัดการคิว DPDK

## รายการคำสั่ง

### update_port_status - การอัปเดตสถานะพอร์ต
```
update port status
```

อัปเดตสถานะของทุกพอร์ต

**ตัวอย่างการใช้งาน:**
```bash
update port status
```

คำสั่งนี้จะดำเนินการต่อไปนี้:
- ตรวจสอบสถานะลิงก์ของแต่ละพอร์ต
- อัปเดตการตั้งค่าคิู
- อัปเดตข้อมูลสถิติของพอร์ต

### set_thread_lcore_port_queue - การตั้งค่าคิวเธรด
```
set thread <0-128> port <0-128> queue <0-128>
```

กำหนดพอร์ตและคิวให้กับเธรดที่ระบุ

**พารามิเตอร์:**
- `<0-128>` (thread) - หมายเลขเธรด
- `<0-128>` (port) - หมายเลขพอร์ต
- `<0-128>` (queue) - หมายเลขคิว

**ตัวอย่างการใช้งาน:**
```bash
# กำหนดคิว 0 ของพอร์ต 0 ให้กับเธรด 0
set thread 0 port 0 queue 0

# กำหนดคิว 1 ของพอร์ต 1 ให้กับเธรด 1
set thread 1 port 1 queue 1

# กำหนดคิว 1 ของพอร์ต 0 ให้กับเธรด 2
set thread 2 port 0 queue 1
```

### show_thread_qconf - การแสดงการตั้งค่าคิวเธรด
```
show thread qconf
```

แสดงการตั้งค่าคิวเธรดปัจจุบัน

**ตัวอย่างการใช้งาน:**
```bash
show thread qconf
```

## ภาพรวมของระบบคิว

### แนวคิดของคิว DPDK
ใน DPDK แต่ละพอร์ตสามารถตั้งค่าคิวส่งและรับหลายคิวได้:

- **คิวรับ (RX Queue)** - รับแพ็กเก็ตขาเข้า
- **คิวส่ง (TX Queue)** - ส่งแพ็กเก็ตขาออก
- **Multi-queue** - การประมวลผลคิวหลายคิูแบบขนาน

### ความสำคัญของการตั้งค่าคิว
การตั้งค่าคิูที่เหมาะสมจะช่วยให้เกิดผลลัพธ์ต่อไปนี้:
- **การปรับปรุงประสิทธิภาพ** - การเร่งความเร็วด้วยการประมวลผลแบบขนาน
- **การกระจายภาระ** - การกระจายการประมวลผลด้วย Worker หลายตัว
- **ประสิทธิภาพ CPU** - การใช้ CPU core อย่างมีประสิทธิภาพ

## วิธีการตั้งค่าคิู

### ขั้นตอนการตั้งค่าพื้นฐาน
1. **การอัปเดตสถานะพอร์ต**
```bash
update port status
```

2. **การตั้งค่าคิูเธรด**
```bash
set thread 0 port 0 queue 0
set thread 1 port 0 queue 1
```

3. **การตรวจสอบการตั้งค่า**
```bash
show thread qconf
```

### รูปแบบการตั้งค่าที่แนะนำ

#### พอร์ตเดียว คิวเดียว
```bash
set thread 0 port 0 queue 0
```

#### พอร์ตเดียว หลายคิว
```bash
set thread 0 port 0 queue 0
set thread 1 port 0 queue 1
set thread 2 port 0 queue 2
```

#### หลายพอร์ต หลายคิว
```bash
set thread 0 port 0 queue 0
set thread 1 port 1 queue 0
set thread 2 port 2 queue 0
```

## ตัวอย่างการตั้งค่า

### การตั้งค่าประสิทธิภาพสูง (4 คอร์, 4 พอร์ต)
```bash
# อัปเดตสถานะพอร์ต
update port status

# กำหนดพอร์ตต่างกันให้กับแต่ละคอร์
set thread 0 port 0 queue 0
set thread 1 port 1 queue 0
set thread 2 port 2 queue 0
set thread 3 port 3 queue 0

# ตรวจสอบการตั้งค่า
show thread qconf
```

### การตั้งค่าการกระจายภาระ (2 คอร์, 1 พอร์ต)
```bash
# อัปเดตสถานะพอร์ต
update port status

# ตั้งค่าหลายคิวสำหรับพอร์ตหนึ่ง
set thread 0 port 0 queue 0
set thread 1 port 0 queue 1

# ตรวจสอบการตั้งค่า
show thread qconf
```

## การปรับแต่งประสิทธิภาพ

### การกำหนดจำนวนคิู
- **จำนวน CPU core** - ตั้งค่าตามจำนวน CPU core ที่ใช้ได้
- **จำนวนพอร์ต** - พิจารณาจำนวนพอร์ตทางกายภาพ
- **ลักษณะการรับส่งข้อมูล** - รูปแบบการรับส่งข้อมูลที่คาดหวัง

### จุดของการปรับให้เหมาะสม
1. **CPU Affinity** - การจัดวาง CPU core และคิวอย่างเหมาะสม
2. **การจัดวางหน่วยความจำ** - การจัดวางหน่วยความจำโดยพิจารณา NUMA node
3. **การประมวลผล Interrupt** - การประมวลผล interrupt อย่างมีประสิทธิภาพ

## การแก้ไขปัญหา

### เมื่อการตั้งค่าคิวไม่มีผล
1. อัปเดตสถานะพอร์ต
```bash
update port status
```

2. ตรวจสอบสถานะ Worker
```bash
show worker
```

3. ตรวจสอบสถานะพอร์ต
```bash
show port
```

### เมื่อประสิทธิภาพไม่ปรับปรุง
1. ตรวจสอบการตั้งค่าคิว
```bash
show thread qconf
```

2. ตรวจสอบภาระของเธรด
```bash
show thread counter
```

3. ตรวจสอบสถิติพอร์ต
```bash
show port statistics
```

## ตำแหน่งการกำหนด

คำสั่งเหล่านี้ถูกกำหนดในไฟล์ต่อไปนี้:
- `sdplane/queue_config.c`

## รายการที่เกี่ยวข้อง

- [การจัดการพอร์ตและสถิติ](port-management.md)
- [การจัดการ Worker และ lcore](worker-management.md)
- [ข้อมูลเธรด](thread-information.md)