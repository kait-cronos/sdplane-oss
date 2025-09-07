# 增强中继器应用

**Language:** [English](../en/enhanced-repeater-application.md) | [日本語](../ja/enhanced-repeater-application.md) | [Français](../fr/enhanced-repeater-application.md) | **中文** | [Deutsch](../de/enhanced-repeater-application.md) | [Italiano](../it/enhanced-repeater-application.md) | [한국어](../ko/enhanced-repeater-application.md) | [ไทย](../th/enhanced-repeater-application.md) | [Español](../es/enhanced-repeater-application.md)

增强中继器应用提供高级VLAN感知第二层交换，集成TAP接口用于L3路由和包捕获功能。

## 概要

增强中继器是一个复杂的第二层转发应用，通过以下功能扩展基本L2转发：
- **VLAN感知交换**: 完全支持IEEE 802.1Q VLAN（标记/去标记）
- **虚拟交换抽象**: 具有独立转发表的多个虚拟交换机
- **TAP接口集成**: 用于内核集成的路由器接口和捕获接口
- **高级VLAN处理**: VLAN转换、插入、删除功能

## 架构

### 虚拟交换框架
增强中继器实现虚拟交换架构：
- **多VLAN**: 支持多个VLAN域（1-4094）
- **端口聚合**: 每个虚拟交换机多个物理端口
- **隔离转发**: 每个VLAN独立的转发域
- **灵活标记**: 每端口原生、标记、转换模式

### TAP接口集成
- **路由器接口**: 用于L3处理的内核网络栈集成
- **捕获接口**: 包监视和分析功能
- **环形缓冲区**: 数据平面和内核间的高效包传输
- **双向**: 输入输出双向包处理

## 主要功能

### VLAN处理
- **VLAN标记**: 向无标记帧添加802.1Q头
- **VLAN去标记**: 从标记帧移除802.1Q头
- **VLAN转换**: 在输入输出间更改VLAN ID
- **原生VLAN**: 在干线端口处理无标记流量

### 虚拟交换
- **学习**: 每VLAN自动MAC地址学习
- **泛洪**: 对未知单播和广播的适当处理
- **分割水平**: 虚拟交换机内防止环路
- **多域**: 每VLAN独立转发表

### 包处理
- **零拷贝**: 最小开销的高效DPDK包处理
- **突发处理**: 针对高包率优化
- **头部操作**: 高效VLAN头插入/删除
- **拷贝优化**: 为TAP接口选择性包拷贝

## 配置

### 虚拟交换设置
创建用于不同VLAN域的虚拟交换机：

```bash
# 创建具有VLAN ID的虚拟交换机
set vswitch 2031
set vswitch 2032
```

### 端口到虚拟交换链接
将物理DPDK端口链接到虚拟交换机：

```bash
# 将端口0链接到虚拟交换机0，VLAN标记2031
set vswitch-link vswitch 0 port 0 tag 2031

# 将端口0链接到虚拟交换机1，原生/无标记
set vswitch-link vswitch 1 port 0 tag 0

# VLAN转换示例
set vswitch-link vswitch 0 port 1 tag 2040
```

### 路由器接口
创建用于L3连接的路由器接口：

```bash
# 创建用于L3处理的路由器接口
set vswitch 2031 router-if rif2031
set vswitch 2032 router-if rif2032
```

### 捕获接口
设置用于监视的捕获接口：

```bash
# 创建用于包监视的捕获接口
set vswitch 2031 capture-if cif2031
set vswitch 2032 capture-if cif2032
```

### 工作者配置
将增强中继器分配给lcore：

```bash
# 设置增强中继器工作者
set worker lcore 1 enhanced-repeater

# 设置队列分配
set thread 1 port 0 queue 0
```

## 配置示例

### 完整设置
请参见[`example-config/sdplane_enhanced_repeater.conf`](../../example-config/sdplane_enhanced_repeater.conf)：

```bash
# 设备设置
set device 03:00.0 driver vfio-pci bind

# DPDK初始化
set rte_eal argv -c 0x7
rte_eal_init
set mempool

# 端口配置
stop port all
set port all dev-configure 1 4
set port all nrxdesc 1024
set port all ntxdesc 1024

# 工作者设置
set worker lthread stat-collector
set worker lthread rib-manager
set worker lcore 1 enhanced-repeater
set worker lcore 2 l3-tap-handler

# 启用混杂模式
set port all promiscuous enable
start port all

# 队列配置
set thread 1 port 0 queue 0

# 虚拟交换设置
set vswitch 2031
set vswitch 2032

# 链接配置
set vswitch-link vswitch 0 port 0 tag 2031
set vswitch-link vswitch 1 port 0 tag 2032

# 接口创建
set vswitch 2031 router-if rif2031
set vswitch 2032 router-if rif2032
set vswitch 2031 capture-if cif2031
set vswitch 2032 capture-if cif2032

# 启动工作者
start worker lcore all
```

## VLAN处理详情

### 标记模式

#### 标记模式
- **输入**: 接受具有特定VLAN标记的包
- **输出**: 保持或转换VLAN标记
- **用途**: 干线端口、VLAN转换

#### 原生模式（tag 0）
- **输入**: 接受无标记包
- **输出**: 删除VLAN头
- **用途**: 接入端口、非VLAN感知设备

#### 转换模式
- **功能**: 在端口间更改VLAN ID
- **配置**: 输入输出链接的不同标记
- **用途**: VLAN映射、服务提供商网络

### 包流

1. **输入处理**:
   - DPDK端口包接收
   - 基于标记或原生配置的VLAN判定
   - 目标虚拟交换机查找

2. **虚拟交换查找**:
   - MAC地址学习和查找
   - VLAN域内输出端口确定
   - 未知单播/广播泛洪处理

3. **输出处理**:
   - 每端口配置的VLAN头操作
   - 多目标包拷贝
   - TAP接口集成

4. **TAP接口处理**:
   - 路由器接口: 内核L3处理
   - 捕获接口: 监视和分析

## 监视和管理

### 状态命令
```bash
# 显示虚拟交换配置
show vswitch_rib

# 显示虚拟交换链接
show vswitch-link

# 显示路由器接口
show rib vswitch router-if

# 检查捕获接口
show rib vswitch capture-if
```

### 统计和性能
```bash
# 端口级统计
show port statistics all

# 工作者性能
show worker statistics

# 线程信息
show thread information
```

### 调试命令
```bash
# 增强中继器调试
debug sdplane enhanced-repeater

# VLAN交换调试（替代工作者类型）
debug sdplane vlan-switch

# RIB和转发调试
debug sdplane rib
debug sdplane fdb-change
```

## 使用案例

### VLAN聚合
- 单物理链路上多VLAN整合
- 提供商边缘功能
- 服务复用

### L3集成
- Router-on-a-stick配置
- 通过TAP接口进行VLAN间路由
- 混合L2/L3转发

### 网络监视
- 每VLAN包捕获
- 流量分析和调试
- 服务监视

### 服务提供商网络
- 客户隔离的VLAN转换
- 多租户网络
- 流量工程

## 性能考虑

### 扩展
- **工作者分配**: 为最佳性能专用lcore
- **队列配置**: 跨核心平衡队列分配
- **内存池**: 根据包率和缓冲需求适当调整

### VLAN处理开销
- **头部操作**: VLAN操作的最小开销
- **包拷贝**: 仅在需要时选择性拷贝
- **突发优化**: 为效率以突发单位处理包

### TAP接口性能
- **环形缓冲区调整**: 设置适当的环形大小
- **选择性传输**: 仅发送相关包到TAP接口
- **内核集成**: 考虑内核处理开销

## 故障排除

### 常见问题
- **VLAN不匹配**: 确认标记配置与网络设置匹配
- **TAP接口创建**: 检查适当权限和内核支持
- **性能问题**: 检查队列分配和工作者分布

### 调试策略
- **启用调试日志**: 使用调试命令获取详细包流
- **监视统计**: 监视端口和工作者统计
- **包捕获**: 使用捕获接口进行流量分析

## 相关文档

- [增强中继器命令](enhanced-repeater.md) - 完整命令参考
- [工作者管理](worker-lcore-thread-management.md) - 工作者配置详情
- [端口管理](port-management.md) - DPDK端口设置
- [TAP接口管理](tap-interface.md) - TAP接口详情