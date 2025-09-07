# ソースからのビルド・インストール

**一般的にはこちらの手順を踏んでください。**

## 前提となるUbuntuパッケージのインストール

### ソースビルド用
```bash
# コアビルドツール
sudo apt install build-essential cmake autotools-dev autoconf automake libtool pkg-config

# DPDK前提パッケージ
sudo apt install python3 python3-pip meson ninja-build python3-pyelftools libnuma-dev pkgconf
```

### オプションパッケージ
```bash
sudo apt install etckeeper tig bridge-utils \
                 iptables-persistent fail2ban dmidecode screen ripgrep
```

## ソースからsdplane-ossのビルド

```bash
# リポジトリのクローン
git clone https://github.com/kait-cronos/sdplane-oss
cd sdplane-oss

# ビルドファイルの生成
sh autogen.sh

# 設定とビルド
mkdir build
cd build
CFLAGS="-g -O0" sh ../configure
make

# システムへのインストール
# $prefix (デフォルト: /usr/local/sbin) にインストールされます
sudo make install
```