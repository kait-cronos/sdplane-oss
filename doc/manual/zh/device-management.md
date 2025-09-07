# 设备管理

**Language:** [English](../en/device-management.md) | [日本語](../ja/device-management.md) | [Français](../fr/device-management.md) | **中文** | [Deutsch](../de/device-management.md) | [Italiano](../it/device-management.md) | [한국어](../ko/device-management.md) | [ไทย](../th/device-management.md) | [Español](../es/device-management.md)

管理DPDK设备和驱动程序的命令。

## 命令一览

### show_devices - 设备信息显示
```
show devices
```

显示系统中可用设备的信息。

**使用例：**
```bash
show devices
```

该命令显示以下信息：
- 设备名
- 设备类型
- 当前驱动程序
- 设备状态
- PCI地址

### set_device_driver - 设备驱动程序设置
```
set device <WORD> driver (ixgbe|igb|igc|uio_pci_generic|igb_uio|vfio-pci|unbound) (|bind|driver_override)
```

为指定设备设置驱动程序。

**参数：**
- `<WORD>` - 设备名或PCI地址
- 驱动程序类型：
  - `ixgbe` - Intel 10GbE ixgbe驱动程序
  - `igb` - Intel 1GbE igb驱动程序
  - `igc` - Intel 2.5GbE igc驱动程序
  - `uio_pci_generic` - 通用UIO驱动程序
  - `igb_uio` - DPDK UIO驱动程序
  - `vfio-pci` - VFIO PCI驱动程序
  - `unbound` - 删除驱动程序
- 操作模式：
  - `bind` - 绑定驱动程序
  - `driver_override` - 驱动程序覆盖

**使用例：**
```bash
# 将设备绑定到vfio-pci驱动程序
set device 0000:01:00.0 driver vfio-pci bind

# 将设备设置为igb_uio驱动程序
set device eth0 driver igb_uio

# 从设备删除驱动程序
set device 0000:01:00.0 driver unbound
```

## 设备管理概要

### DPDK设备管理
DPDK使用专用驱动程序来高效使用网络设备。

### 驱动程序类型

#### 网络驱动程序
- **ixgbe** - Intel 10GbE网卡用
- **igb** - Intel 1GbE网卡用
- **igc** - Intel 2.5GbE网卡用

#### UIO（Userspace I/O）驱动程序
- **uio_pci_generic** - 通用UIO驱动程序
- **igb_uio** - DPDK专用UIO驱动程序

#### VFIO（Virtual Function I/O）驱动程序
- **vfio-pci** - 虚拟化环境中的高性能I/O

#### 特殊设置
- **unbound** - 删除驱动程序并禁用设备

## 设备配置步骤

### 基本配置步骤
1. **设备确认**
```bash
show devices
```

2. **驱动程序设置**
```bash
set device <device> driver <driver> bind
```

3. **配置确认**
```bash
show devices
```

4. **端口设置**
```bash
show port
```

### 配置例

#### Intel 10GbE卡配置
```bash
# 设备确认
show devices

# 绑定ixgbe驱动程序
set device 0000:01:00.0 driver ixgbe bind
set device 0000:01:00.1 driver ixgbe bind

# 配置确认
show devices
show port
```

#### DPDK UIO驱动程序配置
```bash
# 设备确认
show devices

# 绑定igb_uio驱动程序
set device 0000:02:00.0 driver igb_uio bind
set device 0000:02:00.1 driver igb_uio bind

# 配置确认
show devices
show port
```

#### VFIO配置（虚拟化环境）
```bash
# 设备确认
show devices

# 绑定vfio-pci驱动程序
set device 0000:03:00.0 driver vfio-pci bind
set device 0000:03:00.1 driver vfio-pci bind

# 配置确认
show devices
show port
```

## 驱动程序选择指导

### ixgbe（Intel 10GbE）
- **用途**: Intel 10GbE网卡
- **优点**: 高性能、稳定性
- **条件**: 需要Intel 10GbE卡

### igb（Intel 1GbE）
- **用途**: Intel 1GbE网卡
- **优点**: 广泛兼容性、稳定性
- **条件**: 需要Intel 1GbE卡

### igc（Intel 2.5GbE）
- **用途**: Intel 2.5GbE网卡
- **优点**: 中等性能、新标准
- **条件**: 需要Intel 2.5GbE卡

### uio_pci_generic
- **用途**: 通用设备
- **优点**: 广泛兼容性
- **缺点**: 部分功能限制

### igb_uio
- **用途**: DPDK专用环境
- **优点**: DPDK优化
- **缺点**: 需要单独安装

### vfio-pci
- **用途**: 虚拟化环境、重视安全性
- **优点**: 安全性、虚拟化支持
- **条件**: 需要启用IOMMU

## 故障排除

### 设备无法识别时
1. 设备确认
```bash
show devices
```

2. 系统级别确认
```bash
lspci | grep Ethernet
```

3. 内核模块确认
```bash
lsmod | grep uio
lsmod | grep vfio
```

### 驱动程序绑定失败时
1. 确认当前驱动程序
```bash
show devices
```

2. 删除现有驱动程序
```bash
set device <device> driver unbound
```

3. 绑定目标驱动程序
```bash
set device <device> driver <target_driver> bind
```

### 端口无法使用时
1. 确认设备状态
```bash
show devices
show port
```

2. 重新绑定驱动程序
```bash
set device <device> driver unbound
set device <device> driver <driver> bind
```

3. 确认端口配置
```bash
show port
update port status
```

## 高级功能

### 驱动程序覆盖
```bash
# 使用驱动程序覆盖
set device <device> driver <driver> driver_override
```

### 多设备批量设置
```bash
# 顺次设置多个设备
set device 0000:01:00.0 driver vfio-pci bind
set device 0000:01:00.1 driver vfio-pci bind
set device 0000:02:00.0 driver vfio-pci bind
set device 0000:02:00.1 driver vfio-pci bind
```

## 安全注意事项

### 使用VFIO时的注意点
- 需要启用IOMMU
- 安全组配置
- 适当的权限设置

### 使用UIO时的注意点
- 需要root权限
- 理解安全风险
- 适当的访问控制

## 系统集成

### 与systemd服务的协作
```bash
# 在systemd服务中自动设置
# 在/etc/systemd/system/sdplane.service中设置
```

### 启动时自动设置
```bash
# 在启动脚本中设置
# /etc/init.d/sdplane或systemd unit file
```

## 定义位置

这些命令在以下文件中定义：
- `sdplane/dpdk_devbind.c`

## 相关项目

- [端口管理·统计](port-management.md)
- [工作者·lcore管理](worker-lcore-thread-management.md)
- [系统信息·监视](system-monitoring.md)