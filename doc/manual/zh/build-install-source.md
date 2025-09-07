# 从源码构建和安装

**Language:** [English](../en/build-install-source.md) | [Japanese](../ja/build-install-source.md) | [Français](../fr/build-install-source.md) | **中文**

**一般情况下，请按照这个步骤进行。**

## 安装Ubuntu前提包

### 用于从源码构建
```bash
# 基本构建工具
sudo apt install build-essential cmake autotools-dev autoconf automake libtool pkg-config

# DPDK前提包
sudo apt install python3 python3-pip meson ninja-build python3-pyelftools libnuma-dev pkgconf
```

### 可选包
```bash
sudo apt install etckeeper tig bridge-utils \
                 iptables-persistent fail2ban dmidecode screen ripgrep
```

## 从源码构建sdplane-oss

```bash
# 克隆仓库
git clone https://github.com/kait-cronos/sdplane-oss
cd sdplane-oss

# 生成构建文件
sh autogen.sh

# 配置和构建
mkdir build
cd build
CFLAGS="-g -O0" sh ../configure
make

# 安装到系统
# 这将安装到$prefix（默认：/usr/local/sbin）
sudo make install
```