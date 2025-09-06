# L2中继器应用

**语言 / Language:** [English](../l2-repeater-application.md) | [日本語](../ja/l2-repeater-application.md) | [Français](../fr/l2-repeater-application.md) | **中文** | [Deutsch](../de/l2-repeater-application.md) | [Italiano](../it/l2-repeater-application.md) | [한국어](../ko/l2-repeater-application.md) | [ไทย](../th/l2-repeater-application.md) | [Español](../es/l2-repeater-application.md)

L2中继器应用提供基本的端口间中继功能，在DPDK端口间提供简单的第二层包转发。

## 概要

L2中继器是一个直观的第二层转发应用，具有以下功能：
- DPDK端口间配对包转发（简单的端口间中继）
- 不进行MAC地址学习的基本包中继功能
- 可选的MAC地址更新功能（更改源MAC地址）
- 通过DPDK零拷贝包处理实现高性能运行

## 主要功能

### 第二层转发
- **端口间中继**: 预配置端口对间的简单包转发
- **无MAC学习**: 不构建转发表的直接包中继
- **透明转发**: 无论目标MAC如何都转发所有包
- **端口配对**: 固定的端口到端口转发配置

### 性能特性
- **零拷贝处理**: 使用DPDK高效的包处理
- **突发处理**: 为最佳吞吐量进行包突发处理
- **低延迟**: 最小处理开销实现高速转发
- **多核支持**: 在专用lcore上运行以实现扩展

## 配置

### 基本设置
L2中继器通过主sdplane配置系统进行配置：

```bash
# 将工作者类型设置为L2中继器
set worker lcore 1 l2-repeater

# 端口和队列配置
set thread 1 port 0 queue 0  
set thread 1 port 1 queue 0

# 启用混杂模式以进行学习
set port all promiscuous enable
```

### 配置文件示例
完整配置示例请参见[`example-config/sdplane_l2_repeater.conf`](../../example-config/sdplane_l2_repeater.conf)：

```bash
# 设备绑定
set device 02:00.0 driver vfio-pci bind
set device 03:00.0 driver vfio-pci bind

# DPDK初始化
set rte_eal argv -c 0x7
rte_eal_init

# 后台工作者（需要在队列配置前启动）
set worker lthread stat-collector
set worker lthread rib-manager
set worker lthread netlink-thread

set mempool

# 队列配置（必须在rib-manager启动后执行）
set thread 1 port 0 queue 0
set thread 1 port 1 queue 0

# 端口配置
stop port all
set port all dev-configure 1 4
set port all nrxdesc 1024
set port all ntxdesc 1024

# 工作者设置
set worker lcore 1 l2-repeater
set worker lcore 2 tap-handler
set port all promiscuous enable
start port all

# 等待端口启动
sleep 5

# 启动工作者
start worker lcore all
```

## 运行

### 广播转发
L2中继器将包转发到所有其他活动端口：
- **广播行为**: 将在端口接收的所有包转发到所有其他活动端口
- **分割水平**: 不将包发送回接收端口
- **无过滤**: 转发所有包类型（单播、广播、多播）

### 转发行为
- **全流量转发**: 无论目标MAC如何都中继所有包
- **全端口广播**: 将包转发到除输入端口外的所有活动端口
- **透明**: 不更改包内容（除非启用MAC更新）
- **多端口复制**: 为每个目标端口创建包副本

### MAC地址更新
启用后，L2中继器可以更改包的MAC地址：
- **源MAC更新**: 将源MAC更改为输出端口的MAC
- **透明桥接**: 保持原始MAC地址（默认）

## 性能调优

### 缓冲区配置
```bash
# 针对工作负载优化描述符数量
set port all nrxdesc 2048  # 为高包率增加
set port all ntxdesc 2048  # 为缓冲增加
```

### 工作者分配
```bash
# 为L2转发专用特定lcore
set worker lcore 1 l2-repeater  # 分配到专用核心
set worker lcore 2 tap-handler  # 分离TAP处理
```

### 内存池大小调整
内存池应根据预期流量适当调整：
- 考虑包率和缓冲需求
- 考虑突发大小和临时包存储

## 监视和调试

### 端口统计
```bash
# 显示转发统计
show port statistics all

# 监视特定端口
show port statistics 0
show port statistics 1
```

### 调试命令
```bash
# 启用L2中继器调试
debug sdplane l2-repeater

# VLAN交换调试（替代工作者类型）
debug sdplane vlan-switch

# 通用sdplane调试
debug sdplane fdb-change
debug sdplane rib
```

## 使用案例

### 集线器型中继器
- 将流量复制到所有连接的端口
- 透明第二层中继
- 无学习功能的基本集线器功能

### 端口镜像/中继
- 端口间流量镜像
- 网络监视和分析
- 简单的包复制

### 性能测试
- 测量转发性能
- L2转发基准的基线
- 验证DPDK端口配置

## 限制

- **无VLAN处理**: 无VLAN识别的简单L2中继
- **无MAC学习**: 无地址学习的全端口广播转发
- **无STP支持**: 无生成树协议实现
- **无过滤**: 无论目标如何都转发所有包

## 相关应用

- **增强中继器**: 支持VLAN和TAP接口的高级版本
- **L3转发**: 第三层路由功能
- **VLAN交换**: 支持VLAN的交换功能

有关包括VLAN支持的更高级第二层功能，请参见[增强中继器](enhanced-repeater.md)文档。