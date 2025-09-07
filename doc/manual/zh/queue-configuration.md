# 队列配置

**语言 / Language:** [English](../en/queue-configuration.md) | [日本語](../ja/queue-configuration.md) | [Français](../fr/queue-configuration.md) | **中文** | [Deutsch](../de/queue-configuration.md) | [Italiano](../it/queue-configuration.md) | [한국어](../ko/queue-configuration.md) | [ไทย](../th/queue-configuration.md) | [Español](../es/queue-configuration.md)

配置和管理DPDK队列的命令。

## 命令一览

### update_port_status - 端口状态更新
```
update port status
```

更新所有端口的状态。

**使用例：**
```bash
update port status
```

该命令执行以下操作：
- 确认各端口的链路状态
- 更新队列配置
- 更新端口的统计信息

### set_thread_lcore_port_queue - 线程队列配置
```
set thread <0-128> port <0-128> queue <0-128>
```

为指定线程分配端口和队列。

**参数：**
- `<0-128>` (thread) - 线程号
- `<0-128>` (port) - 端口号
- `<0-128>` (queue) - 队列号

**使用例：**
```bash
# 为线程0分配端口0的队列0
set thread 0 port 0 queue 0

# 为线程1分配端口1的队列1
set thread 1 port 1 queue 1

# 为线程2分配端口0的队列1
set thread 2 port 0 queue 1
```

### show_thread_qconf - 线程队列配置显示
```
show thread qconf
```

显示当前的线程队列配置。

**使用例：**
```bash
show thread qconf
```

## 队列系统概要

### DPDK队列概念
在DPDK中，可以为每个端口设置多个发送队列和接收队列：

- **接收队列（RX Queue）** - 接收入站包
- **发送队列（TX Queue）** - 发送出站包
- **多队列** - 并行处理多个队列

### 队列配置的重要性
通过适当的队列配置可实现以下功能：
- **性能提升** - 通过并行处理实现高速化
- **负载分散** - 在多个工作者中分散处理
- **CPU效率** - 有效利用CPU核心

## 队列配置方法

### 基本配置步骤
1. **端口状态更新**
```bash
update port status
```

2. **线程队列配置**
```bash
set thread 0 port 0 queue 0
set thread 1 port 0 queue 1
```

3. **配置确认**
```bash
show thread qconf
```

### 推荐配置模式

#### 单端口，单队列
```bash
set thread 0 port 0 queue 0
```

#### 单端口，多队列
```bash
set thread 0 port 0 queue 0
set thread 1 port 0 queue 1
set thread 2 port 0 queue 2
```

#### 多端口，多队列
```bash
set thread 0 port 0 queue 0
set thread 1 port 1 queue 0
set thread 2 port 2 queue 0
```

## 配置例

### 高性能配置（4核心，4端口）
```bash
# 端口状态更新
update port status

# 为各核心分配不同端口
set thread 0 port 0 queue 0
set thread 1 port 1 queue 0
set thread 2 port 2 queue 0
set thread 3 port 3 queue 0

# 配置确认
show thread qconf
```

### 负载分散配置（2核心，1端口）
```bash
# 端口状态更新
update port status

# 为1个端口设置多个队列
set thread 0 port 0 queue 0
set thread 1 port 0 queue 1

# 配置确认
show thread qconf
```

## 性能调优

### 队列数决定
- **CPU核心数** - 根据可用的CPU核心数进行设置
- **端口数** - 考虑物理端口数
- **流量特性** - 预期的流量模式

### 优化要点
1. **CPU亲和性** - CPU核心和队列的适当配置
2. **内存配置** - 考虑NUMA节点的内存配置
3. **中断处理** - 高效的中断处理

## 故障排除

### 队列配置未反映时
1. 更新端口状态
```bash
update port status
```

2. 确认工作者状态
```bash
show worker
```

3. 确认端口状态
```bash
show port
```

### 性能未提升时
1. 确认队列配置
```bash
show thread qconf
```

2. 确认线程负载
```bash
show thread counter
```

3. 确认端口统计
```bash
show port statistics
```

## 定义位置

这些命令在以下文件中定义：
- `sdplane/queue_config.c`

## 相关项目

- [端口管理·统计](port-management.md)
- [工作者·lcore管理](worker-lcore-thread-management.md)
- [线程信息](worker-lcore-thread-management.md)