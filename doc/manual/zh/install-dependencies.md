# 依赖项安装

**Language:** [English](../en/install-dependencies.md) | [日本語](../ja/install-dependencies.md) | [Français](../fr/install-dependencies.md) | **中文** | [Deutsch](../de/install-dependencies.md) | [Italiano](../it/install-dependencies.md) | [한국어](../ko/install-dependencies.md) | [ไทย](../th/install-dependencies.md) | [Español](../es/install-dependencies.md)

## 依赖项

sdplane-oss需要以下组件：
- **lthread** (yasuhironet/lthread)：轻量级协作线程
- **liburcu-qsbr**：用户空间RCU库
- **libpcap**：数据包捕获库
- **DPDK 23.11.1**：数据平面开发工具包

## 安装sdplane依赖debian包

```bash
sudo apt update && sudo apt install liburcu-dev libpcap-dev
```

## 安装构建工具和DPDK前提包

```bash
sudo apt install build-essential cmake autotools-dev autoconf automake \
                 libtool pkg-config python3 python3-pip meson ninja-build \
                 python3-pyelftools libnuma-dev pkgconf
```

## 安装lthread

```bash
git clone https://github.com/yasuhironet/lthread
cd lthread
cmake .
make
sudo make install
cd ..
```

## 安装DPDK 23.11.1

```bash
# 下载DPDK 23.11.1
wget https://fast.dpdk.org/rel/dpdk-23.11.1.tar.xz
tar xf dpdk-23.11.1.tar.xz
cd dpdk-23.11.1

# 构建并安装DPDK
meson setup -Dprefix=/usr/local build
cd build
ninja install
cd ../..

# 验证安装
pkg-config --modversion libdpdk
# 应显示：23.11.1
```