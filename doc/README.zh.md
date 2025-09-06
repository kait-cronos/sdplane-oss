<div align="center">
<img src="../sdplane-logo.png" alt="sdplane-oss Logo" width="160">
</div>

# sdplane-oss (软件数据平面)

由可以交互式控制DPDK线程操作的Shell和DPDK线程执行环境（sd-plane）组成的"DPDK-dock开发环境"

**Language:** [English](../README.md) | [日本語](README.ja.md) | [Français](README.fr.md) | **中文** | [Deutsch](README.de.md) | [Italiano](README.it.md)

## 特点

- **高性能数据包处理**：
  利用DPDK的零拷贝、用户空间数据包处理
- **二层/三层转发**：
  集成ACL、LPM、FIB支持的L2·L3转发
- **数据包生成**：
  用于测试和基准测试的内置数据包生成器
- **网络虚拟化**：
  TAP接口支持和VLAN交换功能
- **CLI管理**：
  用于配置和监视的交互式命令行界面
- **多线程**：
  基于核心工作线程的协作线程模型

### 架构
- **主应用程序**：核心路由器逻辑和初始化
- **DPDK模块**：L2/L3转发和数据包生成
- **CLI系统**：带补全和帮助功能的命令行界面
- **线程**：基于lthread的协作多任务
- **虚拟化**：TAP接口和虚拟交换

## 支持系统

### 软件要求
- **操作系统**：
  Ubuntu 24.04 LTS（当前支持）
- **网卡**：
  [驱动程序](https://doc.dpdk.org/guides/nics/) | [支持的网卡](https://core.dpdk.org/supported/)
- **内存**：
  需要大页面支持
- **CPU**：
  推荐多核处理器

### 目标硬件平台

本项目已在以下平台测试通过：
- **Topton (N305/N100)**：搭载10G网卡的迷你PC
- **Partaker (J3160)**：搭载1G网卡的迷你PC
- **Intel通用PC**：搭载Intel x520 / Mellanox ConnectX5
- **其他CPU**：应该可以在AMD、ARM CPU等上运行

## 1. 依赖项安装

### 依赖项
- **liburcu-qsbr**：用户空间RCU库
- **libpcap**：数据包捕获库
- **lthread**：[yasuhironet/lthread](https://github.com/yasuhironet/lthread)（轻量级协作线程）
- **DPDK**：数据平面开发工具包

### 安装sdplane依赖debian包
```bash
sudo apt install liburcu-dev libpcap-dev
```

### 安装构建工具和DPDK前提包

```bash
# 基本构建工具
sudo apt install build-essential cmake autotools-dev autoconf automake libtool pkg-config

# DPDK前提包
sudo apt install python3 python3-pip meson ninja-build python3-pyelftools libnuma-dev pkgconf
```

### 安装lthread
```bash
# 安装lthread
git clone https://github.com/yasuhironet/lthread
cd lthread
cmake .
make
sudo make install
```

### 安装DPDK 23.11.1
```bash
# 下载和解压DPDK
wget https://fast.dpdk.org/rel/dpdk-23.11.1.tar.xz
tar vxJf dpdk-23.11.1.tar.xz
cd dpdk-stable-23.11.1

# 构建和安装DPDK
meson setup build
cd build
ninja
sudo meson install
sudo ldconfig

# 验证安装
pkg-config --modversion libdpdk
```

## 2. Intel Core i3-n305/Celelon j3160的Debian包快速启动

对于Intel Core i3-n305/Celelon j3160，可以使用Debian包进行快速安装。

下载并安装预构建的Debian包：

```bash
# 下载n305的最新包
wget https://www.yasuhironet.net/download/n305/sdplane_0.1.4-36_amd64.deb
wget https://www.yasuhironet.net/download/n305/sdplane-dbgsym_0.1.4-36_amd64.ddeb

# 或者j3160的包
wget https://www.yasuhironet.net/download/j3160/sdplane_0.1.4-35_amd64.deb
wget https://www.yasuhironet.net/download/j3160/sdplane-dbgsym_0.1.4-35_amd64.ddeb

# 安装包
sudo apt install ./sdplane_0.1.4-*_amd64.deb
sudo apt install ./sdplane-dbgsym_0.1.4-*_amd64.ddeb
```

**注意**：请查看[yasuhironet.net下载](https://www.yasuhironet.net/download/)获取最新包版本。

跳转到5. 系统配置。

## 3. 从源码构建

**一般情况下，请按照这个步骤进行。**

### 安装Ubuntu前提包

#### 用于从源码构建的可选包
```bash
sudo apt install etckeeper tig bridge-utils \
                 iptables-persistent fail2ban dmidecode screen ripgrep
```

### 从源码构建sdplane-oss

```bash
# 生成构建文件
./autogen.sh

# 配置和构建
mkdir build
cd build
CFLAGS="-g -O0" sh ../configure
make
```

## 4. 创建和安装sdplane-oss Debian包

### 安装前提包
```bash
sudo apt install build-essential cmake devscripts debhelper
```

### 构建sdplane-oss Debian包
```bash
# 首先确保从干净空间开始
(cd build && make distclean)
make distclean

# 从源码构建Debian包
bash build-debian.sh

# 安装生成的包（将在父目录中生成）
sudo apt install ../sdplane_*.deb
```

## 5. 系统配置

- **大页面**：为DPDK配置系统大页面
- **网络**：使用netplan进行网络接口配置
- **防火墙**：CLI需要telnet 9882/tcp端口

**⚠️ CLI没有身份验证。建议仅允许来自localhost的连接 ⚠️**

### 配置大页面
```bash
# 编辑GRUB配置
sudo vi /etc/default/grub

# 在GRUB_CMDLINE_LINUX参数中添加大页面
# 例如添加hugepages=1024：
GRUB_CMDLINE_LINUX="hugepages=1024"

# 更新GRUB
sudo update-grub

# 重启系统
sudo reboot

# 重启后验证大页面
cat /proc/meminfo | grep -E "^HugePages|^Hugepagesize"
```

### 安装DPDK IGB内核模块（可选）

如果您的网卡无法与vfio-pci一起工作，请安装igb_uio。

```bash
git clone http://dpdk.org/git/dpdk-kmods
cd dpdk-kmods/linux/igb_uio
make
sudo make install
cd ../../..

# 模块将安装到/lib/modules/$(uname -r)/extra/igb_uio.ko
```

## 6. sdplane配置

### 配置文件

sdplane使用配置文件来定义启动行为和执行环境。

#### 操作系统配置示例（`etc/`）
- `systemd/sdplane.service`：systemd服务文件
- `netplan/01-netcfg.yaml`：使用netplan的网络配置

#### 应用程序配置示例（`example-config/`）
- 各种应用程序的示例配置文件
- 启动脚本和配置配置文件

## 7. 使用sdplane-oss运行应用程序

```bash
# 基本执行
sudo ./sdplane/sdplane

# 使用配置文件执行
sudo ./sdplane/sdplane -f /path/to/config-file

# 连接到CLI（从另一个终端）
telnet localhost 9882

# 交互式shell现在可用
sdplane> help
sdplane> show version
```

## 提示

### 使用vfio-pci作为网卡驱动时需要IOMMU

必须启用虚拟化功能：
- Intel：Intel VT-d
- AMD：AMD IOMMU / AMD-V

这些选项必须在BIOS设置中启用。
GRUB配置可能也需要修改：

```conf
# /etc/default/grub
GRUB_CMDLINE_LINUX="iommu=pt intel_iommu=on"
```

应用修改：
```bash
sudo update-grub
sudo reboot
```

### 永久加载vfio-pci Linux内核模块的配置

```bash
# 创建自动加载配置文件
sudo tee /etc/modules-load.d/vfio-pci.conf > /dev/null <<EOF
vfio-pci
EOF
```

### Mellanox ConnectX系列的情况

需要从以下链接安装驱动程序：

https://network.nvidia.com/products/ethernet-drivers/linux/mlnx_en/

安装时请执行`./install --dpdk`。
**`--dpdk`选项是必需的。**

请在sdplane.conf中注释以下参数，因为它们不是必需的：

```conf
#set device {pcie-id} driver unbound
#set device {pcie-id} driver {driver-name} driver_override
#set device {pcie-id} driver {driver-name} bind
```

对于Mellanox网卡，您必须执行`update port status`命令来刷新端口信息。

### 如何检查PCIe总线编号

您可以使用DPDK中的dpdk-devbind.py命令来检查网卡的PCIe总线编号：

```bash
# 显示网络设备状态
dpdk-devbind.py -s

# 示例输出：
Network devices using kernel driver
===================================
0000:04:00.0 'NetXtreme BCM5720 Gigabit Ethernet PCIe 165f' numa_node=0 if=eno8303 drv=tg3 unused= *Active*
0000:b1:00.0 'MT27800 Family [ConnectX-5] 1017' numa_node=1 if=enp177s0np0 drv=mlx5_core unused= *Active*
```

### 配置文件中的工作线程顺序

如果配置`rib-manager`、`neigh-manager`和`netlink-thread`工作线程，如果使用它们，必须按此顺序配置。

### 关于DPDK初始化

应该从配置文件中只调用一个调用`rte_eal_init()`的命令。`rte_eal_init()`函数由诸如`rte_eal_init`、`pktgen init`、`l2fwd init`和`l3fwd init`等命令调用。

## 用户指南（手册）

提供了完整的用户指南和命令参考：

- [用户指南](manual/zh/README.md) - 完整概述和命令分类

**应用程序指南：**
- [L2中继器应用程序](manual/zh/l2-repeater-application.md) - 使用MAC学习的简单第二层数据包传输
- [增强中继器应用程序](manual/zh/enhanced-repeater-application.md) - 带TAP接口的VLAN交换
- [数据包生成器应用程序](manual/zh/packet-generator-application.md) - 高性能流量生成和测试

**配置指南：**
- [RIB和路由](manual/zh/routing.md) - RIB和路由功能
- [增强中继器](manual/zh/enhanced-repeater.md) - 增强中继器配置
- [端口管理和统计](manual/zh/port-management.md) - DPDK端口管理和统计
- [工作线程和lcore管理](manual/zh/worker-management.md) - 工作线程和lcore管理
- [系统信息和监视](manual/zh/system-monitoring.md) - 系统信息和监视
- [设备管理](manual/zh/device-management.md) - 设备和驱动程序管理

**开发人员指南：**
- [DPDK集成指南](manual/zh/dpdk-integration-guide.md) - 如何集成DPDK应用程序
- [调试和日志记录](manual/zh/debug-logging.md) - 调试和日志记录功能
- [队列配置](manual/zh/queue-configuration.md) - 队列配置和管理
- [TAP接口](manual/zh/tap-interface.md) - TAP接口管理
- [VTY和Shell管理](manual/zh/vty-shell.md) - VTY和shell管理
- [线程信息](manual/zh/thread-information.md) - 线程信息和监视
- [lthread管理](manual/zh/lthread-management.md) - 轻量级协作线程管理
- [数据包生成](manual/zh/packet-generation.md) - 使用PKTGEN进行数据包生成

## 开发人员指南

### 集成指南
- [DPDK集成指南](manual/zh/dpdk-integration-guide.md) - 如何将您的DPDK应用程序与sdplane-oss集成

### 文档
- 所有开发人员文档都包含在`doc/`中
- 集成指南和示例在`doc/manual/zh/`中

### 代码风格

本项目遵循GNU编码标准。要检查和格式化代码：

```bash
# 检查风格（需要clang-format 18.1.3+）
./style/check_gnu_style.sh check

# 自动格式化代码
./style/check_gnu_style.sh update

# 显示差异
./style/check_gnu_style.sh diff
```

安装所需工具：
```bash
# 对于Ubuntu 24.04
sudo apt install clang-format-18
```

## 许可证

本项目采用Apache 2.0许可证 - 详见[LICENSE](LICENSE)文件。

## 联系方式

- GitHub：https://github.com/kait-cronos/sdplane-oss
- Issues：https://github.com/kait-cronos/sdplane-oss/issues

## 评估设备购买

评估设备可能包括附加功能和软件修改。

有关评估设备的询问，请通过GitHub Issues或直接通过电子邮件联系我们。