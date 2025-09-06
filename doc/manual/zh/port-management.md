# 端口管理·统计

**语言 / Language:** [English](../port-management.md) | [日本語](../ja/port-management.md) | **中文**

管理DPDK端口并处理统计信息的命令。

## 命令一览

### **start port**

启动DPDK端口（默认操作）。

**使用例：**
```bash
# 启动端口（默认）
start port
```

---

### **start port \<0-16\>**

启动特定的DPDK端口。

**使用例：**
```bash
# 启动端口0
start port 0

# 启动端口1
start port 1
```

---

### **start port all**

启动所有DPDK端口。

**使用例：**
```bash
# 启动所有端口
start port all
```

---

### **stop port**

停止DPDK端口（默认操作）。

**使用例：**
```bash
# 停止端口（默认）
stop port
```

---

### **stop port \<0-16\>**

停止特定的DPDK端口。

**使用例：**
```bash
# 停止端口0
stop port 0

# 停止端口1
stop port 1
```

---

### **stop port all**

停止所有DPDK端口。

**使用例：**
```bash
# 停止所有端口
stop port all
```

---

### **reset port**

重置DPDK端口（默认操作）。

**使用例：**
```bash
# 重置端口（默认）
reset port
```

---

### **reset port \<0-16\>**

重置特定的DPDK端口。

**使用例：**
```bash
# 重置端口0
reset port 0

# 重置端口1
reset port 1
```

---

### **reset port all**

重置所有DPDK端口。

**使用例：**
```bash
# 重置所有端口
reset port all
```

---

### **show port**

显示所有端口的基本信息（默认操作）。

**使用例：**
```bash
# 显示所有端口的信息
show port
```

---

### **show port \<0-16\>**

显示特定端口的基本信息。

**使用例：**
```bash
# 显示端口0的信息
show port 0

# 显示端口1的信息
show port 1
```

---

### **show port all**

明确显示所有端口的信息。

**使用例：**
```bash
# 明确显示所有端口的信息
show port all
```

---

### **show port statistics**

显示所有端口的统计信息。

**使用例：**
```bash
# 显示所有统计信息
show port statistics
```

---

### **show port statistics pps**

显示包/秒的统计信息。

**使用例：**
```bash
# 仅显示PPS统计
show port statistics pps
```

---

### **show port statistics total**

显示总包数的统计信息。

**使用例：**
```bash
# 显示总包数
show port statistics total
```

---

### **show port statistics bps**

显示比特/秒的统计信息。

**使用例：**
```bash
# 显示比特/秒
show port statistics bps
```

---

### **show port statistics Bps**

显示字节/秒的统计信息。

**使用例：**
```bash
# 显示字节/秒
show port statistics Bps
```

---

### **show port statistics total-bytes**

显示总字节数的统计信息。

**使用例：**
```bash
# 显示总字节数
show port statistics total-bytes
```

---

### **show port \<0-16\> promiscuous**

显示指定端口的混杂模式状态。

**使用例：**
```bash
# 显示端口0的混杂模式
show port 0 promiscuous

# 显示端口1的混杂模式
show port 1 promiscuous
```

---

### **show port all promiscuous**

显示所有端口的混杂模式状态。

**使用例：**
```bash
# 显示所有端口的混杂模式
show port all promiscuous
```

---

### **show port \<0-16\> flowcontrol**

显示指定端口的流量控制设置。

**使用例：**
```bash
# 显示端口0的流量控制设置
show port 0 flowcontrol

# 显示端口1的流量控制设置
show port 1 flowcontrol
```

---

### **show port all flowcontrol**

显示所有端口的流量控制设置。

**使用例：**
```bash
# 显示所有端口的流量控制设置
show port all flowcontrol
```

---

### **set port \<0-16\> promiscuous enable**

启用指定端口的混杂模式。

**使用例：**
```bash
# 启用端口0的混杂模式
set port 0 promiscuous enable

# 启用端口1的混杂模式
set port 1 promiscuous enable
```

---

### **set port \<0-16\> promiscuous disable**

禁用指定端口的混杂模式。

**使用例：**
```bash
# 禁用端口0的混杂模式
set port 0 promiscuous disable

# 禁用端口1的混杂模式
set port 1 promiscuous disable
```

---

### **set port all promiscuous enable**

启用所有端口的混杂模式。

**使用例：**
```bash
# 启用所有端口的混杂模式
set port all promiscuous enable
```

---

### **set port all promiscuous disable**

禁用所有端口的混杂模式。

**使用例：**
```bash
# 禁用所有端口的混杂模式
set port all promiscuous disable
```

---

### **set port \<0-16\> flowcontrol (rx|tx|autoneg|send-xon|fwd-mac-ctrl) (on|off)**

更改指定端口的流量控制设置。

**选项：**
- `rx` - 接收流量控制
- `tx` - 发送流量控制
- `autoneg` - 自动协商
- `send-xon` - XON发送
- `fwd-mac-ctrl` - MAC控制帧转发

**使用例：**
```bash
# 启用端口0的接收流量控制
set port 0 flowcontrol rx on

# 禁用端口1的自动协商
set port 1 flowcontrol autoneg off
```

---

### **set port all flowcontrol (rx|tx|autoneg|send-xon|fwd-mac-ctrl) (on|off)**

更改所有端口的流量控制设置。

**选项：**
- `rx` - 接收流量控制
- `tx` - 发送流量控制
- `autoneg` - 自动协商
- `send-xon` - XON发送
- `fwd-mac-ctrl` - MAC控制帧转发

**使用例：**
```bash
# 禁用所有端口的自动协商
set port all flowcontrol autoneg off

# 启用所有端口的发送流量控制
set port all flowcontrol tx on
```

---

### **set port \<0-16\> dev-configure \<0-64\> \<0-64\>**


对指定端口进行DPDK设备配置。

**参数：**
- 第1参数: 接收队列数 (0-64)
- 第2参数: 发送队列数 (0-64)

**使用例：**
```bash
# 配置端口0为接收队列4，发送队列4
set port 0 dev-configure 4 4

# 配置端口1为接收队列2，发送队列2
set port 1 dev-configure 2 2
```

---

### **set port all dev-configure \<0-64\> \<0-64\>**

对所有端口进行DPDK设备配置。

**参数：**
- 第1参数: 接收队列数 (0-64)
- 第2参数: 发送队列数 (0-64)

**使用例：**
```bash
# 配置所有端口为接收队列1，发送队列1
set port all dev-configure 1 1
```

---

### **set port \<0-16\> nrxdesc \<0-16384\>**


设置指定端口的接收描述符数量。

**使用例：**
```bash
# 将端口0的接收描述符数量设置为1024
set port 0 nrxdesc 1024

# 将端口1的接收描述符数量设置为512
set port 1 nrxdesc 512
```

---

### **set port all nrxdesc \<0-16384\>**

设置所有端口的接收描述符数量。

**使用例：**
```bash
# 将所有端口的接收描述符数量设置为512
set port all nrxdesc 512
```

---

### **set port \<0-16\> ntxdesc \<0-16384\>**


设置指定端口的发送描述符数量。

**使用例：**
```bash
# 将端口0的发送描述符数量设置为1024
set port 0 ntxdesc 1024

# 将端口1的发送描述符数量设置为512
set port 1 ntxdesc 512
```

---

### **set port all ntxdesc \<0-16384\>**

设置所有端口的发送描述符数量。

**使用例：**
```bash
# 将所有端口的发送描述符数量设置为512
set port all ntxdesc 512
```

---

### **set port \<0-16\> link up**


使指定端口的链路上线。

**使用例：**
```bash
# 使端口0的链路上线
set port 0 link up

# 使端口1的链路上线
set port 1 link up
```

---

### **set port \<0-16\> link down**

使指定端口的链路下线。

**使用例：**
```bash
# 使端口0的链路下线
set port 0 link down

# 使端口1的链路下线
set port 1 link down
```

---

### **set port all link up**

使所有端口的链路上线。

**使用例：**
```bash
# 使所有端口的链路上线
set port all link up
```

---

### **set port all link down**

使所有端口的链路下线。

**使用例：**
```bash
# 使所有端口的链路下线
set port all link down
```

## 定义位置

这些命令在以下文件中定义：
- `sdplane/dpdk_port_cmd.c`

## 相关项目

- [工作者·lcore管理](worker-management.md)
- [系统信息·监视](system-monitoring.md)
- [队列配置](queue-configuration.md)