# 线程信息

**语言 / Language:** [English](../thread-information.md) | [日本語](../ja/thread-information.md) | **中文**

处理线程信息和监视的命令。

## 命令一览

### show_thread_cmd - 线程信息显示
```
show thread
```

显示当前线程的状态和信息。

**使用例：**
```bash
show thread
```

该命令显示以下信息：
- 线程ID
- 线程状态
- 正在运行的任务
- CPU使用率
- 内存使用量

### show_thread_counter - 线程计数器显示
```
show thread counter
```

显示线程的计数器信息。

**使用例：**
```bash
show thread counter
```

该命令显示以下信息：
- 处理包数
- 执行次数
- 错误次数
- 处理时间统计

## 线程系统概要

### sdplane的线程架构
sdplane使用以下线程类型：

#### 1. 工作者线程
- **L2转发** - Layer 2包转发
- **L3转发** - Layer 3包转发
- **TAP处理器** - TAP接口处理
- **PKTGEN** - 包生成

#### 2. 管理线程
- **RIB管理器** - 路由信息管理
- **统计收集器** - 统计信息收集
- **Netlink线程** - Netlink通信处理

#### 3. 系统线程
- **VTY服务器** - VTY连接处理
- **控制台** - 控制台输入输出

### 与lthread的关系
sdplane使用协作多线程（cooperative threading）模型：

- **lthread** - 轻量级线程实现
- **协作调度** - 通过显式yield进行控制
- **高效率** - 减少上下文切换的开销

## 线程信息的查看方法

### 基本显示项目
- **Thread ID** - 线程标识符
- **Name** - 线程名称
- **State** - 线程状态
- **lcore** - 运行中的CPU核心
- **Type** - 线程类型

### 线程状态
- **Running** - 运行中
- **Ready** - 可运行
- **Blocked** - 阻塞中
- **Terminated** - 已结束

### 计数器信息
- **Packets** - 处理包数
- **Loops** - 循环执行次数
- **Errors** - 错误次数
- **CPU Time** - CPU使用时间

## 使用例

### 基本监视
```bash
# 显示线程信息
show thread

# 显示线程计数器
show thread counter
```

### 输出例解释
```bash
# show thread的输出例
Thread ID: 1
Name: l2fwd-worker
State: Running
lcore: 1
Type: L2FWD

Thread ID: 2
Name: rib-manager
State: Running
lcore: 2
Type: RIB_MANAGER
```

```bash
# show thread counter的输出例
Thread ID: 1
Packets: 1000000
Loops: 5000000
Errors: 0
CPU Time: 123.45s
```

## 监视和故障排除

### 定期监视
```bash
# 定期监视命令
show thread
show thread counter
```

### 性能分析
```bash
# 性能相关信息
show thread counter
show loop-count l2fwd pps
show worker
```

### 故障排除

#### 线程无响应时
1. 确认线程状态
```bash
show thread
```

2. 确认工作者状态
```bash
show worker
```

3. 必要时重启
```bash
restart worker lcore 1
```

#### 性能下降时
1. 确认计数器信息
```bash
show thread counter
```

2. 确认循环计数器
```bash
show loop-count l2fwd pps
```

3. 确认错误次数
```bash
show thread counter
```

#### 内存使用量过多时
1. 确认内存池信息
```bash
show mempool
```

2. 确认线程信息
```bash
show thread
```

## 线程优化

### CPU亲和性设置
- 将工作者配置到适当的lcore
- 考虑NUMA节点的配置
- 均衡化CPU使用率

### 内存效率改善
- 适当的内存池大小
- 防止内存泄漏
- 提高缓存效率

## 高级功能

### lthread管理
```bash
# lthread工作者设置
set worker lthread stat-collector
set worker lthread rib-manager
set worker lthread netlink-thread
```

### 统计信息活用
- 性能监视
- 容量规划
- 异常检测

## 定义位置

这些命令在以下文件中定义：
- `sdplane/thread_info.c`

## 相关项目

- [工作者·lcore管理](worker-management.md)
- [lthread管理](lthread-management.md)
- [系统信息·监视](system-monitoring.md)