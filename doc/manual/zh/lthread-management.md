# lthread管理

**语言 / Language:** [English](../en/lthread-management.md) | [日本語](../ja/lthread-management.md) | [Français](../fr/lthread-management.md) | **中文** | [Deutsch](../de/lthread-management.md) | [Italiano](../it/lthread-management.md) | [한국어](../ko/lthread-management.md) | [ไทย](../th/lthread-management.md) | [Español](../es/lthread-management.md)

管理lthread（轻量线程）的命令。

## 命令一览

### set_worker_lthread_stat_collector - lthread统计收集器设置
```
set worker lthread stat-collector
```

设置收集统计信息的lthread工作者。

**使用例：**
```bash
set worker lthread stat-collector
```

### set_worker_lthread_rib_manager - lthread RIB管理器设置
```
set worker lthread rib-manager
```

设置管理RIB（Routing Information Base）的lthread工作者。

**使用例：**
```bash
set worker lthread rib-manager
```

### set_worker_lthread_netlink_thread - lthread netlink线程设置
```
set worker lthread netlink-thread
```

设置处理Netlink通信的lthread工作者。

**使用例：**
```bash
set worker lthread netlink-thread
```

## lthread概要

### lthread是什么
lthread（lightweight thread）是实现协作多线程的轻量线程实现。

### 主要特点
- **轻量性** - 低内存开销
- **高速性** - 高速上下文切换
- **协作性** - 通过显式yield进行控制
- **可扩展性** - 高效管理大量线程

### 与传统线程的区别
- **抢占式 vs 协作式** - 显式控制权让渡
- **内核线程 vs 用户线程** - 不通过内核执行
- **重 vs 轻** - 创建·切换开销小

## lthread工作者类型

### 统计收集器 (stat-collector)
定期收集系统统计信息的工作者。

**功能：**
- 端口统计收集
- 线程统计收集
- 系统统计收集
- 统计数据汇总

**使用例：**
```bash
# 设置统计收集器
set worker lthread stat-collector

# 确认统计信息
show port statistics
show thread counter
```

### RIB管理器 (rib-manager)
管理RIB（Routing Information Base）的工作者。

**功能：**
- 路由表管理
- 路由添加·删除·更新
- 路由信息分发
- 路由状态监视

**使用例：**
```bash
# 设置RIB管理器
set worker lthread rib-manager

# 确认RIB信息
show rib
```

### Netlink线程 (netlink-thread)
处理Netlink通信的工作者。

**功能：**
- 与内核的Netlink通信
- 网络设置监视
- 接口状态监视
- 路由信息接收

**使用例：**
```bash
# 设置Netlink线程
set worker lthread netlink-thread

# 确认网络状态
show port
show vswitch
```

## lthread配置

### 基本配置步骤
1. **设置必要的lthread工作者**
```bash
# 设置统计收集器
set worker lthread stat-collector

# 设置RIB管理器
set worker lthread rib-manager

# 设置Netlink线程
set worker lthread netlink-thread
```

2. **配置确认**
```bash
# 确认工作者状态
show worker

# 确认线程信息
show thread
```

### 推荐配置
一般用途推荐以下组合：
```bash
# 基本lthread工作者配置
set worker lthread stat-collector
set worker lthread rib-manager
set worker lthread netlink-thread
```

## 使用例

### 基本配置
```bash
# 设置lthread工作者
set worker lthread stat-collector
set worker lthread rib-manager
set worker lthread netlink-thread

# 配置确认
show worker
show thread
```

### 统计监视配置
```bash
# 设置统计收集器
set worker lthread stat-collector

# 定期确认统计信息
show port statistics
show thread counter
show mempool
```

### 路由管理配置
```bash
# 设置RIB管理器
set worker lthread rib-manager

# 确认路由信息
show rib
show vswitch
```

## 监视和管理

### lthread状态确认
```bash
# 整体状态确认
show worker
show thread

# 特定统计信息确认
show thread counter
show loop-count console pps
```

### 性能监视
```bash
# lthread性能确认
show thread counter

# 系统整体性能确认
show port statistics pps
show mempool
```

## 故障排除

### lthread无法运行时
1. 确认工作者配置
```bash
show worker
```

2. 确认线程状态
```bash
show thread
```

3. 确认系统状态
```bash
show rcu
show mempool
```

### 统计信息未更新时
1. 确认统计收集器状态
```bash
show worker
show thread
```

2. 手动确认统计信息
```bash
show port statistics
show thread counter
```

### RIB未更新时
1. 确认RIB管理器状态
```bash
show worker
show thread
```

2. 确认RIB信息
```bash
show rib
```

3. 确认Netlink线程状态
```bash
show worker
```

## 高级功能

### lthread协作动作
lthread协作运行，需要注意以下几点：

- **显式yield** - 长时间处理需要显式yield
- **避免死锁** - 通过协作设计避免死锁
- **公平性** - 所有线程都适当执行

### 性能优化
- **适当的工作者配置** - 考虑与CPU核心的亲和性
- **内存效率** - 设置适当的内存池大小
- **负载分散** - 在多个工作者中分散负载

## 定义位置

这些命令在以下文件中定义：
- `sdplane/lthread_main.c`

## 相关项目

- [工作者·lcore管理](worker-lcore-thread-management.md)
- [线程信息](worker-lcore-thread-management.md)
- [系统信息·监视](system-monitoring.md)
- [RIB·路由](routing.md)