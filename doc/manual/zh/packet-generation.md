# 包生成

**语言 / Language:** [English](../packet-generation.md) | [日本語](../ja/packet-generation.md) | [Français](../fr/packet-generation.md) | **中文** | [Deutsch](../de/packet-generation.md) | [Italiano](../it/packet-generation.md) | [한국어](../ko/packet-generation.md) | [ไทย](../th/packet-generation.md) | [Español](../es/packet-generation.md)

使用PKTGEN（Packet Generator）进行包生成功能的命令。

## 命令一览

### show_pktgen - PKTGEN信息显示
```
show pktgen
```

显示PKTGEN（包生成器）的当前状态和配置。

**使用例：**
```bash
show pktgen
```

该命令显示以下信息：
- PKTGEN的初始化状态
- 当前配置参数
- 正在运行的任务
- 统计信息

### pktgen_init - PKTGEN初始化
```
pktgen init argv-list <0-7>
```

使用指定的argv-list初始化PKTGEN。

**参数：**
- `<0-7>` - 使用的argv-list索引

**使用例：**
```bash
# 使用argv-list 0初始化PKTGEN
pktgen init argv-list 0

# 使用argv-list 2初始化PKTGEN
pktgen init argv-list 2
```

### pktgen_do_start - PKTGEN启动
```
pktgen do start (<0-7>|all)
```

在指定端口上开始包生成。

**目标：**
- `<0-7>` - 特定端口号
- `all` - 所有端口

**使用例：**
```bash
# 在端口0上开始包生成
pktgen do start 0

# 在所有端口上开始包生成
pktgen do start all
```

### pktgen_do_stop - PKTGEN停止
```
pktgen do stop (<0-7>|all)
```

在指定端口上停止包生成。

**目标：**
- `<0-7>` - 特定端口号
- `all` - 所有端口

**使用例：**
```bash
# 在端口1上停止包生成
pktgen do stop 1

# 在所有端口上停止包生成
pktgen do stop all
```

## PKTGEN概要

### PKTGEN是什么
PKTGEN（Packet Generator）是用于网络测试的包生成工具。提供以下功能：

- **高速包生成** - 高性能的包生成
- **多样的包格式** - 支持各种协议
- **灵活的配置** - 可进行详细的包设置
- **统计功能** - 提供详细的统计信息

### 主要用途
- **网络性能测试** - 测量吞吐量和延迟
- **负载测试** - 系统负载耐受性测试
- **功能测试** - 网络功能验证
- **基准测试** - 性能比较测试

## PKTGEN配置

### 基本配置步骤
1. **argv-list设置**
```bash
# 设置PKTGEN用参数
set argv-list 0 "-c 0x3 -n 4"
set argv-list 1 "--socket-mem 1024"
set argv-list 2 "--huge-dir /mnt/huge"
```

2. **PKTGEN初始化**
```bash
pktgen init argv-list 0
```

3. **工作者设置**
```bash
set worker lcore 1 pktgen
start worker lcore 1
```

4. **开始包生成**
```bash
pktgen do start 0
```

### 配置参数
argv-list中可设置的参数示例：

- **-c** - CPU掩码
- **-n** - 内存通道数
- **--socket-mem** - 套接字内存大小
- **--huge-dir** - 大页目录
- **--file-prefix** - 文件前缀

## 使用例

### 基本包生成
```bash
# 配置
set argv-list 0 "-c 0x3 -n 4 --socket-mem 1024"

# 初始化
pktgen init argv-list 0

# 工作者配置
set worker lcore 1 pktgen
start worker lcore 1

# 开始包生成
pktgen do start 0

# 状态确认
show pktgen

# 停止包生成
pktgen do stop 0
```

### 多端口生成
```bash
# 多端口开始
pktgen do start all

# 状态确认
show pktgen

# 多端口停止
pktgen do stop all
```

## 监视和统计

### 统计信息确认
```bash
# 显示PKTGEN统计
show pktgen

# 显示端口统计
show port statistics

# 显示工作者统计
show worker
```

### 性能监视
```bash
# 确认PPS（Packets Per Second）
show port statistics pps

# 确认总包数
show port statistics total

# 确认字节/秒
show port statistics Bps
```

## 故障排除

### PKTGEN无法启动时
1. 确认初始化状态
```bash
show pktgen
```

2. 确认工作者状态
```bash
show worker
```

3. 确认端口状态
```bash
show port
```

### 包生成无法停止时
1. 明确停止
```bash
pktgen do stop all
```

2. 重启工作者
```bash
restart worker lcore 1
```

### 性能低时
1. 确认CPU使用率
2. 确认内存设置
3. 确认端口设置

## 高级功能

### 包格式设置
PKTGEN可以生成各种包格式：
- **Ethernet** - 基本Ethernet帧
- **IP** - IPv4/IPv6包
- **UDP/TCP** - UDP/TCP包
- **VLAN** - 带VLAN标签的包

### 负载控制
- **速率控制** - 控制包生成速率
- **突发控制** - 生成突发包
- **大小控制** - 控制包大小

## 定义位置

这些命令在以下文件中定义：
- `sdplane/pktgen_cmd.c`

## 相关项目

- [工作者·lcore管理](worker-management.md)
- [端口管理·统计](port-management.md)
- [系统信息·监视](system-monitoring.md)