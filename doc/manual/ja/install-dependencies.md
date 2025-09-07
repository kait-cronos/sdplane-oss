# 依存関係のインストール

## 依存関係
- **liburcu-qsbr**：ユーザー空間RCUライブラリ
- **libpcap**：パケットキャプチャライブラリ
- **lthread**：[yasuhironet/lthread](https://github.com/yasuhironet/lthread)（軽量協調スレッド）
- **DPDK**：Data Plane Development Kit

## sdplane依存関係debianパッケージのインストール
```bash
sudo apt install liburcu-dev libpcap-dev
```

## ビルドツールとDPDK前提パッケージのインストール

```bash
# コアビルドツール
sudo apt install build-essential cmake autotools-dev autoconf automake libtool pkg-config

# DPDK前提パッケージ
sudo apt install python3 python3-pip meson ninja-build python3-pyelftools libnuma-dev pkgconf
```

## lthreadのインストール
```bash
# lthreadのインストール
git clone https://github.com/yasuhironet/lthread
cd lthread
cmake .
make
sudo make install
```
## DPDK 23.11.1のインストール
```bash
# DPDKのダウンロードと展開
wget https://fast.dpdk.org/rel/dpdk-23.11.1.tar.xz
tar vxJf dpdk-23.11.1.tar.xz
cd dpdk-stable-23.11.1

# DPDKのビルドとインストール
meson setup build
cd build
ninja
sudo meson install
sudo ldconfig

# インストールの確認
pkg-config --modversion libdpdk
# 出力例: 23.11.1
```