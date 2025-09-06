# การจัดการ Worker และ lcore

**ภาษา / Language:** [English](../worker-management.md) | [日本語](../ja/worker-management.md) | [Français](../fr/worker-management.md) | [中文](../zh/worker-management.md) | [Deutsch](../de/worker-management.md) | [Italiano](../it/worker-management.md) | [한국어](../ko/worker-management.md) | **ไทย**

คำสั่งที่จัดการเธรด Worker ของ DPDK และ lcore

## รายการคำสั่ง

### set_worker - การตั้งค่าประเภท Worker
```
set worker lcore <0-16> (|none|l2fwd|l3fwd|l3fwd-lpm|tap-handler|l2-repeater|enhanced-repeater|vlan-switch|pktgen|linkflap-generator)
```

ตั้งค่าประเภท Worker สำหรับ lcore ที่ระบุ

**ประเภท Worker:**
- `none` - ไม่มี Worker
- `l2fwd` - Layer 2 Forwarding
- `l3fwd` - Layer 3 Forwarding
- `l3fwd-lpm` - Layer 3 Forwarding (LPM)
- `tap-handler` - TAP Interface Handler
- `l2-repeater` - Layer 2 Repeater
- `enhanced-repeater` - Enhanced Repeater พร้อม VLAN switching และ TAP interface
- `vlan-switch` - VLAN Switch
- `pktgen` - Packet Generator
- `linkflap-generator` - Link Flap Generator

**ตัวอย่างการใช้งาน:**
```bash
# ตั้งค่า Worker L2 forwarding สำหรับ lcore 1
set worker lcore 1 l2fwd

# ตั้งค่า Worker enhanced repeater สำหรับ lcore 1
set worker lcore 1 enhanced-repeater

# ตั้งค่าไม่มี Worker สำหรับ lcore 2
set worker lcore 2 none

# ตั้งค่า Worker L3 forwarding (LPM) สำหรับ lcore 3
set worker lcore 3 l3fwd-lpm
```

### reset_worker - การรีเซ็ต Worker
```
reset worker lcore <0-16>
```

รีเซ็ต Worker ของ lcore ที่ระบุ

**ตัวอย่างการใช้งาน:**
```bash
# รีเซ็ต Worker ของ lcore 2
reset worker lcore 2
```

### start_worker - การเริ่มต้น Worker
```
start worker lcore <0-16>
```

เริ่มต้น Worker ของ lcore ที่ระบุ

**ตัวอย่างการใช้งาน:**
```bash
# เริ่มต้น Worker ของ lcore 1
start worker lcore 1
```

### restart_worker - การเริ่มต้น Worker ใหม่
```
restart worker lcore <0-16>
```

เริ่มต้น Worker ใหม่ของ lcore ที่ระบุ

**ตัวอย่างการใช้งาน:**
```bash
# เริ่มต้น Worker ใหม่ของ lcore 4
restart worker lcore 4
```

### start_worker_all - การเริ่มต้น Worker (พร้อมตัวเลือก all)
```
start worker lcore (<0-16>|all)
```

เริ่มต้น Worker ของ lcore ที่ระบุหรือ lcore ทั้งหมด

**ตัวอย่างการใช้งาน:**
```bash
# เริ่มต้น Worker ของ lcore 1
start worker lcore 1

# เริ่มต้น Worker ทั้งหมด
start worker lcore all
```

### stop_worker - การหยุด Worker
```
stop worker lcore (<0-16>|all)
```

หยุด Worker ของ lcore ที่ระบุหรือ lcore ทั้งหมด

**ตัวอย่างการใช้งาน:**
```bash
# หยุด Worker ของ lcore 1
stop worker lcore 1

# หยุด Worker ทั้งหมด
stop worker lcore all
```

### reset_worker_all - การรีเซ็ต Worker (พร้อมตัวเลือก all)
```
reset worker lcore (<0-16>|all)
```

รีเซ็ต Worker ของ lcore ที่ระบุหรือ lcore ทั้งหมด

**ตัวอย่างการใช้งาน:**
```bash
# รีเซ็ต Worker ของ lcore 2
reset worker lcore 2

# รีเซ็ต Worker ทั้งหมด
reset worker lcore all
```

### restart_worker_all - การเริ่มต้น Worker ใหม่ (พร้อมตัวเลือก all)
```
restart worker lcore (<0-16>|all)
```

เริ่มต้น Worker ใหม่ของ lcore ที่ระบุหรือ lcore ทั้งหมด

**ตัวอย่างการใช้งาน:**
```bash
# เริ่มต้น Worker ใหม่ของ lcore 3
restart worker lcore 3

# เริ่มต้น Worker ใหม่ทั้งหมด
restart worker lcore all
```

### show_worker - การแสดงข้อมูล Worker
```
show worker
```

แสดงสถานะและการตั้งค่าปัจจุบันของ Worker

**ตัวอย่างการใช้งาน:**
```bash
show worker
```

### set_mempool - การตั้งค่า Memory Pool
```
set mempool
```

ตั้งค่า DPDK Memory Pool

**ตัวอย่างการใช้งาน:**
```bash
set mempool
```

### set_rte_eal_argv - การตั้งค่าอาร์กิวเมนต์ RTE EAL Command Line
```
set rte_eal argv <WORD> <WORD> <WORD> <WORD> <WORD> <WORD>
```

ตั้งค่าอาร์กิวเมนต์ Command Line ที่ใช้สำหรับการเริ่มต้น RTE EAL (Environment Abstraction Layer)

**ตัวอย่างการใช้งาน:**
```bash
# ตั้งค่าอาร์กิวเมนต์ EAL
set rte_eal argv -c 0x1 -n 4 --socket-mem 1024,1024 --huge-dir /mnt/huge
```

### rte_eal_init - การเริ่มต้น RTE EAL
```
rte_eal_init
```

เริ่มต้น RTE EAL (Environment Abstraction Layer)

**ตัวอย่างการใช้งาน:**
```bash
rte_eal_init
```

## คำอธิบายประเภท Worker

### L2 Forwarding (l2fwd)
Worker ที่ทำการส่งต่อแพ็กเก็ตในระดับ Layer 2 โดยส่งต่อแพ็กเก็ตตาม MAC address

### L3 Forwarding (l3fwd)
Worker ที่ทำการส่งต่อแพ็กเก็ตในระดับ Layer 3 โดยทำการ routing ตาม IP address

### L3 Forwarding LPM (l3fwd-lpm)
Worker การ Forwarding ใน Layer 3 ที่ใช้ Longest Prefix Matching (LPM)

### TAP Handler (tap-handler)
Worker ที่ทำการส่งต่อแพ็กเก็ตระหว่าง TAP interface และพอร์ต DPDK

### L2 Repeater (l2-repeater)
Worker ที่ทำการ duplicate และ repeat แพ็กเก็ตในระดับ Layer 2

### VLAN Switch (vlan-switch)
Worker การ Switching ที่ให้บริการฟังก์ชัน VLAN (Virtual LAN)

### Packet Generator (pktgen)
Worker ที่สร้างแพ็กเก็ตสำหรับการทดสอบ

### Link Flap Generator (linkflap-generator)
Worker สำหรับทดสอบการเปลี่ยนแปลงสถานะของ network link

## ตำแหน่งการกำหนด

คำสั่งเหล่านี้ถูกกำหนดในไฟล์ต่อไปนี้:
- `sdplane/dpdk_lcore_cmd.c`

## รายการที่เกี่ยวข้อง

- [การจัดการพอร์ตและสถิติ](port-management.md)
- [ข้อมูลเธรด](thread-information.md)
- [การจัดการ lthread](lthread-management.md)