# 系统信息·监视

**语言 / Language:** [English](../system-monitoring.md) | [日本語](../ja/system-monitoring.md) | [Français](../fr/system-monitoring.md) | **中文** | [Deutsch](../de/system-monitoring.md) | [Italiano](../it/system-monitoring.md) | [한국어](../ko/system-monitoring.md) | [ไทย](../th/system-monitoring.md) | [Español](../es/system-monitoring.md)

提供sdplane系统信息和监视功能的命令。

## 命令一览

### show_version - 版本显示
```
show version
```

显示sdplane的版本信息。

**使用例：**
```bash
show version
```

### set_locale - 区域设置
```
set locale (C|C.utf8|en_US.utf8|POSIX)
```

设置系统的区域设置。

**可用区域设置：**
- `C` - 标准C区域设置
- `C.utf8` - 支持UTF-8的C区域设置
- `en_US.utf8` - 英语UTF-8区域设置
- `POSIX` - POSIX区域设置

**使用例：**
```bash
# 设置为支持UTF-8的C区域设置
set locale C.utf8

# 设置为英语UTF-8区域设置
set locale en_US.utf8
```

### set_argv_list_1 - argv-list设置
```
set argv-list <0-7> <WORD>
```

设置命令行参数列表。

**参数：**
- `<0-7>` - 索引（0-7）
- `<WORD>` - 要设置的字符串

**使用例：**
```bash
# 为索引0设置参数
set argv-list 0 "--verbose"

# 为索引1设置参数
set argv-list 1 "--config"
```

### **show argv-list**

显示所有已设置的命令行参数列表。

**使用例：**
```bash
# 显示所有argv-list
show argv-list
```

---

### **show argv-list \<0-7\>**

显示特定索引的argv-list。

**使用例：**
```bash
# 显示特定索引的argv-list
show argv-list 0

# 显示argv-list索引3
show argv-list 3
```

### show_loop_count - 循环计数器显示
```
show loop-count (console|vty-shell|l2fwd) (pps|total)
```

显示各组件的循环计数器。

**组件：**
- `console` - 控制台
- `vty-shell` - VTY Shell
- `l2fwd` - L2转发

**统计类型：**
- `pps` - 每秒循环数
- `total` - 总循环数

**使用例：**
```bash
# 显示控制台的PPS
show loop-count console pps

# 显示L2转发的总循环数
show loop-count l2fwd total
```

### show_rcu - RCU信息显示
```
show rcu
```

显示RCU（Read-Copy-Update）信息。

**使用例：**
```bash
show rcu
```

### show_fdb - FDB信息显示
```
show fdb
```

显示FDB（Forwarding Database）信息。

**使用例：**
```bash
show fdb
```

### show_vswitch - vswitch信息显示
```
show vswitch
```

显示虚拟交换机信息。

**使用例：**
```bash
show vswitch
```

### sleep_cmd - 睡眠命令
```
sleep <0-300>
```

睡眠指定的秒数。

**参数：**
- `<0-300>` - 睡眠时间（秒）

**使用例：**
```bash
# 睡眠5秒
sleep 5

# 睡眠30秒
sleep 30
```

### show_mempool - 内存池信息显示
```
show mempool
```

显示DPDK内存池信息。

**使用例：**
```bash
show mempool
```

## 监视项目说明

### 版本信息
- sdplane版本
- 构建信息
- 依赖库版本

### 循环计数器
- 各组件的处理循环次数
- 用于性能监视
- 用于计算PPS（Packets Per Second）

### RCU信息
- Read-Copy-Update机制状态
- 同步处理状况
- 内存管理状态

### FDB信息
- MAC地址表状态
- 已学习的MAC地址
- 老化信息

### vswitch信息
- 虚拟交换机配置
- 端口信息
- VLAN配置

### 内存池信息
- 可用内存
- 使用中内存
- 内存池统计

## 监视最佳实践

### 定期监视
```bash
# 基本监视命令
show version
show mempool
show vswitch
show rcu
```

### 性能监视
```bash
# 通过循环计数器进行性能监视
show loop-count console pps
show loop-count l2fwd pps
```

### 故障排除
```bash
# 系统状态确认
show fdb
show vswitch
show mempool
```

## 定义位置

这些命令在以下文件中定义：
- `sdplane/sdplane.c`

## 相关项目

- [端口管理·统计](port-management.md)
- [工作者·lcore管理](worker-lcore-thread-management.md)
- [线程信息](worker-lcore-thread-management.md)