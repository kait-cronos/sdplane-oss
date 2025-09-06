<div align="center">
<img src="../sdplane-logo.png" alt="sdplane-oss Logo" width="160">
</div>

# sdplane-oss (ソフトデータプレーン)

DPDKスレッドの動作を対話的に制御できるShellと、DPDKスレッド実行環境（sd-plane）で構成された「DPDK-dock開発環境」

**Language:** [English](../README.md) | **日本語** | [Français](README.fr.md) | [中文](README.zh.md) | [Deutsch](README.de.md) | [Italiano](README.it.md) | [한국어](README.ko.md) | [ไทย](README.th.md) | [Español](README.es.md)

## 特徴

- **高性能パケット処理**：
  DPDKを活用したゼロコピー、ユーザー空間パケット処理
- **レイヤー2/3フォワーディング**：
  ACL、LPM、FIBサポートを統合したL2・L3フォワーディング
- **パケット生成**：
  テストとベンチマーク用の内蔵パケットジェネレーター
- **ネットワーク仮想化**：
  TAPインターフェースサポートとVLANスイッチング機能
- **CLI管理**：
  設定と監視のためのインタラクティブコマンドラインインターフェース
- **マルチスレッド**：
  コア別ワーカーによる協調スレッドモデル

### アーキテクチャ
- **メインアプリケーション**：コアルーターロジックと初期化
- **DPDKモジュール**：L2/L3フォワーディングとパケット生成
- **CLIシステム**：補完とヘルプ機能付きコマンドラインインターフェース
- **スレッド**：lthreadベースの協調マルチタスク
- **仮想化**：TAPインターフェースと仮想スイッチング

## サポートシステム

### ソフトウェア要件
- **OS**：
  Ubuntu 24.04 LTS（現在サポート中）
- **NIC**：
  [ドライバー](https://doc.dpdk.org/guides/nics/) | [サポートNIC](https://core.dpdk.org/supported/)
- **メモリ**：
  ヒュージページサポートが必要
- **CPU**：
  マルチコアプロセッサ推奨

### 対象ハードウェアプラットフォーム

本プロジェクトは以下でテスト済みです：
- **Topton (N305/N100)**：10G NIC搭載ミニPC
- **Partaker (J3160)**：1G NIC搭載ミニPC
- **Intel汎用PC**：Intel x520 / Mellanox ConnectX5搭載
- **その他のCPU**：AMD、ARM CPU等でも動作するはずです。

## 1. 依存関係のインストール

### 依存関係
- **liburcu-qsbr**：ユーザー空間RCUライブラリ
- **libpcap**：パケットキャプチャライブラリ
- **lthread**：[yasuhironet/lthread](https://github.com/yasuhironet/lthread)（軽量協調スレッド）
- **DPDK**：Data Plane Development Kit

### sdplane依存関係debianパッケージのインストール
```bash
sudo apt install liburcu-dev libpcap-dev
```

### ビルドツールとDPDK前提パッケージのインストール

```bash
# コアビルドツール
sudo apt install build-essential cmake autotools-dev autoconf automake libtool pkg-config

# DPDK前提パッケージ
sudo apt install python3 python3-pip meson ninja-build python3-pyelftools libnuma-dev pkgconf
```

### lthreadのインストール
```bash
# lthreadのインストール
git clone https://github.com/yasuhironet/lthread
cd lthread
cmake .
make
sudo make install
```
### DPDK 23.11.1のインストール
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

## 2. Intel Core i3-n305/Celelon j3160向け Debianパッケージによるクイックスタート

Intel Core i3-n305/Celelon j3160では、Debianパッケージによるクイックスタートが可能です。

ビルド済みDebianパッケージをダウンロード・インストールします：

```bash
# 最新パッケージのダウンロード (n305用)
wget https://www.yasuhironet.net/download/n305/sdplane_0.1.4-36_amd64.deb
wget https://www.yasuhironet.net/download/n305/sdplane-dbgsym_0.1.4-36_amd64.ddeb

# もしくは (j3160用)
wget https://www.yasuhironet.net/download/j3160/sdplane_0.1.4-35_amd64.deb
wget https://www.yasuhironet.net/download/j3160/sdplane-dbgsym_0.1.4-35_amd64.ddeb

# パッケージのインストール
sudo apt install ./sdplane_0.1.4-*_amd64.deb
sudo apt install ./sdplane-dbgsym_0.1.4-*_amd64.ddeb
```

**注意**: 最新パッケージバージョンについては [yasuhironet.net ダウンロード](https://www.yasuhironet.net/download/)を確認してください。

5. システム設定 にジャンプしてください。

## 3. ソースからのビルド

**一般的にはこちらの手順を踏んでください。**

<!--
#### オプションパッケージ
```bash
sudo apt install etckeeper tig bridge-utils \
                 iptables-persistent fail2ban dmidecode screen ripgrep
```
-->

### ソースからsdplane-ossのビルド

```bash
# リポジトリのクローン
cd
git clone https://github.com/kait-cronos/sdplane-oss
cd sdplane-oss

# ビルドファイルの生成
sh autogen.sh

# 設定とビルド
mkdir build
cd build
CFLAGS="-g -O0" sh ../configure
make

# prefix (/usr/local/sbin) へのインストール
make install
```

## 4. sdplane-ossのDebian Packageの作成とインストール

### 前提パッケージのインストール
```bash
sudo apt install build-essential cmake devscripts debhelper
```

### sdplane-oss Debianパッケージのビルド
```bash
# まずクリーンな状態から始める
(cd ~/sdplane-oss/build && make distclean)
(cd ~/sdplane-oss && make distclean)

# ソースからDebianパッケージをビルド
bash build-debian.sh

# 生成されたパッケージをインストール（親ディレクトリに生成される）
sudo apt install ../sdplane_*.deb
```

## 5. システム設定

- **ヒュージページ**：DPDK用システムヒュージページの設定
- **ネットワーク**：ネットワークインターフェース設定にnetplanを使用
- **ファイアウォール**： CLIのために telnet 9882/tcp portが必要 

**⚠️ CLIに認証がありません。localhostからのみ接続を許可することを推奨 ⚠️**

### ヒュージページの設定
```bash
# GRUB設定の編集
sudo vi /etc/default/grub

# 以下のいずれかの行を追加:
# 2MBヒュージページの場合 (1536ページ = 約3GB):
GRUB_CMDLINE_LINUX="hugepages=1536"

# または1GBヒュージページの場合 (8ページ = 8GB):
# (8GB未満のRAMの場合、hugepages=4などに調整してください.)
GRUB_CMDLINE_LINUX="default_hugepagesz=1G hugepagesz=1G hugepages=8"

# GRUBを更新して再起動
sudo update-grub
sudo reboot
```

### DPDK IGBカーネルモジュールのインストール（オプション）

vfio-pciで動作しないNICの場合は、オプションでigb_uioをインストールしてください。

```bash
git clone http://dpdk.org/git/dpdk-kmods
cd dpdk-kmods/linux/igb_uio
make
sudo mkdir -p /lib/modules/`uname -r`/extra/dpdk/
sudo cp igb_uio.ko /lib/modules/`uname -r`/extra/dpdk/
echo igb_uio | sudo tee /etc/modules-load.d/igb_uio.conf
```

## 6. sdplane設定

### 設定ファイル

Debian Packageからインストールした場合、`/etc/sdplane/sdplane.conf.sample`やsystemd service fileが自動的に生成されます。

サンプルを参考に `/etc/sdplane/sdplane.conf`を作成してください。

#### OS設定例（`etc/`）

- [`etc/sdplane.conf.sample`](etc/sdplane.conf.sample)：メイン設定テンプレート
- [`etc/sdplane.service`](etc/sdplane.service)：systemdサービスファイル
- [`etc/modules-load.d/`](etc/modules-load.d/)：カーネルモジュール読み込み設定

#### アプリケーション設定例（`example-config/`）

- [`example-config/sdplane-pktgen.conf`](example-config/sdplane-pktgen.conf)：パケットジェネレーター設定
- [`example-config/sdplane-topton.conf`](example-config/sdplane-topton.conf)：Toptonハードウェア設定
- [`example-config/sdplane_l2_repeater.conf`](example-config/sdplane_l2_repeater.conf)：L2リピーター設定
- [`example-config/sdplane_enhanced_repeater.conf`](example-config/sdplane_enhanced_repeater.conf)：拡張リピーター設定（VLANスイッチング、ルーターインターフェース、キャプチャインターフェース）

## 7. sdplane-ossを用いたApplicationの実行

```bash
# フォアグラウンドで実行
sudo sdplane

# 設定ファイル指定で実行
sudo sdplane -f /etc/sdplane/sdplane_enhanced_repeater.conf

# aptでインストールした場合、systemd経由で実行
sudo systemctl enable sdplane
sudo systemctl start sdplane

# CLIに接続
telnet localhost 9882
```


## Tips

### vfio-pciをNICドライバーに使用する場合はIOMMUが必須

- Intel：Intel VT-d
- AMD：AMD IOMMU / AMD-V

上記をBIOSで有効にする必要があります。  
また、GRUBの設定変更が必要な場合があります。

```conf
# /etc/default/grub
GRUB_CMDLINE_LINUX="iommu=pt intel_iommu=on"
```

```bash
sudo update-grub
sudo reboot
```

### vfio-pci Linux Kernel Moduleを永続的にロードする設定

```conf
#/etc/modules-load.d/vfio-pci.conf
vfio-pci
```

### Mellanox ConnectXシリーズの場合

以下のリンクからドライバーのインストールが必要です。

https://network.nvidia.com/products/ethernet-drivers/linux/mlnx_en/

インストール時には、`./install --dpdk` を実行してください。  
**オプション `--dpdk` が必須です。**

以下の設定はsdplane.confでは不要なため、コメントアウトしてください。

```conf
#set device {pcie-id} driver unbound
#set device {pcie-id} driver {driver名} driver_override
#set device {pcie-id} driver {driver名} bind
```

メラノックスNICの場合は、ポート情報を更新するために、`update port status` コマンドを実行する必要があります。

### PCIeバス番号の確認方法

DPDKでは、dpdk-devbind.pyコマンドを使用してNICのPCIeバス番号を確認できます。

```bash
> dpdk-devbind.py -s     

Network devices using kernel driver
===================================
0000:04:00.0 'NetXtreme BCM5720 Gigabit Ethernet PCIe 165f' numa_node=0 if=eno8303 drv=tg3 unused= *Active*
0000:b1:00.0 'MT27800 Family [ConnectX-5] 1017' numa_node=1 if=enp177s0np0 drv=mlx5_core unused= *Active*
```

### 設定ファイル中のワーカースレッドの順序

`rib-manager`、`neigh-manager`、`netlink-thread` のワーカーを設定する場合、もし使用されるのであれば、この順序で設定されなければいけません。

### DPDK初期化に関して

`rte_eal_init()` を呼ぶコマンドは、設定ファイルからはどれか一つのみ呼び出すのが正しいです。`rte_eal_init()` 関数は、`rte_eal_init`、`pktgen init`、`l2fwd init`、`l3fwd init` などのコマンドから呼ばれます。

## ユーザーガイド（マニュアル）

詳細なユーザーガイドとコマンドリファレンスは以下をご覧ください：

- [ユーザーガイド](manual/ja/README.md) - 全機能の概要とコマンド分類

**アプリケーションガイド:**
- [L2リピーターアプリケーション](manual/ja/l2-repeater-application.md) - MAC学習機能付きシンプルレイヤー2パケット転送
- [拡張リピーターアプリケーション](manual/ja/enhanced-repeater-application.md) - TAPインターフェース付きVLAN対応スイッチング
- [パケットジェネレーターアプリケーション](manual/ja/packet-generator-application.md) - 高性能トラフィック生成とテスト

**設定ガイド:**
- [ポート管理・統計](manual/ja/port-management.md) - DPDKポートの管理と統計情報
- [ワーカー・lcore管理](manual/ja/worker-management.md) - ワーカースレッドとlcoreの管理
- [デバッグ・ログ](manual/ja/debug-logging.md) - デバッグとログ機能
- [VTY・シェル管理](manual/ja/vty-shell.md) - VTYとシェルの管理
- [システム情報・監視](manual/ja/system-monitoring.md) - システム情報と監視機能
- [RIB・ルーティング](manual/ja/routing.md) - RIBとルーティング機能
- [キュー設定](manual/ja/queue-configuration.md) - キューの設定と管理
- [パケット生成](manual/ja/packet-generation.md) - PKTGENを使用したパケット生成
- [スレッド情報](manual/ja/thread-information.md) - スレッドの情報と監視
- [TAPインターフェース](manual/ja/tap-interface.md) - TAPインターフェースの管理
- [lthread管理](manual/ja/lthread-management.md) - lthreadの管理
- [デバイス管理](manual/ja/device-management.md) - デバイスとドライバーの管理
- [拡張リピーター](manual/ja/enhanced-repeater.md) - 仮想スイッチング、VLAN処理、TAPインターフェース

## 開発者ガイド

### 統合ガイド

- [DPDKアプリケーション統合ガイド](manual/ja/dpdk-integration-guide.md) - DPDK-dock方式でDPDKアプリケーションをsdplaneに統合する方法

### ドキュメント

- [技術プレゼンテーション/2024-11-22-sdn-onsen-yasu.pdf)](https://enog.jp/wordpress/wp-content/uploads/2024/11/2024-11-22-sdn-onsen-yasu.pdf)（日本語）
- [技術プレゼンテーション/20250822_ENOG87_ohara.pdf](https://enog.jp/wordpress/wp-content/uploads/2025/08/20250822_ENOG87_ohara.pdf)（日本語）

### コードスタイル
本プロジェクトはGNU コーディング標準に従います。提供されたスクリプトを使用してコードの確認とフォーマットを行ってください：

```bash
# フォーマットの確認
./style/check_gnu_style.sh check

# フォーマットの差分表示
./style/check_gnu_style.sh diff

# コードの自動フォーマット
./style/check_gnu_style.sh update
```

## ライセンス

本プロジェクトはMITライセンスの下でライセンスされています。詳細については[LICENSE](LICENSE)ファイルをご覧ください。

## お問い合わせ

ご質問、問題、貢献については、こちらまでご連絡ください：**sdplane [at] nwlab.org**

## 評価用機器の購入

評価用機器には追加機能やソフトウェアの修正が含まれる場合があります。詳細については販売ページをご覧ください：

**[https://www.rca.co.jp/sdplane/](https://www.rca.co.jp/sdplane/)**

