# 包生成器（PKTGEN）应用

**语言 / Language:** [English](../packet-generator-application.md) | [日本語](../ja/packet-generator-application.md) | **中文**

包生成器（PKTGEN）应用使用DPDK优化的包处理框架提供高性能包生成和流量测试功能。

## 概要

PKTGEN是集成到sdplane-oss中的复杂流量生成工具，实现：
- **高速包生成**: 多千兆位包传输速率
- **灵活的流量模式**: 可定制包大小、速率、模式
- **多端口支持**: 多个端口上的独立流量生成
- **高级功能**: 范围测试、速率限制、流量整形
- **性能测试**: 网络吞吐量和延迟测量

## 架构

### 核心组件
- **TX引擎**: 使用DPDK的高性能包传输
- **RX引擎**: 包接收和统计收集
- **L2P框架**: 最佳性能的lcore到端口映射
- **配置管理**: 动态流量参数配置
- **统计引擎**: 全面的性能指标和报告

### 工作者模型
PKTGEN使用专用工作者线程（lcore）运行：
- **TX工作者**: 包传输专用核心
- **RX工作者**: 包接收专用核心
- **混合工作者**: 单核心上TX/RX组合
- **控制线程**: 管理和统计收集

## 主要功能

### 流量生成
- **包速率**: 直到接口限制的线速流量生成
- **包大小**: 64字节到巨型帧可配置
- **流量模式**: 均匀、突发、自定义模式
- **多流**: 每端口多流量流

### 高级功能
- **速率限制**: 精确流量速率控制
- **范围测试**: 包大小扫描和速率扫描
- **负载模式**: 恒定、渐增、突发流量
- **协议支持**: 以太网、IPv4、IPv6、TCP、UDP

### 性能监视
- **实时统计**: TX/RX速率、包数、错误数
- **延迟测量**: 端到端包延迟测试
- **吞吐量分析**: 带宽利用率和效率
- **错误检测**: 包丢失和损坏检测

## 配置

### 基本设置
PKTGEN需要特定的初始化和工作者配置：

```bash
# PKTGEN设备绑定
set device 01:00.0 driver vfio-pci bind
set device 01:00.1 driver vfio-pci bind

# 通过argv-list初始化PKTGEN
set argv-list 2 ./usr/local/bin/pktgen -l 0-7 -n 4 --proc-type auto --log-level 7 --file-prefix pg -- -v -T -P -l pktgen.log -m [4:5].0 -m [6:7].1 -f themes/black-yellow.theme

# PKTGEN初始化
pktgen init argv-list 2
```

### 工作者分配
最佳性能的专用核心分配：

```bash
# 将PKTGEN工作者分配给特定lcore
set worker lcore 4 pktgen  # 端口0 TX/RX
set worker lcore 5 pktgen  # 端口0 TX/RX
set worker lcore 6 pktgen  # 端口1 TX/RX
set worker lcore 7 pktgen  # 端口1 TX/RX
```

### 端口配置
用于PKTGEN的DPDK端口配置：

```bash
# 端口设置
stop port all
set port all dev-configure 1 4
set port all nrxdesc 1024
set port all ntxdesc 1024
set port all promiscuous enable
start port all
```

## 配置示例

### 完整PKTGEN设置
请参见[`example-config/sdplane-pktgen.conf`](../../example-config/sdplane-pktgen.conf)：

```bash
# 日志配置
log file /var/log/sdplane.log
log stdout

# 设备绑定
set device 01:00.0 driver vfio-pci bind
set device 01:00.1 driver vfio-pci bind
set device 03:00.0 driver uio_pci_generic bind

# PKTGEN初始化
set argv-list 2 ./usr/local/bin/pktgen -l 0-7 -n 4 --proc-type auto --log-level 7 --file-prefix pg -- -v -T -P -l pktgen.log -m [4:5].0 -m [6:7].1 -f themes/black-yellow.theme
pktgen init argv-list 2

# 内存池设置
set mempool

# 后台工作者
set worker lthread stat-collector
set worker lthread rib-manager
set worker lthread netlink-thread

# 端口配置
stop port all
set port all dev-configure 1 4
set port all nrxdesc 1024
set port all ntxdesc 1024

# 工作者分配
set worker lcore 2 vlan-switch
set worker lcore 3 tap-handler
set worker lcore 4 pktgen
set worker lcore 5 pktgen
set worker lcore 6 pktgen
set worker lcore 7 pktgen

# 启用混杂模式并启动
set port all promiscuous enable
start port all

# 启动所有工作者
start worker lcore all
```

## PKTGEN命令行参数

### 核心参数
- `-l 0-7`: CPU核心列表（核心0-7）
- `-n 4`: 内存通道数
- `--proc-type auto`: 进程类型（主/从）
- `--log-level 7`: 调试日志级别
- `--file-prefix pg`: 共享内存前缀

### PKTGEN特定选项
- `-v`: 详细输出
- `-T`: 启用彩色终端输出
- `-P`: 启用混杂模式
- `-l pktgen.log`: 日志文件位置
- `-m [4:5].0`: 将lcore 4,5映射到端口0
- `-m [6:7].1`: 将lcore 6,7映射到端口1
- `-f themes/black-yellow.theme`: 颜色主题

## lcore到端口映射

### 映射语法
映射格式`[lcore_list].port`将核心分配给端口：
- `[4:5].0`: lcore 4和5处理端口0
- `[6:7].1`: lcore 6和7处理端口1
- `[4].0`: 单个lcore 4处理端口0
- `[4-7].0`: lcore 4到7处理端口0

### 性能优化
- **专用核心**: 为TX和RX分配单独核心
- **NUMA感知**: 使用网络接口本地的核心
- **避免冲突**: 避免与系统工作者重叠
- **负载平衡**: 跨多核心分布流量

## 操作和监视

### 状态命令
```bash
# 显示PKTGEN状态和配置
show pktgen

# 显示工作者分配和性能
show worker statistics

# 端口级统计
show port statistics all
```

### 运行时管理
PKTGEN通过交互式CLI提供广泛的运行时配置：
- **流量参数**: 包大小、速率、模式
- **启动/停止控制**: 每端口流量控制
- **统计**: 实时性能监视
- **范围测试**: 参数扫描自动化

### 性能监视
```bash
# 监视实时统计
# （通过PKTGEN交互界面可用）

# 关键指标:
# - TX/RX每秒包数
# - 带宽利用率
# - 包丢失率
# - 延迟测量
```

## CLI命令

### PKTGEN控制命令

#### 初始化
```bash
# 用argv-list配置初始化PKTGEN
pktgen init argv-list <0-7>
```

#### 流量生成启动/停止
```bash
# 在特定端口启动流量生成
pktgen do start port <0-7>
pktgen do start port all

# 停止流量生成
pktgen do stop port <0-7>
pktgen do stop port all
```

#### 流量配置命令

##### 包数设置
```bash
# 设置要发送的包数
pktgen do set port <0-7> count <0-4000000000>
pktgen do set port all count <0-4000000000>
```

##### 包大小设置
```bash
# 以字节为单位设置包大小
pktgen do set port <0-7> size <0-9999>
pktgen do set port all size <0-9999>
```

##### 发送速率设置
```bash
# 以百分比设置发送速率
pktgen do set port <0-7> rate <0-100>
pktgen do set port all rate <0-100>
```

##### TCP/UDP端口号设置
```bash
# 设置TCP源和目标端口
pktgen do set port <0-7> tcp src <0-65535> dst <0-65535>
pktgen do set port all tcp src <0-65535> dst <0-65535>

# 设置UDP源和目标端口
pktgen do set port <0-7> udp src <0-65535> dst <0-65535>
pktgen do set port all udp src <0-65535> dst <0-65535>
```

##### TTL值设置
```bash
# 设置IP生存时间值
pktgen do set port <0-7> ttl <0-255>
pktgen do set port all ttl <0-255>
```

##### MAC地址设置
```bash
# 设置源MAC地址
pktgen do set port <0-7> mac source <MAC>
pktgen do set port all mac source <MAC>

# 设置目标MAC地址
pktgen do set port <0-7> mac destination <MAC>
pktgen do set port all mac destination <MAC>
```

##### IPv4地址设置
```bash
# 设置源IPv4地址
pktgen do set port <0-7> ipv4 source <IPv4>
pktgen do set port all ipv4 source <IPv4>

# 设置目标IPv4地址
pktgen do set port <0-7> ipv4 destination <IPv4>
pktgen do set port all ipv4 destination <IPv4>
```

#### 状态·监视命令
```bash
# 显示PKTGEN状态和配置
show pktgen

# 显示端口统计
show port statistics all
show port statistics <0-7>
```

## 使用案例

### 网络性能测试
- **吞吐量测试**: 最大带宽测量
- **延迟测试**: 端到端延迟分析
- **负载测试**: 持续流量生成
- **压力测试**: 最大包率验证

### 设备验证
- **交换机测试**: 转发性能验证
- **路由器测试**: L3转发性能
- **接口测试**: 端口和电缆验证
- **协议测试**: 特定协议行为

### 网络开发
- **协议开发**: 新网络协议测试
- **应用测试**: 现实流量模式生成
- **性能调优**: 网络配置优化
- **基准测试**: 标准化性能比较

## 性能调优

### 核心分配
```bash
# 为最高性能优化核心使用
# 尽可能分离TX和RX核心
# 使用网络接口本地的NUMA核心
set worker lcore 4 pktgen  # 端口0的TX核心
set worker lcore 5 pktgen  # 端口0的RX核心
```

### 内存设置
```bash
# 根据流量模式优化描述符环
set port all nrxdesc 2048  # 为高速率增加
set port all ntxdesc 2048  # 为突发增加
```

### 系统调优
- **CPU隔离**: 从OS调度器隔离PKTGEN核心
- **中断亲和性**: 将中断绑定到非PKTGEN核心
- **内存分配**: 使用大页以获得最佳性能
- **NIC调优**: 优化网络接口配置

## 故障排除

### 常见问题
- **性能低**: 检查核心分配和NUMA拓扑
- **包丢失**: 检查缓冲区大小和系统资源
- **初始化失败**: 检查设备绑定和权限
- **速率限制**: 检查接口能力和配置

### 调试策略
- **启用详细日志**: 使用更高日志级别获得详细输出
- **检查统计**: 监视TX/RX计数器异常
- **验证映射**: 确保正确的lcore到端口分配
- **系统监视**: 测试期间检查CPU、内存、中断使用

#### 调试命令
```bash
# 启用PKTGEN调试日志
debug sdplane pktgen

# 通用sdplane调试
debug sdplane rib
debug sdplane fdb-change
```

### 性能验证
```bash
# 确认PKTGEN达到预期速率
show pktgen
show port statistics all

# 检查错误或丢包
# 测试期间监视系统资源
```

## 高级功能

### 范围测试
PKTGEN支持参数范围的自动测试：
- **包大小扫描**: 64到1518字节测试
- **速率扫描**: 线速1%到100%测试
- **自动报告**: 全面测试报告生成

### 流量模式
- **恒定速率**: 稳定流量生成
- **突发模式**: 有空闲期的流量突发
- **渐增模式**: 逐渐增加/减少速率
- **自定义模式**: 用户定义流量配置文件

## 相关文档

- [包生成命令](packet-generation.md) - 命令参考
- [工作者管理](worker-management.md) - 工作者配置
- [端口管理](port-management.md) - DPDK端口设置
- [性能调优指南](#) - 系统优化提示

## 外部资源

- [DPDK Pktgen文档](http://pktgen-dpdk.readthedocs.io/) - 官方PKTGEN文档
- [DPDK性能指南](https://doc.dpdk.org/guides/prog_guide/) - DPDK优化指南