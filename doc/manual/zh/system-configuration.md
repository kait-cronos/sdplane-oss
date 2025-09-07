# 系统配置

**Language:** [English](../en/system-configuration.md) | [Japanese](../ja/system-configuration.md) | [Français](../fr/system-configuration.md) | **中文**

- **大页面**：为DPDK配置系统大页面
- **网络**：使用netplan进行网络接口配置
- **防火墙**：CLI需要telnet 9882/tcp端口

**⚠️ CLI没有身份验证。建议仅允许来自localhost的连接 ⚠️**

## 配置大页面
```bash
# 编辑GRUB配置
sudo vi /etc/default/grub

# 添加以下任一行：
# 对于2MB大页面（1536页面 = ~3GB）：
GRUB_CMDLINE_LINUX="hugepages=1536"

# 或对于1GB大页面（8页面 = 8GB）：
GRUB_CMDLINE_LINUX="default_hugepagesz=1G hugepagesz=1G hugepages=8"

# 更新GRUB并重启
sudo update-grub
sudo reboot
```

## 安装DPDK IGB内核模块（可选）

对于无法与vfio-pci一起工作的网卡，可选择安装igb_uio：

```bash
git clone http://dpdk.org/git/dpdk-kmods
cd dpdk-kmods/linux/igb_uio
make
sudo mkdir -p /lib/modules/`uname -r`/extra/dpdk/
sudo cp igb_uio.ko /lib/modules/`uname -r`/extra/dpdk/
echo igb_uio | sudo tee /etc/modules-load.d/igb_uio.conf
```

## 配置DPDK UIO驱动程序

DPDK需要用户I/O（UIO）驱动程序来从用户应用程序访问网络接口。

### 标准网卡

对于大多数标准网卡，您有三种UIO驱动程序选项：

1. **vfio-pci**（推荐）
   - 最安全和现代的选项
   - 需要IOMMU支持（Intel VT-d或AMD-Vi）
   - 无需额外的内核模块编译

2. **uio_pci_generic**
   - Linux内核中包含的通用UIO驱动程序
   - 无需额外安装
   - 功能相比vfio-pci有限

3. **igb_uio**
   - DPDK特定的UIO驱动程序
   - 需要手动编译和安装（见上文）
   - 为较旧硬件提供额外功能

### 具有分叉驱动程序的网卡

某些网卡，如**Mellanox ConnectX系列**，提供分叉驱动程序，允许同一物理端口被内核网络和DPDK应用程序同时使用。对于这些网卡：

- 不需要UIO驱动程序配置
- 网卡可以保持绑定到其内核驱动程序
- DPDK应用程序可以直接访问硬件

### 配置UIO驱动程序

要配置UIO驱动程序，您需要：

1. **加载驱动程序模块**：
```bash
# 对于vfio-pci（需要在BIOS/UEFI和内核中启用IOMMU）
sudo modprobe vfio-pci

# 对于uio_pci_generic
sudo modprobe uio_pci_generic

# 对于igb_uio（安装后）
sudo modprobe igb_uio
```

2. **通过在`/etc/modules-load.d/`中创建配置文件使驱动程序在启动时加载**：
```bash
# vfio-pci示例
echo "vfio-pci" | sudo tee /etc/modules-load.d/vfio-pci.conf

# uio_pci_generic示例
echo "uio_pci_generic" | sudo tee /etc/modules-load.d/uio_pci_generic.conf

# igb_uio示例
echo "igb_uio" | sudo tee /etc/modules-load.d/igb_uio.conf
```

3. **使用DPDK的`dpdk-devbind.py`脚本将网卡绑定到UIO驱动程序**：
```bash
# 从内核驱动程序解绑并绑定到UIO驱动程序
sudo dpdk-devbind.py --bind=vfio-pci 0000:01:00.0
sudo dpdk-devbind.py --bind=uio_pci_generic 0000:01:00.1
sudo dpdk-devbind.py --bind=igb_uio 0000:01:00.2
```

**注意**：将`0000:01:00.0`替换为您实际的PCI设备地址。使用`lspci`或`dpdk-devbind.py --status`来识别您的网卡。

**替代方案**：您可以在sdplane.conf文件中使用`set device XX:XX.X driver ...`命令在配置文件开头配置设备绑定，而不是手动使用`dpdk-devbind.py`。这允许sdplane在启动时自动管理设备绑定。