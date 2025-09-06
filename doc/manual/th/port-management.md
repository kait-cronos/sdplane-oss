# การจัดการพอร์ตและสถิติ

**ภาษา / Language:** [English](../port-management.md) | [日本語](../ja/port-management.md) | [Français](../fr/port-management.md) | [中文](../zh/port-management.md) | [Deutsch](../de/port-management.md) | [Italiano](../it/port-management.md) | [한국어](../ko/port-management.md) | **ไทย**

คำสั่งที่จัดการพอร์ต DPDK และข้อมูลสถิติ

## รายการคำสั่ง

### **start port**

เริ่มต้นพอร์ต DPDK (การทำงานเริ่มต้น)

**ตัวอย่างการใช้งาน:**
```bash
# เริ่มต้นพอร์ต (เริ่มต้น)
start port
```

---

### **start port \<0-16\>**

เริ่มต้นพอร์ต DPDK ที่ระบุ

**ตัวอย่างการใช้งาน:**
```bash
# เริ่มต้นพอร์ต 0
start port 0

# เริ่มต้นพอร์ต 1
start port 1
```

---

### **start port all**

เริ่มต้นพอร์ต DPDK ทั้งหมด

**ตัวอย่างการใช้งาน:**
```bash
# เริ่มต้นทุกพอร์ต
start port all
```

---

### **stop port**

หยุดพอร์ต DPDK (การทำงานเริ่มต้น)

**ตัวอย่างการใช้งาน:**
```bash
# หยุดพอร์ต (เริ่มต้น)
stop port
```

---

### **stop port \<0-16\>**

หยุดพอร์ต DPDK ที่ระบุ

**ตัวอย่างการใช้งาน:**
```bash
# หยุดพอร์ต 0
stop port 0

# หยุดพอร์ต 1
stop port 1
```

---

### **stop port all**

หยุดพอร์ต DPDK ทั้งหมด

**ตัวอย่างการใช้งาน:**
```bash
# หยุดทุกพอร์ต
stop port all
```

---

### **reset port**

รีเซ็ตพอร์ต DPDK (การทำงานเริ่มต้น)

**ตัวอย่างการใช้งาน:**
```bash
# รีเซ็ตพอร์ต (เริ่มต้น)
reset port
```

---

### **reset port \<0-16\>**

รีเซ็ตพอร์ต DPDK ที่ระบุ

**ตัวอย่างการใช้งาน:**
```bash
# รีเซ็ตพอร์ต 0
reset port 0

# รีเซ็ตพอร์ต 1
reset port 1
```

---

### **reset port all**

รีเซ็ตพอร์ต DPDK ทั้งหมด

**ตัวอย่างการใช้งาน:**
```bash
# รีเซ็ตทุกพอร์ต
reset port all
```

---

### **show port**

แสดงข้อมูลพื้นฐานของทุกพอร์ต (การทำงานเริ่มต้น)

**ตัวอย่างการใช้งาน:**
```bash
# แสดงข้อมูลทุกพอร์ต
show port
```

---

### **show port \<0-16\>**

แสดงข้อมูลพื้นฐานของพอร์ตที่ระบุ

**ตัวอย่างการใช้งาน:**
```bash
# แสดงข้อมูลพอร์ต 0
show port 0

# แสดงข้อมูลพอร์ต 1
show port 1
```

---

### **show port all**

แสดงข้อมูลทุกพอร์ตอย่างชัดเจน

**ตัวอย่างการใช้งาน:**
```bash
# แสดงข้อมูลทุกพอร์ตอย่างชัดเจน
show port all
```

---

### **show port statistics**

แสดงข้อมูลสถิติทุกพอร์ต

**ตัวอย่างการใช้งาน:**
```bash
# แสดงข้อมูลสถิติทั้งหมด
show port statistics
```

---

### **show port statistics pps**

แสดงสถิติแพ็กเก็ต/วินาที

**ตัวอย่างการใช้งาน:**
```bash
# แสดงเฉพาะสถิติ PPS
show port statistics pps
```

---

### **show port statistics total**

แสดงสถิติจำนวนแพ็กเก็ตรวม

**ตัวอย่างการใช้งาน:**
```bash
# แสดงจำนวนแพ็กเก็ตรวม
show port statistics total
```

---

### **show port statistics bps**

แสดงสถิติบิต/วินาที

**ตัวอย่างการใช้งาน:**
```bash
# แสดงบิต/วินาที
show port statistics bps
```

---

### **show port statistics Bps**

แสดงสถิติไบต์/วินาที

**ตัวอย่างการใช้งาน:**
```bash
# แสดงไบต์/วินาที
show port statistics Bps
```

---

### **show port statistics total-bytes**

แสดงสถิติจำนวนไบต์รวม

**ตัวอย่างการใช้งาน:**
```bash
# แสดงจำนวนไบต์รวม
show port statistics total-bytes
```

---

### **show port \<0-16\> promiscuous**

แสดงสถานะโมด promiscuous ของพอร์ตที่ระบุ

**ตัวอย่างการใช้งาน:**
```bash
# แสดงโมด promiscuous ของพอร์ต 0
show port 0 promiscuous

# แสดงโมด promiscuous ของพอร์ต 1
show port 1 promiscuous
```

---

### **show port all promiscuous**

แสดงสถานะโมด promiscuous ของทุกพอร์ต

**ตัวอย่างการใช้งาน:**
```bash
# แสดงโมด promiscuous ของทุกพอร์ต
show port all promiscuous
```

---

### **show port \<0-16\> flowcontrol**

แสดงการตั้งค่าการควบคุมการไหลของพอร์ตที่ระบุ

**ตัวอย่างการใช้งาน:**
```bash
# แสดงการตั้งค่าการควบคุมการไหลของพอร์ต 0
show port 0 flowcontrol

# แสดงการตั้งค่าการควบคุมการไหลของพอร์ต 1
show port 1 flowcontrol
```

---

### **show port all flowcontrol**

แสดงการตั้งค่าการควบคุมการไหลของทุกพอร์ต

**ตัวอย่างการใช้งาน:**
```bash
# แสดงการตั้งค่าการควบคุมการไหลของทุกพอร์ต
show port all flowcontrol
```

---

### **set port \<0-16\> promiscuous enable**

เปิดใช้งานโมด promiscuous ของพอร์ตที่ระบุ

**ตัวอย่างการใช้งาน:**
```bash
# เปิดใช้งานโมด promiscuous ของพอร์ต 0
set port 0 promiscuous enable

# เปิดใช้งานโมด promiscuous ของพอร์ต 1
set port 1 promiscuous enable
```

---

### **set port \<0-16\> promiscuous disable**

ปิดใช้งานโมด promiscuous ของพอร์ตที่ระบุ

**ตัวอย่างการใช้งาน:**
```bash
# ปิดใช้งานโมด promiscuous ของพอร์ต 0
set port 0 promiscuous disable

# ปิดใช้งานโมด promiscuous ของพอร์ต 1
set port 1 promiscuous disable
```

---

### **set port all promiscuous enable**

เปิดใช้งานโมด promiscuous ของทุกพอร์ต

**ตัวอย่างการใช้งาน:**
```bash
# เปิดใช้งานโมด promiscuous ของทุกพอร์ต
set port all promiscuous enable
```

---

### **set port all promiscuous disable**

ปิดใช้งานโมด promiscuous ของทุกพอร์ต

**ตัวอย่างการใช้งาน:**
```bash
# ปิดใช้งานโมด promiscuous ของทุกพอร์ต
set port all promiscuous disable
```

---

### **set port \<0-16\> flowcontrol (rx|tx|autoneg|send-xon|fwd-mac-ctrl) (on|off)**

เปลี่ยนการตั้งค่าการควบคุมการไหลของพอร์ตที่ระบุ

**ตัวเลือก:**
- `rx` - การควบคุมการไหลการรับ
- `tx` - การควบคุมการไหลการส่ง
- `autoneg` - การเจรจาอัตโนมัติ
- `send-xon` - การส่ง XON
- `fwd-mac-ctrl` - การส่งต่อเฟรม MAC control

**ตัวอย่างการใช้งาน:**
```bash
# เปิดใช้งานการควบคุมการไหลการรับของพอร์ต 0
set port 0 flowcontrol rx on

# ปิดใช้งานการเจรจาอัตโนมัติของพอร์ต 1
set port 1 flowcontrol autoneg off
```

---

### **set port all flowcontrol (rx|tx|autoneg|send-xon|fwd-mac-ctrl) (on|off)**

เปลี่ยนการตั้งค่าการควบคุมการไหลของทุกพอร์ต

**ตัวเลือก:**
- `rx` - การควบคุมการไหลการรับ
- `tx` - การควบคุมการไหลการส่ง
- `autoneg` - การเจรจาอัตโนมัติ
- `send-xon` - การส่ง XON
- `fwd-mac-ctrl` - การส่งต่อเฟรม MAC control

**ตัวอย่างการใช้งาน:**
```bash
# ปิดใช้งานการเจรจาอัตโนมัติของทุกพอร์ต
set port all flowcontrol autoneg off

# เปิดใช้งานการควบคุมการไหลการส่งของทุกพอร์ต
set port all flowcontrol tx on
```

---

### **set port \<0-16\> dev-configure \<0-64\> \<0-64\>**

ตั้งค่าอุปกรณ์ DPDK ของพอร์ตที่ระบุ

**พารามิเตอร์:**
- อาร์กิวเมนต์ที่ 1: จำนวนคิวรับ (0-64)
- อาร์กิวเมนต์ที่ 2: จำนวนคิวส่ง (0-64)

**ตัวอย่างการใช้งาน:**
```bash
# ตั้งค่าพอร์ต 0 ด้วยคิวรับ 4 และคิวส่ง 4
set port 0 dev-configure 4 4

# ตั้งค่าพอร์ต 1 ด้วยคิวรับ 2 และคิวส่ง 2
set port 1 dev-configure 2 2
```

---

### **set port all dev-configure \<0-64\> \<0-64\>**

ตั้งค่าอุปกรณ์ DPDK ของทุกพอร์ต

**พารามิเตอร์:**
- อาร์กิวเมนต์ที่ 1: จำนวนคิวรับ (0-64)
- อาร์กิวเมนต์ที่ 2: จำนวนคิวส่ง (0-64)

**ตัวอย่างการใช้งาน:**
```bash
# ตั้งค่าทุกพอร์ตด้วยคิวรับ 1 และคิวส่ง 1
set port all dev-configure 1 1
```

---

### **set port \<0-16\> nrxdesc \<0-16384\>**

ตั้งค่าจำนวน descriptor การรับของพอร์ตที่ระบุ

**ตัวอย่างการใช้งาน:**
```bash
# ตั้งค่าจำนวน descriptor การรับของพอร์ต 0 เป็น 1024
set port 0 nrxdesc 1024

# ตั้งค่าจำนวน descriptor การรับของพอร์ต 1 เป็น 512
set port 1 nrxdesc 512
```

---

### **set port all nrxdesc \<0-16384\>**

ตั้งค่าจำนวน descriptor การรับของทุกพอร์ต

**ตัวอย่างการใช้งาน:**
```bash
# ตั้งค่าจำนวน descriptor การรับของทุกพอร์ตเป็น 512
set port all nrxdesc 512
```

---

### **set port \<0-16\> ntxdesc \<0-16384\>**

ตั้งค่าจำนวน descriptor การส่งของพอร์ตที่ระบุ

**ตัวอย่างการใช้งาน:**
```bash
# ตั้งค่าจำนวน descriptor การส่งของพอร์ต 0 เป็น 1024
set port 0 ntxdesc 1024

# ตั้งค่าจำนวน descriptor การส่งของพอร์ต 1 เป็น 512
set port 1 ntxdesc 512
```

---

### **set port all ntxdesc \<0-16384\>**

ตั้งค่าจำนวน descriptor การส่งของทุกพอร์ต

**ตัวอย่างการใช้งาน:**
```bash
# ตั้งค่าจำนวน descriptor การส่งของทุกพอร์ตเป็น 512
set port all ntxdesc 512
```

---

### **set port \<0-16\> link up**

เปิดลิงก์ของพอร์ตที่ระบุ

**ตัวอย่างการใช้งาน:**
```bash
# เปิดลิงก์ของพอร์ต 0
set port 0 link up

# เปิดลิงก์ของพอร์ต 1
set port 1 link up
```

---

### **set port \<0-16\> link down**

ปิดลิงก์ของพอร์ตที่ระบุ

**ตัวอย่างการใช้งาน:**
```bash
# ปิดลิงก์ของพอร์ต 0
set port 0 link down

# ปิดลิงก์ของพอร์ต 1
set port 1 link down
```

---

### **set port all link up**

เปิดลิงก์ของทุกพอร์ต

**ตัวอย่างการใช้งาน:**
```bash
# เปิดลิงก์ของทุกพอร์ต
set port all link up
```

---

### **set port all link down**

ปิดลิงก์ของทุกพอร์ต

**ตัวอย่างการใช้งาน:**
```bash
# ปิดลิงก์ของทุกพอร์ต
set port all link down
```

## ตำแหน่งการกำหนด

คำสั่งเหล่านี้ถูกกำหนดในไฟล์ต่อไปนี้:
- `sdplane/dpdk_port_cmd.c`

## รายการที่เกี่ยวข้อง

- [การจัดการ Worker และ lcore](worker-management.md)
- [ข้อมูลระบบและการตรวจสอบ](system-monitoring.md)
- [การตั้งค่าคิว](queue-configuration.md)