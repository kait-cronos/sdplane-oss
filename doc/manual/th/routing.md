# RIB และการเราต์

**Language:** [English](../en/routing.md) | [日本語](../ja/routing.md) | [Français](../fr/routing.md) | [中文](../zh/routing.md) | [Deutsch](../de/routing.md) | [Italiano](../it/routing.md) | [한국어](../ko/routing.md) | **ไทย**

คำสั่งที่จัดการ RIB (Routing Information Base) และข้อมูลทรัพยากรระบบ

## รายการคำสั่ง

### show_rib - การแสดงข้อมูล RIB
```
show rib
```

แสดงข้อมูล RIB (Routing Information Base)

**ตัวอย่างการใช้งาน:**
```bash
show rib
```

คำสั่งนี้จะแสดงข้อมูลต่อไปนี้:
- เวอร์ชัน RIB และ memory pointer
- การตั้งค่า virtual switch และการกำหนด VLAN
- สถานะพอร์ต DPDK และการตั้งค่าคิว
- การกำหนดคิว lcore-to-port
- ตาราง neighbor สำหรับการส่งต่อ L2/L3

## ภาพรวมของ RIB

### RIB คืออะไร
RIB (Routing Information Base) เป็นฐานข้อมูลกลางที่เก็บทรัพยากรระบบและข้อมูลเครือข่าย ใน sdplane จะจัดการข้อมูลต่อไปนี้:

- **การตั้งค่า Virtual Switch** - การ switching VLAN และการกำหนดพอร์ต
- **ข้อมูลพอร์ต DPDK** - สถานะลิงก์ การตั้งค่าคิว ข้อมูลฟีเจอร์
- **การตั้งค่าคิว lcore** - การกำหนดการประมวลผลแพ็กเก็ตสำหรับแต่ละ CPU core
- **ตาราง Neighbor** - รายการฐานข้อมูลการส่งต่อ L2/L3

### โครงสร้างของ RIB
RIB ประกอบด้วย structure หลัก 2 ตัว:

```c
struct rib {
    struct rib_info *rib_info;  // pointer ไปยังข้อมูลจริง
};

struct rib_info {
    uint32_t ver;                                    // หมายเลขเวอร์ชัน
    uint8_t vswitch_size;                           // จำนวน virtual switch
    uint8_t port_size;                              // จำนวนพอร์ต DPDK
    uint8_t lcore_size;                             // จำนวน lcore
    struct vswitch_conf vswitch[MAX_VSWITCH];       // การตั้งค่า virtual switch
    struct vswitch_link vswitch_link[MAX_VSWITCH_LINK]; // ลิงก์พอร์ต VLAN
    struct port_conf port[MAX_ETH_PORTS];           // การตั้งค่าพอร์ต DPDK
    struct lcore_qconf lcore_qconf[RTE_MAX_LCORE];  // การกำหนดคิว lcore
    struct neigh_table neigh_tables[NEIGH_NR_TABLES]; // ตาราง neighbor/forwarding
};
```

## วิธีการอ่านข้อมูล RIB

### รายการการแสดงผลพื้นฐาน
- **Destination** - network address ปลายทาง
- **Netmask** - netmask
- **Gateway** - gateway (next hop)
- **Interface** - interface ส่งออก
- **Metric** - ค่า metric ของ route
- **Status** - สถานะของ route

### สถานะของ Route
- **Active** - route ที่ active
- **Inactive** - route ที่ inactive
- **Pending** - route ที่กำลังตั้งค่า
- **Invalid** - route ที่ไม่ถูกต้อง

## ตัวอย่างการใช้งาน

### วิธีการใช้งานพื้นฐาน
```bash
# แสดงข้อมูล RIB
show rib
```

### การตีความผลลัพธ์
```
rib information version: 21 (0x55555dd42010)
vswitches: 
dpdk ports: 
  dpdk_port[0]: 
    link: speed=1000Mbps duplex=full autoneg=on status=up
    nb_rxd=1024 nb_txd=1024
    queues: nrxq=1 ntxq=4
  dpdk_port[1]: 
    link: speed=0Mbps duplex=half autoneg=on status=down
    nb_rxd=1024 nb_txd=1024
    queues: nrxq=1 ntxq=4
  dpdk_port[2]: 
    link: speed=0Mbps duplex=half autoneg=off status=down
    nb_rxd=1024 nb_txd=1024
    queues: nrxq=1 ntxq=4
lcores: 
  lcore[0]: nrxq=0
  lcore[1]: nrxq=1
    rxq[0]: dpdk_port[0], queue_id=0
  lcore[2]: nrxq=1
    rxq[0]: dpdk_port[1], queue_id=0
  lcore[3]: nrxq=1
    rxq[0]: dpdk_port[2], queue_id=0
  lcore[4]: nrxq=0
  lcore[5]: nrxq=0
  lcore[6]: nrxq=0
  lcore[7]: nrxq=0
```

ในตัวอย่างนี้:
- RIB เวอร์ชัน 21 แสดงสถานะระบบปัจจุบัน
- พอร์ต DPDK 0 ทำงาน (up) ด้วยความเร็วลิงก์ 1Gbps
- พอร์ต DPDK 1, 2 ไม่ทำงาน (down) และไม่มีลิงก์
- lcore 1, 2, 3 รับผิดชอบการประมวลผลแพ็กเก็ตจากพอร์ต 0, 1, 2 ตามลำดับ
- แต่ละพอร์ตใช้ 1 RX คิวและ 4 TX คิว
- RX/TX descriptor ring ตั้งค่าที่ 1024 entries

## การจัดการ RIB

### การอัปเดตอัตโนมัติ
RIB จะถูกอัปเดตโดยอัตโนมัติใน timing ต่อไปนี้:
- การเปลี่ยนแปลงสถานะของ interface
- การเปลี่ยนแปลงการตั้งค่าเครือข่าย
- การรับข้อมูลจาก routing protocol

### การอัปเดตด้วยตนเอง
การตรวจสอบข้อมูล RIB ด้วยตนเอง:
```bash
show rib
```

## การแก้ไขปัญหา

### เมื่อการเราต์ไม่ทำงานอย่างถูกต้อง
1. ตรวจสอบข้อมูล RIB
```bash
show rib
```

2. ตรวจสอบสถานะ interface
```bash
show port
```

3. ตรวจสอบสถานะ Worker
```bash
show worker
```

### เมื่อไม่มี route แสดงใน RIB
- ตรวจสอบการตั้งค่าเครือข่าย
- ตรวจสอบสถานะของ interface
- ตรวจสอบการทำงานของ RIB manager

## ฟังก์ชันขั้นสูง

### RIB Manager
RIB Manager ทำงานเป็นเธรดแยกต่างหาก และให้ฟังก์ชันต่อไปนี้:
- การอัปเดตข้อมูลการเราต์อัตโนมัติ
- การตรวจสอบความถูกต้องของ route
- การตรวจสอบสถานะเครือข่าย

### Worker ที่เกี่ยวข้อง
- **rib-manager** - Worker ที่จัดการ RIB
- **l3fwd** - ใช้ RIB ในการ Layer 3 forwarding
- **l3fwd-lpm** - ความร่วมมือระหว่าง LPM table และ RIB

## ตำแหน่งการกำหนด

คำสั่งนี้ถูกกำหนดในไฟล์ต่อไปนี้:
- `sdplane/rib.c`

## รายการที่เกี่ยวข้อง

- [การจัดการ Worker และ lcore](worker-lcore-thread-management.md)
- [การจัดการ lthread](lthread-management.md)
- [ข้อมูลระบบและการตรวจสอบ](system-monitoring.md)