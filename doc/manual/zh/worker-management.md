# 工作者·lcore管理

**语言 / Language:** [English](../worker-lcore-thread-management.md) | [日本語](../ja/worker-lcore-thread-management.md) | [Français](../fr/worker-lcore-thread-management.md) | **中文** | [Deutsch](../de/worker-lcore-thread-management.md) | [Italiano](../it/worker-lcore-thread-management.md) | [한국어](../ko/worker-lcore-thread-management.md) | [ไทย](../th/worker-lcore-thread-management.md) | [Español](../es/worker-lcore-thread-management.md)

管理DPDK工作者线程和lcore的命令。

## 命令一览

### set_worker - 工作者类型设置
```
set worker lcore <0-16> (|none|l2fwd|l3fwd|l3fwd-lpm|tap-handler|l2-repeater|enhanced-repeater|vlan-switch|pktgen|linkflap-generator)
```

为指定的lcore设置工作者类型。

**工作者类型：**
- `none` - 无工作者
- `l2fwd` - Layer 2转发
- `l3fwd` - Layer 3转发
- `l3fwd-lpm` - Layer 3转发 (LPM)
- `tap-handler` - TAP接口处理器
- `l2-repeater` - Layer 2中继器
- `enhanced-repeater` - 具有VLAN交换和TAP接口的增强中继器
- `vlan-switch` - VLAN交换机
- `pktgen` - 包生成器
- `linkflap-generator` - 链路翻转生成器

**使用例：**
```bash
# 为lcore 1设置L2转发工作者
set worker lcore 1 l2fwd

# 为lcore 1设置增强中继器工作者
set worker lcore 1 enhanced-repeater

# 为lcore 2设置无工作者
set worker lcore 2 none

# 为lcore 3设置L3转发(LPM)工作者
set worker lcore 3 l3fwd-lpm
```

### reset_worker - 工作者重置
```
reset worker lcore <0-16>
```

重置指定lcore的工作者。

**使用例：**
```bash
# 重置lcore 2的工作者
reset worker lcore 2
```

### start_worker - 工作者启动
```
start worker lcore <0-16>
```

启动指定lcore的工作者。

**使用例：**
```bash
# 启动lcore 1的工作者
start worker lcore 1
```

### restart_worker - 工作者重新启动
```
restart worker lcore <0-16>
```

重新启动指定lcore的工作者。

**使用例：**
```bash
# 重新启动lcore 4的工作者
restart worker lcore 4
```

### start_worker_all - 工作者启动（all选项）
```
start worker lcore (<0-16>|all)
```

启动指定lcore或所有lcore的工作者。

**使用例：**
```bash
# 启动lcore 1的工作者
start worker lcore 1

# 启动所有工作者
start worker lcore all
```

### stop_worker - 工作者停止
```
stop worker lcore (<0-16>|all)
```

停止指定lcore或所有lcore的工作者。

**使用例：**
```bash
# 停止lcore 1的工作者
stop worker lcore 1

# 停止所有工作者
stop worker lcore all
```

### reset_worker_all - 工作者重置（all选项）
```
reset worker lcore (<0-16>|all)
```

重置指定lcore或所有lcore的工作者。

**使用例：**
```bash
# 重置lcore 2的工作者
reset worker lcore 2

# 重置所有工作者
reset worker lcore all
```

### restart_worker_all - 工作者重新启动（all选项）
```
restart worker lcore (<0-16>|all)
```

重新启动指定lcore或所有lcore的工作者。

**使用例：**
```bash
# 重新启动lcore 3的工作者
restart worker lcore 3

# 重新启动所有工作者
restart worker lcore all
```

### show_worker - 工作者信息显示
```
show worker
```

显示当前工作者的状态和配置。

**使用例：**
```bash
show worker
```

### set_mempool - 内存池设置
```
set mempool
```

设置DPDK内存池。

**使用例：**
```bash
set mempool
```

### set_rte_eal_argv - RTE EAL命令行参数设置
```
set rte_eal argv <WORD> <WORD> <WORD> <WORD> <WORD> <WORD>
```

设置用于RTE EAL（Environment Abstraction Layer）初始化的命令行参数。

**使用例：**
```bash
# 设置EAL参数
set rte_eal argv -c 0x1 -n 4 --socket-mem 1024,1024 --huge-dir /mnt/huge
```

### rte_eal_init - RTE EAL初始化
```
rte_eal_init
```

初始化RTE EAL（Environment Abstraction Layer）。

**使用例：**
```bash
rte_eal_init
```

## 工作者类型说明

### L2转发 (l2fwd)
在Layer 2级别执行包转发的工作者。基于MAC地址转发包。

### L3转发 (l3fwd)
在Layer 3级别执行包转发的工作者。基于IP地址执行路由。

### L3转发 LPM (l3fwd-lpm)
使用Longest Prefix Matching（LPM）的Layer 3转发工作者。

### TAP处理器 (tap-handler)
在TAP接口和DPDK端口之间执行包转发的工作者。

### L2中继器 (l2-repeater)
在Layer 2级别执行包复制·中继的工作者。


### VLAN交换机 (vlan-switch)
提供VLAN（Virtual LAN）功能的交换工作者。

### 包生成器 (pktgen)
生成测试包的工作者。

### 链路翻转生成器 (linkflap-generator)
用于测试网络链路状态变化的工作者。

## 定义位置

这些命令在以下文件中定义：
- `sdplane/dpdk_lcore_cmd.c`

## 相关项目

- [端口管理·统计](port-management.md)
- [线程信息](worker-lcore-thread-management.md)
- [lthread管理](lthread-management.md)