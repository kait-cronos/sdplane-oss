# DPDK应用集成指南

**语言 / Language:** [English](../dpdk-integration-guide.md) | [日本語](../ja/dpdk-integration-guide.md) | [Français](../fr/dpdk-integration-guide.md) | **中文** | [Deutsch](../de/dpdk-integration-guide.md) | [Italiano](../it/dpdk-integration-guide.md) | [한국어](../ko/dpdk-integration-guide.md) | [ไทย](../th/dpdk-integration-guide.md) | [Español](../es/dpdk-integration-guide.md)

本指南说明如何使用DPDK-dock方法将现有的DPDK应用及其基于pthread的工作者线程集成到sdplane框架中。

## 概要

sdplane提供集成框架，使用**DPDK-dock方法**在单个进程空间内运行多个DPDK应用。不是让每个DPDK应用初始化自己的EAL环境，而是将应用作为工作者模块集成到sdplane的协作线程模型中。

DPDK-dock方法允许多个DPDK应用通过以下方式高效共享资源并共存：
- DPDK EAL初始化的集中化
- 共享内存池管理的提供
- 端口和队列配置的统一
- 不同DPDK工作负载间协作多任务处理的实现

## 主要集成点

### 工作者线程转换
- 将传统DPDK pthread工作者转换为sdplane lcore工作者
- 将`pthread_create()`替换为sdplane的`set worker lcore <id> <worker-type>`
- 与sdplane基于lcore的线程模型集成

### 初始化集成
- 删除应用特定的`rte_eal_init()`调用
- 使用sdplane的集中EAL初始化
- 通过sdplane命令框架注册应用特定的CLI命令

### 内存和端口管理
- 利用sdplane的共享内存池管理
- 使用sdplane的端口配置和队列分配系统
- 与sdplane的RIB（Routing Information Base）集成以获取端口状态

## 集成步骤

### 1. 识别工作者函数
在DPDK应用中识别主要的包处理循环。这些通常是具有以下特征的函数：
- 在处理包的无限循环中运行
- 使用`rte_eth_rx_burst()`和`rte_eth_tx_burst()`
- 处理包转发或处理逻辑

### 2. 创建工作者模块
按照sdplane的工作者接口实现工作者函数：

```c
static __thread uint64_t loop_counter = 0;

int
my_worker_function(__rte_unused void *dummy)
{
    unsigned lcore_id = rte_lcore_id();
    int thread_id;
    
    // 为监视注册循环计数器
    thread_id = thread_lookup_by_lcore(my_worker_function, lcore_id);
    thread_register_loop_counter(thread_id, &loop_counter);
    
    while (!force_quit && !force_stop[lcore_id]) {
        // 在此处放置包处理逻辑
        
        // 为监视增加循环计数器
        loop_counter++;
    }
    
    return 0;
}
```

### 工作者循环计数器监视

`loop_counter`变量允许从sdplane shell监视工作者性能：

- **线程局部变量**: 每个工作者维护自己的循环计数器
- **注册**: 使用工作者名称和lcore ID向sdplane监视系统注册计数器
- **增量**: 计数器在主循环的每次迭代中增加
- **监视**: 从sdplane CLI显示计数器值以确认工作者活动

**CLI监视命令:**
```bash
# 显示包含循环计数器的线程计数器信息
show thread counter

# 显示一般线程信息
show thread

# 显示工作者配置和状态
show worker
```

这允许管理员确认工作者正在积极处理，并通过观察循环计数器增量来检测潜在的性能问题或工作者停滞。

### 3. 使用RCU访问RIB信息

为了在DPDK包处理工作者中访问端口信息和配置，sdplane通过RCU（Read-Copy-Update）提供RIB（Routing Information Base）访问以实现线程安全操作。

#### RIB访问模式

```c
#if HAVE_LIBURCU_QSBR
#include <urcu/urcu-qsbr.h>
#endif /*HAVE_LIBURCU_QSBR*/

static __thread struct rib *rib = NULL;

int
my_worker_function(__rte_unused void *dummy)
{
    unsigned lcore_id = rte_lcore_id();
    int thread_id;
    
    // 为监视注册循环计数器
    thread_id = thread_lookup_by_lcore(my_worker_function, lcore_id);
    thread_register_loop_counter(thread_id, &loop_counter);
    
#if HAVE_LIBURCU_QSBR
    urcu_qsbr_register_thread();
#endif /*HAVE_LIBURCU_QSBR*/

    while (!force_quit && !force_stop[lcore_id]) {
#if HAVE_LIBURCU_QSBR
        urcu_qsbr_read_lock();
        rib = (struct rib *) rcu_dereference(rcu_global_ptr_rib);
#endif /*HAVE_LIBURCU_QSBR*/

        // 在此处放置包处理逻辑
        // 通过rib->rib_info->port[portid]访问端口信息
        
#if HAVE_LIBURCU_QSBR
        urcu_qsbr_read_unlock();
        urcu_qsbr_quiescent_state();
#endif /*HAVE_LIBURCU_QSBR*/

        loop_counter++;
    }

#if HAVE_LIBURCU_QSBR
    urcu_qsbr_unregister_thread();
#endif /*HAVE_LIBURCU_QSBR*/
    
    return 0;
}
```

#### 访问端口信息

获取RIB后，访问端口特定信息：

```c
// 检查端口链路状态
if (!rib->rib_info->port[portid].link.link_status) {
    // 端口关闭，跳过处理
    continue;
}

// 检查端口是否停止
if (unlikely(rib->rib_info->port[portid].is_stopped)) {
    // 端口被管理停止
    continue;
}

// 访问端口配置
struct port_config *port_config = &rib->rib_info->port[portid];

// 获取lcore队列配置
struct lcore_qconf *lcore_qconf = &rib->rib_info->lcore_qconf[lcore_id];
for (i = 0; i < lcore_qconf->nrxq; i++) {
    portid = lcore_qconf->rx_queue_list[i].port_id;
    queueid = lcore_qconf->rx_queue_list[i].queue_id;
    // 处理来自此端口/队列的包
}
```

#### RCU安全指导原则

- **线程注册**: 始终使用`urcu_qsbr_register_thread()`注册线程
- **读锁定**: 在访问RIB数据前获取读锁
- **解引用**: 使用`rcu_dereference()`安全访问RCU保护指针
- **静默状态**: 调用`urcu_qsbr_quiescent_state()`表示安全点
- **线程清理**: 使用`urcu_qsbr_unregister_thread()`注销线程

#### RIB数据结构

通过RIB可用的关键信息：
- **端口信息**: 链路状态、配置、统计
- **队列配置**: lcore到端口/队列分配
- **VLAN配置**: 虚拟交换机和VLAN设置（高级功能）
- **接口配置**: TAP接口和路由信息

### 4. 添加CLI命令
向sdplane的CLI系统注册应用特定命令：

```c
CLI_COMMAND2(my_app_command,
             "my-app command <WORD>",
             "my application\n",
             "command help\n")
{
    // 命令实现
    return 0;
}

// 在初始化函数中
INSTALL_COMMAND2(cmdset, my_app_command);
```

### 4. 构建系统配置
更新构建配置以包含模块：

```makefile
# 添加到sdplane/Makefile.am
sdplane_SOURCES += my_worker.c my_worker.h
```

### 5. 集成测试
使用sdplane配置验证功能：

```bash
# 在sdplane配置中
set worker lcore 1 my-worker-type
set thread 1 port 0 queue 0

# 启动工作者
start worker lcore all
```

## 集成应用

以下DPDK应用已使用DPDK-dock方法成功集成到sdplane：

### L2FWD集成 (`module/l2fwd/`)
**状态**: ✅ 成功集成
- 为sdplane调整原始DPDK l2fwd应用
- 演示端口间基本包转发
- **参考**: 原始DPDK l2fwd源码的补丁文件可用
- 演示pthread到lcore工作者转换
- 主要文件: `module/l2fwd/l2fwd.c`，集成补丁

### PKTGEN集成 (`module/pktgen/`)
**状态**: ✅ 成功集成
- 完整DPDK PKTGEN应用集成
- 复杂多线程应用示例
- 外部库与sdplane CLI集成
- 演示高级集成技术
- 主要文件: `module/pktgen/app/`，`sdplane/pktgen_cmd.c`

### L3FWD集成
**状态**: ⚠️ 部分集成（未完全工作）
- 尝试集成但需要额外工作
- 不推荐作为新集成的参考
- 使用L2FWD和PKTGEN作为主要示例

## 推荐集成参考

对于集成新DPDK应用的开发者，建议使用以下作为主要参考：

### 1. L2FWD集成（推荐）
- **位置**: `module/l2fwd/`
- **补丁文件**: 原始DPDK l2fwd源码可用
- **范围**: 理解基本pthread到lcore转换
- **集成模式**: 简单包转发工作者

### 2. PKTGEN集成（高级参考）
- **位置**: `module/pktgen/`和`sdplane/pktgen_cmd.c`
- **范围**: 带CLI命令的复杂应用集成
- **集成模式**: 具有外部依赖的多组件应用

### 集成补丁文件

对于L2FWD集成，提供补丁文件显示将原始DPDK l2fwd应用适配为sdplane集成所需的确切修改。这些补丁演示：
- EAL初始化删除
- 线程模型适配
- CLI命令集成
- 资源管理更改

开发者应参考这些补丁文件以理解DPDK应用集成的系统方法。

## 自定义工作者示例

### L2中继器 (`sdplane/l2_repeater.c`)
自定义sdplane工作者（不基于现有DPDK应用）：
- 简单包转发工作者
- 广播转发到所有活动端口
- 与sdplane端口管理集成

### 增强中继器 (`sdplane/enhanced_repeater.c`)
具有高级功能的自定义sdplane工作者：
- VLAN感知交换
- TAP接口集成
- 端口状态的RIB集成

### VLAN交换机 (`sdplane/vlan_switch.c`)
自定义第二层交换实现：
- MAC学习和转发
- VLAN处理
- 多端口包处理

## 最佳实践

### 性能考虑
- 使用突发处理以获得最佳性能
- 尽可能最小化包拷贝
- 利用DPDK的零拷贝机制
- 在工作者分配中考虑NUMA拓扑

### 错误处理
- 检查DPDK函数返回值
- 实现适当的关闭处理
- 使用适当的日志级别
- 适当处理资源清理

### 线程模型
- 理解sdplane基于lcore的线程
- 设计高效的包处理循环
- 使用适当的同步机制
- 考虑线程亲和性和CPU隔离

## 调试和监视

### 调试日志
启用工作者调试日志：

```bash
debug sdplane my-worker-type
```

### 统计收集
与sdplane统计框架集成：

```c
// 更新端口统计
port_statistics[portid].rx += nb_rx;
port_statistics[portid].tx += nb_tx;
```

### CLI监视
提供用于监视的状态命令：

```bash
show my-worker status
show my-worker statistics
```

## 常见集成模式

### 包处理管道
```c
// sdplane工作者中的典型包处理
while (!force_quit && !force_stop[lcore_id]) {
    // 1. 包接收
    nb_rx = rte_eth_rx_burst(portid, queueid, pkts_burst, MAX_PKT_BURST);
    
    // 2. 包处理
    for (i = 0; i < nb_rx; i++) {
        // 处理逻辑
        process_packet(pkts_burst[i]);
    }
    
    // 3. 包发送
    rte_eth_tx_burst(dst_port, queueid, pkts_burst, nb_rx);
}
```

### 配置集成
```c
// 向sdplane配置系统注册
struct worker_config my_worker_config = {
    .name = "my-worker",
    .worker_func = my_worker_function,
    .init_func = my_worker_init,
    .cleanup_func = my_worker_cleanup
};
```

## 故障排除

### 常见问题
- **工作者未启动**: 检查lcore分配和配置
- **包丢失**: 检查队列配置和缓冲区大小
- **性能问题**: 检查CPU亲和性和NUMA设置
- **CLI命令不工作**: 检查适当的命令注册

### 调试技术
- 使用sdplane调试日志系统
- 监视工作者统计和计数器
- 检查端口链路状态和配置
- 验证内存池分配

有关详细实现示例，请参考代码库中现有的工作者模块和相应的CLI命令定义。