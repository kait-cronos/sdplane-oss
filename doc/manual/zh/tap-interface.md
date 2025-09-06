# TAP接口

**语言 / Language:** [English](../tap-interface.md) | [日本語](../ja/tap-interface.md) | **中文**

管理TAP接口的命令。

## 命令一览

### set_tap_capture_ifname - TAP捕获接口名设置
```
set tap capture ifname <WORD>
```

设置TAP捕获功能使用的接口名。

**参数：**
- `<WORD>` - 接口名

**使用例：**
```bash
# 设置tap0接口
set tap capture ifname tap0

# 设置tap1接口
set tap capture ifname tap1
```

### set_tap_capture_persistent - TAP捕获持久化启用
```
set tap capture persistent
```

启用TAP捕获的持久化。

**使用例：**
```bash
# 启用持久化
set tap capture persistent
```

### no_tap_capture_persistent - TAP捕获持久化禁用
```
no tap capture persistent
```

禁用TAP捕获的持久化。

**使用例：**
```bash
# 禁用持久化
no tap capture persistent
```

### unset_tap_capture_persistent - TAP捕获持久化设置删除
```
unset tap capture persistent
```

删除TAP捕获的持久化设置。

**使用例：**
```bash
# 删除持久化设置
unset tap capture persistent
```

## TAP接口概要

### TAP接口是什么
TAP（Network TAP）接口是用于网络流量监视或测试的虚拟网络接口。

### 主要功能
- **包捕获** - 网络流量的捕获
- **包注入** - 测试包的注入
- **桥接功能** - 不同网络间的桥接
- **监视功能** - 流量的监视和分析

### 在sdplane中的用途
- **调试** - 包流的调试
- **测试** - 网络功能的测试
- **监视** - 流量监视
- **开发** - 新功能的开发和测试

## TAP接口配置

### 基本配置步骤
1. **创建TAP接口**
```bash
# 在系统级别创建TAP接口
sudo ip tuntap add tap0 mode tap
sudo ip link set tap0 up
```

2. **在sdplane中配置**
```bash
# 设置TAP捕获接口名
set tap capture ifname tap0

# 启用持久化
set tap capture persistent
```

3. **设置TAP处理器工作者**
```bash
# 设置TAP处理器工作者
set worker lcore 2 tap-handler
start worker lcore 2
```

### 配置例

#### 基本TAP配置
```bash
# TAP接口设置
set tap capture ifname tap0
set tap capture persistent

# 工作者设置
set worker lcore 2 tap-handler
start worker lcore 2

# 配置确认
show worker
```

#### 多TAP接口配置
```bash
# 设置多个TAP接口
set tap capture ifname tap0
set tap capture ifname tap1

# 启用持久化
set tap capture persistent
```

## 持久化功能

### 持久化是什么
启用持久化功能后，TAP接口的设置在系统重启后也会保持。

### 持久化的优点
- **设置保持** - 重启后设置仍然有效
- **自动恢复** - 从系统故障自动恢复
- **运营效率** - 减少手动设置

### 持久化设置
```bash
# 启用持久化
set tap capture persistent

# 禁用持久化
no tap capture persistent

# 删除持久化设置
unset tap capture persistent
```

## 使用例

### 调试用途
```bash
# 调试用TAP接口设置
set tap capture ifname debug-tap
set tap capture persistent

# 设置TAP处理器工作者
set worker lcore 3 tap-handler
start worker lcore 3

# 开始包捕获
tcpdump -i debug-tap
```

### 测试用途
```bash
# 测试用TAP接口设置
set tap capture ifname test-tap
set tap capture persistent

# 准备测试包注入
set worker lcore 4 tap-handler
start worker lcore 4
```

## 监视和管理

### TAP接口状态确认
```bash
# 确认工作者状态
show worker

# 确认线程信息
show thread

# 系统级别确认
ip link show tap0
```

### 流量监视
```bash
# 使用tcpdump监视
tcpdump -i tap0

# 使用Wireshark监视
wireshark -i tap0
```

## 故障排除

### TAP接口无法创建时
1. 系统级别确认
```bash
# 确认TAP接口存在
ip link show tap0

# 确认权限
sudo ip tuntap add tap0 mode tap
```

2. 在sdplane中确认
```bash
# 配置确认
show worker

# 确认工作者状态
show thread
```

### 包无法捕获时
1. 确认接口状态
```bash
ip link show tap0
```

2. 确认工作者状态
```bash
show worker
```

3. 重启TAP处理器
```bash
restart worker lcore 2
```

### 持久化无法正常工作时
1. 确认持久化设置
```bash
# 确认当前设置（通过show系列命令确认）
show worker
```

2. 确认系统设置
```bash
# 确认系统级别设置
systemctl status sdplane
```

## 高级功能

### 与VLAN的协作
TAP接口可以与VLAN功能协作使用：
```bash
# 与VLAN交换机工作者协作
set worker lcore 5 vlan-switch
start worker lcore 5
```

### 桥接功能
使用多个TAP接口进行桥接：
```bash
# 设置多个TAP接口
set tap capture ifname tap0
set tap capture ifname tap1
```

## 定义位置

这些命令在以下文件中定义：
- `sdplane/tap_cmd.c`

## 相关项目

- [工作者·lcore管理](worker-management.md)
- [VTY·Shell管理](vty-shell.md)
- [调试·日志](debug-logging.md)