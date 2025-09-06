# RIB·路由

**语言 / Language:** [English](../routing.md) | [日本語](../ja/routing.md) | [Français](../fr/routing.md) | **中文** | [Deutsch](../de/routing.md) | [Italiano](../it/routing.md) | [한국어](../ko/routing.md) | [ไทย](../th/routing.md) | [Español](../es/routing.md)

管理RIB（Routing Information Base）和系统资源信息的命令。

## 命令一览

### show_rib - RIB信息显示
```
show rib
```

显示RIB（Routing Information Base）信息。

**使用例：**
```bash
show rib
```

该命令显示以下信息：
- RIB版本和内存指针
- 虚拟交换机配置和VLAN分配
- DPDK端口状态和队列配置
- lcore到端口队列分配
- L2/L3转发用的邻居表

## RIB概要

### RIB是什么
RIB（Routing Information Base）是存储系统资源和网络信息的中央数据库。在sdplane中，管理以下信息：

- **虚拟交换机配置** - VLAN交换和端口分配
- **DPDK端口信息** - 链路状态、队列配置、功能信息
- **lcore队列配置** - 每个CPU核心的包处理分配
- **邻居表** - L2/L3转发数据库条目

### RIB结构
RIB由两个主要结构体组成：

```c
struct rib {
    struct rib_info *rib_info;  // 指向实际数据的指针
};

struct rib_info {
    uint32_t ver;                                    // 版本号
    uint8_t vswitch_size;                           // 虚拟交换机数
    uint8_t port_size;                              // DPDK端口数
    uint8_t lcore_size;                             // lcore数
    struct vswitch_conf vswitch[MAX_VSWITCH];       // 虚拟交换机配置
    struct vswitch_link vswitch_link[MAX_VSWITCH_LINK]; // VLAN端口链接
    struct port_conf port[MAX_ETH_PORTS];           // DPDK端口配置
    struct lcore_qconf lcore_qconf[RTE_MAX_LCORE];  // lcore队列分配
    struct neigh_table neigh_tables[NEIGH_NR_TABLES]; // 邻居/转发表
};
```

## RIB信息的查看方法

### 基本显示项目
- **Destination** - 目标网络地址
- **Netmask** - 网络掩码
- **Gateway** - 网关（下一跳）
- **Interface** - 输出接口
- **Metric** - 路由的度量值
- **Status** - 路由状态

### 路由状态
- **Active** - 活动路由
- **Inactive** - 非活动路由
- **Pending** - 配置中的路由
- **Invalid** - 无效路由

## 使用例

### 基本使用方法
```bash
# 显示RIB信息
show rib
```

### 输出例解释
```
rib information version: 21 (0x55555dd42010)
vswitches: 
dpdk ports: 
  dpdk_port[0]: 
    link: speed=1000Mbps duplex=full autoneg=on status=up
    nb_rxd=1024 nb_txd=1024
    queues: nrxq=1 ntxq=4
  dpdk_port[1]: 
    link: speed=0Mbps duplex=half autoneg=on status=down
    nb_rxd=1024 nb_txd=1024
    queues: nrxq=1 ntxq=4
  dpdk_port[2]: 
    link: speed=0Mbps duplex=half autoneg=off status=down
    nb_rxd=1024 nb_txd=1024
    queues: nrxq=1 ntxq=4
lcores: 
  lcore[0]: nrxq=0
  lcore[1]: nrxq=1
    rxq[0]: dpdk_port[0], queue_id=0
  lcore[2]: nrxq=1
    rxq[0]: dpdk_port[1], queue_id=0
  lcore[3]: nrxq=1
    rxq[0]: dpdk_port[2], queue_id=0
  lcore[4]: nrxq=0
  lcore[5]: nrxq=0
  lcore[6]: nrxq=0
  lcore[7]: nrxq=0
```

这个例子中：
- RIB版本21显示当前系统状态
- DPDK端口0活动（up），链路速度1Gbps
- DPDK端口1、2非活动（down），无链路
- lcore 1、2、3分别负责处理来自端口0、1、2的包
- 每个端口使用1个RX队列和4个TX队列
- RX/TX描述符环设置为1024条目

## RIB管理

### 自动更新
RIB在以下时机自动更新：
- 接口状态变更
- 网络配置变更
- 从路由协议接收信息

### 手动更新
手动确认RIB信息：
```bash
show rib
```

## 故障排除

### 路由无法正常工作时
1. 确认RIB信息
```bash
show rib
```

2. 确认接口状态
```bash
show port
```

3. 确认工作者状态
```bash
show worker
```

### RIB中不显示路由时
- 确认网络配置
- 确认接口状态
- 确认RIB管理器运行

## 高级功能

### RIB管理器
RIB管理器作为独立线程运行，提供以下功能：
- 路由信息自动更新
- 路由有效性检查
- 网络状态监视

### 相关工作者
- **rib-manager** - 执行RIB管理的工作者
- **l3fwd** - 在Layer 3转发中使用RIB
- **l3fwd-lpm** - LPM表与RIB的协作

## 定义位置

该命令在以下文件中定义：
- `sdplane/rib.c`

## 相关项目

- [工作者·lcore管理](worker-management.md)
- [lthread管理](lthread-management.md)
- [系统信息·监视](system-monitoring.md)