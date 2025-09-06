# ข้อมูลระบบและการตรวจสอบ

**ภาษา / Language:** [English](../system-monitoring.md) | [日本語](../ja/system-monitoring.md) | [Français](../fr/system-monitoring.md) | [中文](../zh/system-monitoring.md) | [Deutsch](../de/system-monitoring.md) | [Italiano](../it/system-monitoring.md) | [한국어](../ko/system-monitoring.md) | **ไทย**

คำสั่งที่ให้ข้อมูลระบบและฟังก์ชันการตรวจสอบของ sdplane

## รายการคำสั่ง

### show_version - การแสดงเวอร์ชัน
```
show version
```

แสดงข้อมูลเวอร์ชันของ sdplane

**ตัวอย่างการใช้งาน:**
```bash
show version
```

### set_locale - การตั้งค่าโลเคล
```
set locale (C|C.utf8|en_US.utf8|POSIX)
```

ตั้งค่าโลเคลของระบบ

**โลเคลที่ใช้ได้:**
- `C` - โลเคล C มาตรฐาน
- `C.utf8` - โลเคล C รองรับ UTF-8
- `en_US.utf8` - โลเคลภาษาอังกฤษ UTF-8
- `POSIX` - โลเคล POSIX

**ตัวอย่างการใช้งาน:**
```bash
# ตั้งค่าโลเคล C รองรับ UTF-8
set locale C.utf8

# ตั้งค่าโลเคลภาษาอังกฤษ UTF-8
set locale en_US.utf8
```

### set_argv_list_1 - การตั้งค่า argv-list
```
set argv-list <0-7> <WORD>
```

ตั้งค่ารายการอาร์กิวเมนต์ command line

**พารามิเตอร์:**
- `<0-7>` - ดัชนี (0-7)
- `<WORD>` - สตริงที่ต้องการตั้งค่า

**ตัวอย่างการใช้งาน:**
```bash
# ตั้งค่าอาร์กิวเมนต์ที่ดัชนี 0
set argv-list 0 "--verbose"

# ตั้งค่าอาร์กิวเมนต์ที่ดัชนี 1
set argv-list 1 "--config"
```

### **show argv-list**

แสดงรายการอาร์กิวเมนต์ command line ทั้งหมดที่ตั้งค่าไว้

**ตัวอย่างการใช้งาน:**
```bash
# แสดง argv-list ทั้งหมด
show argv-list
```

---

### **show argv-list \<0-7\>**

แสดง argv-list ของดัชนีที่ระบุ

**ตัวอย่างการใช้งาน:**
```bash
# แสดง argv-list ของดัชนีที่ระบุ
show argv-list 0

# แสดง argv-list ดัชนี 3
show argv-list 3
```

### show_loop_count - การแสดงตัวนับลูป
```
show loop-count (console|vty-shell|l2fwd) (pps|total)
```

แสดงตัวนับลูปของแต่ละคอมโพเนนต์

**คอมโพเนนต์:**
- `console` - คอนโซล
- `vty-shell` - VTY Shell
- `l2fwd` - L2 Forwarding

**ประเภทสถิติ:**
- `pps` - จำนวนลูปต่อวินาที
- `total` - จำนวนลูปรวม

**ตัวอย่างการใช้งาน:**
```bash
# แสดง PPS ของคอนโซล
show loop-count console pps

# แสดงจำนวนลูปรวมของ L2 forwarding
show loop-count l2fwd total
```

### show_rcu - การแสดงข้อมูล RCU
```
show rcu
```

แสดงข้อมูล RCU (Read-Copy-Update)

**ตัวอย่างการใช้งาน:**
```bash
show rcu
```

### show_fdb - การแสดงข้อมูล FDB
```
show fdb
```

แสดงข้อมูล FDB (Forwarding Database)

**ตัวอย่างการใช้งาน:**
```bash
show fdb
```

### show_vswitch - การแสดงข้อมูล vswitch
```
show vswitch
```

แสดงข้อมูลของ virtual switch

**ตัวอย่างการใช้งาน:**
```bash
show vswitch
```

### sleep_cmd - คำสั่ง Sleep
```
sleep <0-300>
```

Sleep เป็นจำนวนวินาทีที่ระบุ

**พารามิเตอร์:**
- `<0-300>` - เวลา sleep (วินาที)

**ตัวอย่างการใช้งาน:**
```bash
# Sleep 5 วินาที
sleep 5

# Sleep 30 วินาที
sleep 30
```

### show_mempool - การแสดงข้อมูล Memory Pool
```
show mempool
```

แสดงข้อมูล DPDK Memory Pool

**ตัวอย่างการใช้งาน:**
```bash
show mempool
```

## คำอธิบายรายการตรวจสอบ

### ข้อมูลเวอร์ชัน
- เวอร์ชันของ sdplane
- ข้อมูลการ build
- เวอร์ชันของไลบรารีที่ใช้

### ตัวนับลูป
- จำนวนรอบการประมวลผลของแต่ละคอมโพเนนต์
- ใช้สำหรับการตรวจสอบประสิทธิภาพ
- ใช้ในการคำนวณ PPS (Packets Per Second)

### ข้อมูล RCU
- สถานะของกลไก Read-Copy-Update
- สถานการณ์ของการประมวลผลแบบ synchronous
- สถานะการจัดการหน่วยความจำ

### ข้อมูล FDB
- สถานะของตาราง MAC address
- MAC address ที่เรียนรู้แล้ว
- ข้อมูล aging

### ข้อมูล vswitch
- การตั้งค่าของ virtual switch
- ข้อมูลพอร์ต
- การตั้งค่า VLAN

### ข้อมูล Memory Pool
- หน่วยความจำที่ใช้ได้
- หน่วยความจำที่ใช้อยู่
- สถิติของ memory pool

## แนวปฏิบัติที่ดีในการตรวจสอบ

### การตรวจสอบเป็นระยะ
```bash
# คำสั่งการตรวจสอบพื้นฐาน
show version
show mempool
show vswitch
show rcu
```

### การตรวจสอบประสิทธิภาพ
```bash
# การตรวจสอบประสิทธิภาพด้วยตัวนับลูป
show loop-count console pps
show loop-count l2fwd pps
```

### การแก้ไขปัญหา
```bash
# การตรวจสอบสถานะระบบ
show fdb
show vswitch
show mempool
```

## ตำแหน่งการกำหนด

คำสั่งเหล่านี้ถูกกำหนดในไฟล์ต่อไปนี้:
- `sdplane/sdplane.c`

## รายการที่เกี่ยวข้อง

- [การจัดการพอร์ตและสถิติ](port-management.md)
- [การจัดการ Worker และ lcore](worker-management.md)
- [ข้อมูลเธรด](thread-information.md)