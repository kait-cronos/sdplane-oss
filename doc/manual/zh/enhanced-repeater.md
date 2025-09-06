# 增强中继器配置

增强中继器提供高级VLAN交换功能，支持用于L3路由和包捕获的TAP接口。它支持创建虚拟交换机、使用VLAN标记链接DPDK端口、用于内核集成的路由器接口以及用于监视的捕获接口。

## 虚拟交换机命令

### set vswitch
```
set vswitch <1-4094>
```
**描述**: 用指定的VLAN ID创建虚拟交换机

**参数**:
- `<1-4094>`: 虚拟交换机的VLAN ID

**示例**:
```
set vswitch 2031
set vswitch 2032
```

### delete vswitch
```
delete vswitch <0-3>
```
**描述**: 删除指定ID的虚拟交换机

**参数**:
- `<0-3>`: 要删除的虚拟交换机ID

**示例**:
```
delete vswitch 0
```

### show vswitch_rib
```
show vswitch_rib
```
**描述**: 显示包含配置和状态的虚拟交换机RIB信息

## 虚拟交换机链接命令

### set vswitch-link
```
set vswitch-link vswitch <0-3> port <0-7> tag <0-4094>
```
**描述**: 使用VLAN标记配置将DPDK端口链接到虚拟交换机

**参数**:
- `vswitch <0-3>`: 虚拟交换机ID (0-3)
- `port <0-7>`: DPDK端口ID (0-7)  
- `tag <0-4094>`: VLAN标记ID (0: 原生/无标记，1-4094: 标记VLAN)

**示例**:
```
# 将端口0链接到虚拟交换机0，使用VLAN标记2031
set vswitch-link vswitch 0 port 0 tag 2031

# 将端口0链接到虚拟交换机1，原生/无标记
set vswitch-link vswitch 1 port 0 tag 0
```

### delete vswitch-link
```
delete vswitch-link <0-7>
```
**描述**: 删除指定ID的虚拟交换机链接

**参数**:
- `<0-7>`: 虚拟交换机链接ID

### show vswitch-link
```
show vswitch-link
```
**描述**: 显示所有虚拟交换机链接配置

## 路由器接口命令

### set vswitch router-if
```
set vswitch <1-4094> router-if <WORD>
```
**描述**: 为指定虚拟交换机创建用于L3连接的路由器接口

**参数**:
- `<1-4094>`: 虚拟交换机的VLAN ID
- `<WORD>`: TAP接口名称

**示例**:
```
set vswitch 2031 router-if rif2031
set vswitch 2032 router-if rif2032
```

### no set vswitch router-if
```
no set vswitch <1-4094> router-if
```
**描述**: 从指定虚拟交换机删除路由器接口

**参数**:
- `<1-4094>`: 虚拟交换机的VLAN ID

### show rib vswitch router-if
```
show rib vswitch router-if
```
**描述**: 显示包含MAC地址、IP地址和接口状态的路由器接口配置

## 捕获接口命令

### set vswitch capture-if
```
set vswitch <1-4094> capture-if <WORD>
```
**描述**: 为指定虚拟交换机创建用于包监视的捕获接口

**参数**:
- `<1-4094>`: 虚拟交换机的VLAN ID
- `<WORD>`: TAP接口名称

**示例**:
```
set vswitch 2031 capture-if cif2031
set vswitch 2032 capture-if cif2032
```

### no set vswitch capture-if
```
no set vswitch <1-4094> capture-if
```
**描述**: 从指定虚拟交换机删除捕获接口

**参数**:
- `<1-4094>`: 虚拟交换机的VLAN ID

### show rib vswitch capture-if
```
show rib vswitch capture-if
```
**描述**: 显示捕获接口配置

## VLAN处理功能

增强中继器执行高级VLAN处理：

- **VLAN转换**: 基于vswitch-link配置更改VLAN ID
- **VLAN删除**: 当标记设置为0（原生）时删除VLAN头  
- **VLAN插入**: 将无标记包转发到标记端口时添加VLAN头
- **分割水平**: 通过不将包发送回接收端口来防止环路

## 配置示例

```bash
# 创建虚拟交换机
set vswitch 2031
set vswitch 2032

# 将DPDK端口链接到带VLAN标记的虚拟交换机
set vswitch-link vswitch 0 port 0 tag 2031
set vswitch-link vswitch 1 port 0 tag 2032

# 为L3处理创建路由器接口
set vswitch 2031 router-if rif2031
set vswitch 2032 router-if rif2032

# 为监视创建捕获接口
set vswitch 2031 capture-if cif2031
set vswitch 2032 capture-if cif2032

# 配置工作者使用增强中继器
set worker lcore 1 enhanced-repeater
```

## TAP接口集成

路由器接口和捕获接口创建与Linux内核网络栈集成的TAP接口：

- **路由器接口**: 启用L3路由、IP地址分配和内核网络处理
- **捕获接口**: 启用包监视、分析和调试
- **环形缓冲区**: 使用DPDK环形在数据平面和内核之间进行高效包传输