# sdplane-oss 用户指南

**语言 / Language:** [English](../README.md) | [日本語](../ja/README.md) | [Français](../fr/README.md) | **中文** | [Deutsch](../de/README.md) | [Italiano](../it/README.md) | [한국어](../ko/README.md) | [ไทย](../th/README.md) | [Español](../es/README.md)

sdplane-oss是一个基于DPDK的高性能软件路由器。本用户指南介绍了sdplane的所有命令和功能。

## 目录

1. [端口管理·统计](port-management.md) - DPDK端口的管理和统计信息
2. [工作线程·lcore管理](worker-lcore-thread-management.md) - 工作线程和lcore的管理
3. [调试·日志](debug-logging.md) - 调试和日志功能
4. [VTY·shell管理](vty-shell.md) - VTY和shell的管理
5. [系统信息·监视](system-monitoring.md) - 系统信息和监视功能
6. [RIB·路由](routing.md) - RIB和路由功能
7. [队列配置](queue-configuration.md) - 队列的配置和管理
8. [数据包生成](packet-generation.md) - 使用PKTGEN进行数据包生成
9. [线程信息](worker-lcore-thread-management.md) - 线程的信息和监视
10. [TAP接口](tap-interface.md) - TAP接口的管理
11. [lthread管理](lthread-management.md) - lthread的管理
12. [设备管理](device-management.md) - 设备和驱动程序的管理

## 基本使用方法

### 连接方法

要连接到sdplane：

```bash
# 启动sdplane
sudo ./sdplane/sdplane

# 从另一个终端连接到CLI
telnet localhost 9882
```

### 帮助显示

每个命令都可以使用`?`来显示帮助：

```
sdplane# ?
sdplane# show ?
sdplane# set ?
```

### 基本命令

- `show version` - 显示版本信息
- `show port` - 显示端口信息
- `show worker` - 显示工作线程信息
- `exit` - 退出CLI

## 命令分类

sdplane定义了79个命令，分为以下13个功能类别：

1. **端口管理·统计** (10个命令) - DPDK端口的控制和统计
2. **工作线程·lcore管理** (6个命令) - 工作线程和lcore的管理
3. **调试·日志** (2个命令) - 调试和日志功能
4. **VTY·shell管理** (4个命令) - VTY和shell的控制
5. **系统信息·监视** (10个命令) - 系统信息和监视
6. **RIB·路由** (1个命令) - 路由信息的管理
7. **队列配置** (3个命令) - 队列的配置
8. **数据包生成** (3个命令) - 通过PKTGEN进行数据包生成
9. **线程信息** (2个命令) - 线程的监视
10. **TAP接口** (2个命令) - TAP接口的管理
11. **lthread管理** (3个命令) - lthread的管理
12. **设备管理** (2个命令) - 设备和驱动程序的管理

有关详细使用方法，请参考各类别的文档。

## 应用程序

### L2中继器应用程序
使用MAC学习的简单第二层数据包传输：
- [L2中继器应用程序指南](l2-repeater-application.md)

### 增强中继器应用程序
带TAP接口的高级VLAN交换：
- [增强中继器应用程序指南](enhanced-repeater-application.md)

### 数据包生成器应用程序
高性能流量生成和测试：
- [数据包生成器应用程序指南](packet-generator-application.md)

## 配置

### 基本配置

1. **系统配置**：
   ```bash
   # 配置大页面
   echo 1024 > /sys/kernel/mm/hugepages/hugepages-2048kB/nr_hugepages
   
   # 将网卡绑定到DPDK驱动程序
   sudo modprobe vfio-pci
   echo 0000:03:00.0 > /sys/bus/pci/drivers/virtio-pci/unbind
   echo vfio-pci > /sys/bus/pci/devices/0000:03:00.0/driver_override
   echo 0000:03:00.0 > /sys/bus/pci/drivers/vfio-pci/bind
   ```

2. **sdplane配置**：
   ```bash
   # 基本配置示例
   set rte_eal argv -c 0x7
   rte_eal_init
   
   set worker lthread stat-collector
   set worker lthread rib-manager
   set worker lcore 1 enhanced-repeater
   
   set port all dev-configure 1 4
   set port all promiscuous enable
   start port all
   start worker lcore all
   ```

### 详细配置指南

- [增强中继器](enhanced-repeater.md) - 增强中继器配置
- [端口管理](port-management.md) - DPDK端口配置
- [工作线程管理](worker-lcore-thread-management.md) - 工作线程和lcore配置
- [设备管理](device-management.md) - 设备和驱动程序配置

## 监视和调试

### 系统监视
- [系统信息·监视](system-monitoring.md) - 性能监视和系统状态
- [线程信息](worker-lcore-thread-management.md) - 线程监视和性能

### 调试工具
- [调试·日志](debug-logging.md) - 调试工具和日志
- [VTY·shell管理](vty-shell.md) - CLI界面和会话管理

## 开发和集成

### 开发人员指南
- [DPDK集成指南](dpdk-integration-guide.md) - 自定义DPDK应用程序集成
- [队列配置](queue-configuration.md) - RX/TX队列的高级配置
- [lthread管理](lthread-management.md) - 系统服务的协作线程

### 接口和虚拟化
- [TAP接口](tap-interface.md) - 通过TAP与Linux内核集成
- [数据包生成](packet-generation.md) - 测试和基准测试工具

## 常用示例

### L2中继器配置
```bash
# L2中继器的最小配置
set rte_eal argv -c 0x3
rte_eal_init
set worker lcore 1 l2-repeater
set port all dev-configure 1 1
start port all
start worker lcore all
```

### 带TAP的增强中继器配置
```bash
# 带TAP的增强中继器配置
set rte_eal argv -c 0x7
rte_eal_init
set worker lthread stat-collector
set worker lthread rib-manager  
set worker lcore 1 enhanced-repeater
set worker lcore 2 l3-tap-handler
set port all dev-configure 1 4
start port all
start worker lcore all
```

### PKTGEN性能测试
```bash
# 数据包生成器配置
set rte_eal argv -c 0xf
pktgen init
set worker lcore 1 pktgen
set worker lcore 2 pktgen
set port all dev-configure 1 4
start port all
start worker lcore all

# 用于测试的PKTGEN配置
pktgen do set port 0 count 0
pktgen do set port 0 size 64
pktgen do set port 0 rate 100
pktgen do start port 0
```

## 故障排除

### 常见问题

1. **sdplane无法启动**：
   ```bash
   # 检查大页面
   cat /proc/meminfo | grep HugePages
   
   # 检查权限
   sudo ./sdplane/sdplane
   ```

2. **未检测到端口**：
   ```bash
   # 检查驱动程序绑定
   dpdk-devbind.py -s
   
   # 检查DPDK配置
   show port all
   ```

3. **性能低下**：
   ```bash
   # 监视统计信息
   show port statistics all
   show thread counter
   
   # 检查工作线程配置
   show worker
   ```

### 诊断
```bash
# 系统信息
show system
show version
show memory

# 组件状态
show port all
show worker  
show thread information

# 详细调试
debug sdplane worker
debug sdplane port
```

## 其他资源

- **DPDK文档**：https://doc.dpdk.org/
- **GitHub仓库**：https://github.com/kait-cronos/sdplane-oss
- **问题和支持**：https://github.com/kait-cronos/sdplane-oss/issues

对于技术支持或特定问题，请参考上面列出的详细指南或在GitHub上创建issue。