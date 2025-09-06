# การดีบักและการบันทึก

**Language:** [English](../debug-logging.md) | [日本語](../ja/debug-logging.md) | [Français](../fr/debug-logging.md) | [中文](../zh/debug-logging.md) | [Deutsch](../de/debug-logging.md) | [Italiano](../it/debug-logging.md) | [한국어](../ko/debug-logging.md) | [ไทย](../th/debug-logging.md) | [Español](../es/debug-logging.md)

คำสั่งสำหรับควบคุมฟังก์ชันการดีบักและการบันทึกของ sdplane

## รายการคำสั่ง

### log_file - การกำหนดค่าเอาต์พุตไฟล์บันทึก
```
log file <file-path>
```

กำหนดค่าการบันทึกเพื่อเอาต์พุตไปยังไฟล์

**พารามิเตอร์:**
- <เส้นทาง-ไฟล์> - เส้นทางไปยังไฟล์เอาต์พุตบันทึก

**ตัวอย่าง:**
```bash
# เอาต์พุตบันทึกไปยังไฟล์ที่ระบุ
log file /var/log/sdplane.log

# ไฟล์บันทึกดีบัก
log file /tmp/sdplane-debug.log
```

### log_stdout - การกำหนดค่าบันทึกเอาต์พุตมาตรฐาน
```
log stdout
```

กำหนดค่าการบันทึกสำหรับเอาต์พุตไปยังเอาต์พุตมาตรฐาน

**ตัวอย่าง:**
```bash
# แสดงบันทึกบนเอาต์พุตมาตรฐาน
log stdout
```

**หมายเหตุ:** สามารถกำหนดค่า `log file` และ `log stdout` พร้อมกันได้ และบันทึกจะถูกส่งไปยังทั้งสองปลายทาง

### debug - การกำหนดค่าดีบัก
```
debug <category> <target>
```

เปิดใช้งานการบันทึกดีบักสำหรับเป้าหมายเฉพาะในหมวดหมู่ที่ระบุ

**หมวดหมู่:**
- `sdplane` - หมวดหมู่ sdplane หลัก
- `zcmdsh` - หมวดหมู่คำสั่งเชลล์

**รายการเป้าหมาย sdplane:**
- `lthread` - เธรดน้ำหนักเบา
- `console` - คอนโซล
- `tap-handler` - ตัวจัดการ TAP
- `l2fwd` - การส่งต่อ L2
- `l3fwd` - การส่งต่อ L3
- `vty-server` - เซิร์ฟเวอร์ VTY
- `vty-shell` - เชลล์ VTY
- `stat-collector` - ตัวรวบรวมสถิติ
- `packet` - การประมวลผลแพ็กเก็ต
- `fdb` - FDB (ฐานข้อมูลการส่งต่อ)
- `fdb-change` - การเปลี่ยนแปลง FDB
- `rib` - RIB (ฐานข้อมูลข้อมูลเส้นทาง)
- `vswitch` - สวิตช์เสมือน
- `vlan-switch` - สวิตช์ VLAN
- `pktgen` - ตัวสร้างแพ็กเก็ต
- `enhanced-repeater` - รีพีตเตอร์ขั้นสูง
- `netlink` - อินเทอร์เฟซ Netlink
- `neighbor` - การจัดการเพื่อนบ้าน
- `all` - เป้าหมายทั้งหมด

**ตัวอย่าง:**
```bash
# เปิดใช้งานดีบักสำหรับเป้าหมายเฉพาะ
debug sdplane rib
debug sdplane fdb-change
debug sdplane pktgen

# เปิดใช้งานดีบัก sdplane ทั้งหมด
debug sdplane all

# ดีบักหมวดหมู่ zcmdsh
debug zcmdsh shell
debug zcmdsh command
```

### no debug - ปิดใช้งานดีบัก
```
no debug <category> <target>
```

ปิดใช้งานการบันทึกดีบักสำหรับเป้าหมายเฉพาะในหมวดหมู่ที่ระบุ

**ตัวอย่าง:**
```bash
# ปิดใช้งานดีบักสำหรับเป้าหมายเฉพาะ
no debug sdplane rib
no debug sdplane fdb-change

# ปิดใช้งานดีบัก sdplane ทั้งหมด (แนะนำ)
no debug sdplane all

# ปิดใช้งานดีบักหมวดหมู่ zcmdsh
no debug zcmdsh all
```

### show_debug_sdplane - Display sdplane Debug Information
```
show debugging sdplane
```

Display current sdplane debug configuration.

**Examples:**
```bash
show debugging sdplane
```

This command displays the following information:
- Currently enabled debug targets
- Debug status for each target
- Available debug options

## Debug System Overview

The sdplane debug system has the following features:

### Category-based Debugging
- Debug categories are separated by different functional modules
- You can enable debug logs only for the necessary functions

### Target-based Control
- Debug messages are classified by target type
- You can display only necessary information by setting appropriate targets

### Dynamic Configuration
- Debug configuration can be changed while the system is running
- Debug targets can be adjusted without restart

## Usage

### 1. Configure Log Output
```bash
# Configure log file output (recommended)
log file /var/log/sdplane.log

# Configure standard output
log stdout

# Enable both (convenient for debugging)
log file /var/log/sdplane.log
log stdout
```

### 2. Check Current Debug Configuration
```bash
show debugging sdplane
```

### 3. Check Debug Targets
Use the `show debugging sdplane` command to check available targets and their status.

### 4. Change Debug Configuration
```bash
# Enable debug for specific targets
debug sdplane rib
debug sdplane fdb-change

# Enable all targets at once
debug sdplane all
```

### 5. Check Debug Logs
Debug logs are output to the configured destinations (file or standard output).

## Troubleshooting

### When Debug Logs Are Not Output
1. Check if log output is configured (`log file` or `log stdout`)
2. Check if debug targets are correctly configured (`debug sdplane <target>`)
3. Check current debug status (`show debugging sdplane`)
4. Check log file disk space and permissions

### Log File Management
```bash
# Check log file size
ls -lh /var/log/sdplane.log

# Tail log file
tail -f /var/log/sdplane.log

# Check log file location (configuration file example)
grep "log file" /etc/sdplane/sdplane.conf
```

### Performance Impact
- Enabling debug logs may impact performance
- It is recommended to enable only minimal debugging in production environments
- Regularly rotate log files to prevent them from becoming too large

## Configuration Examples

### Basic Log Configuration
```bash
# Configuration file example (/etc/sdplane/sdplane.conf)
log file /var/log/sdplane.log
log stdout

# Enable debug at system startup
debug sdplane rib
debug sdplane fdb-change
```

### Debugging Configuration
```bash
# Detailed debug log configuration
log file /tmp/sdplane-debug.log
log stdout

# Enable all target debugging (development only)
debug sdplane all

# Enable specific targets only
debug sdplane rib
debug sdplane fdb-change
debug sdplane vswitch
```

### Production Environment Configuration
```bash
# Standard logging only in production
log file /var/log/sdplane.log

# Enable only critical targets as needed
# debug sdplane fdb-change
# debug sdplane rib
```

### Debug Cleanup Operations
```bash
# Disable all debugging
no debug sdplane all
no debug zcmdsh all
```

## Definition Location

These commands are defined in the following file:
- `sdplane/debug_sdplane.c`

## Related Topics

- [System Information & Monitoring](system-monitoring.md)
- [VTY & Shell Management](vty-shell.md)
