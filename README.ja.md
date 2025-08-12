# sdplane-oss (ソフトデータプレーン)

DPDK（Data Plane Development Kit）を基盤とした高性能オープンソースソフトウェアルーターで、ソフトウェア定義ネットワークアプリケーション向けに設計されています。

**言語 / Language:** [English](README.md) | **日本語**

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
- **Partaker (N100)**：1G NIC搭載ミニPC

Intel (Core i7/9、Xeon)、AMD、ARM CPU等のほかのCPUでも動かない理由はありません。

## 1. 依存関係のインストール

### 依存関係
- **liburcu-qsbr**：ユーザー空間RCUライブラリ
- **libpcap**：パケットキャプチャライブラリ
- **lthread**：[yasuhironet/lthread](https://github.com/yasuhironet/lthread)（軽量協調スレッド）
- **DPDK**：Data Plane Development Kit

### sdplane依存関係debianパッケージのインストール
```
sudo apt install liburcu-dev libpcap-dev
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

## 2. Debianパッケージによるクイックスタート

簡単インストールのため、ビルド済みDebianパッケージをダウンロード・インストールします：

```bash
# 最新パッケージのダウンロード (n305用)
wget https://www.yasuhironet.net/download/n305/sdplane_0.1.4-36_amd64.deb
wget https://www.yasuhironet.net/download/n305/sdplane-dbgsym_0.1.4-36_amd64.ddeb

# もしくは (n100用)
wget https://www.yasuhironet.net/download/n100/sdplane_0.1.4-35_amd64.deb
wget https://www.yasuhironet.net/download/n100/sdplane-dbgsym_0.1.4-35_amd64.ddeb

# パッケージのインストール
sudo apt install ./sdplane_0.1.4-*_amd64.deb
sudo apt install ./sdplane-dbgsym_0.1.4-*_amd64.ddeb
```

**注意**: 最新パッケージバージョンについては [yasuhironet.net ダウンロード](https://www.yasuhironet.net/download/)を確認してください。

5. システム設定 にジャンプしてください。

## 3. ソースからのビルド

### 必須Ubuntuパッケージのインストール

#### ソースからのビルド用
```bash
# コアビルドツール
sudo apt install build-essential cmake autotools-dev autoconf automake libtool pkg-config

# DPDK前提パッケージ
sudo apt install python3 python3-pip meson ninja-build python3-pyelftools libnuma-dev pkgconf
```

#### オプションパッケージ
```bash
sudo apt install etckeeper tig bridge-utils \
                 iptables-persistent fail2ban dmidecode screen ripgrep
```

### ソースからsdplane-ossのビルド

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
```

## 4. sdplane-oss Debianパッケージのビルド（オプション）

### 前提パッケージのインストール
```bash
sudo apt install build-essential cmake devscripts debhelper
```

### sdplane-oss Debianパッケージのビルド
```bash
# まずクリーンな状態から始める
(cd build && make distclean)
make distclean

# ソースからDebianパッケージをビルド
bash build-debian.sh

# 生成されたパッケージをインストール（親ディレクトリに生成される）
sudo apt install ../sdplane_*.deb
```

## 5. システム設定

- **ヒュージページ**：DPDK用システムヒュージページの設定
- **ネットワーク**：ネットワークインターフェース設定にnetplanを使用
- **ファイアウォール**：必要に応じてiptablesルールを設定

### ヒュージページの設定
```bash
# GRUB設定の編集
sudo vi /etc/default/grub

# 以下のいずれかの行を追加:
# 2MBヒュージページの場合 (1536ページ = 約3GB):
GRUB_CMDLINE_LINUX="hugepages=1536"

# または1GBヒュージページの場合 (8ページ = 8GB):
GRUB_CMDLINE_LINUX="default_hugepagesz=1G hugepagesz=1G hugepages=8"

# GRUBを更新して再起動
sudo update-grub
sudo reboot
```

### DPDK IGBカーネルモジュールのインストール（オプション）
```bash
# 方法1: パッケージからインストール
sudo apt-get install -y dpdk-igb-uio-dkms

# 方法2: ソースからビルド
git clone http://dpdk.org/git/dpdk-kmods
cd dpdk-kmods/linux/igb_uio
make
sudo mkdir -p /lib/modules/`uname -r`/extra/dpdk/
sudo cp igb_uio.ko /lib/modules/`uname -r`/extra/dpdk/
echo igb_uio | sudo tee /etc/modules-load.d/igb_uio.conf
```

## 6. sdplane設定

### 設定ファイル

以下の設定ファイルのいずれかを
/etc/sdplane/sdplane.confとして配置してください

#### OS設定（`etc/`）
- [`etc/sdplane.conf.sample`](etc/sdplane.conf.sample)：メイン設定テンプレート
- [`etc/sdplane.service`](etc/sdplane.service)：systemdサービスファイル
- [`etc/modules-load.d/`](etc/modules-load.d/)：カーネルモジュール読み込み設定

#### アプリケーション設定（`example-config/`）
- [`example-config/sdplane-pktgen.conf`](example-config/sdplane-pktgen.conf)：パケットジェネレーター設定
- [`example-config/sdplane-topton.conf`](example-config/sdplane-topton.conf)：Toptonハードウェア設定
- [`example-config/sdplane_l2_repeater.conf`](example-config/sdplane_l2_repeater.conf)：L2リピーター設定
- [`example-config/sdplane_enhanced_repeater.conf`](example-config/sdplane_enhanced_repeater.conf)：拡張リピーター設定

## 7. ソフトウェアルーターの実行

```bash
# フォアグラウンドで実行
sudo ./sdplane/sdplane
  もしくは
# dpkgでインストールした場合、systemd経由で実行
sudo systemctl enable sdplane
sudo systemctl start sdplane

# CLIに接続
telnet localhost 9882
```

## ユーザーガイド（マニュアル）

詳細なユーザーガイドとコマンドリファレンスは以下をご覧ください：

- [ユーザーガイド](doc/manual/ja/README.md) - 全機能の概要とコマンド分類
- [ポート管理・統計](doc/manual/ja/port-management.md) - DPDKポートの管理と統計情報
- [ワーカー・lcore管理](doc/manual/ja/worker-management.md) - ワーカースレッドとlcoreの管理
- [デバッグ・ログ](doc/manual/ja/debug-logging.md) - デバッグとログ機能
- [VTY・シェル管理](doc/manual/ja/vty-shell.md) - VTYとシェルの管理
- [システム情報・監視](doc/manual/ja/system-monitoring.md) - システム情報と監視機能
- [RIB・ルーティング](doc/manual/ja/routing.md) - RIBとルーティング機能
- [キュー設定](doc/manual/ja/queue-configuration.md) - キューの設定と管理
- [パケット生成](doc/manual/ja/packet-generation.md) - PKTGENを使用したパケット生成
- [スレッド情報](doc/manual/ja/thread-information.md) - スレッドの情報と監視
- [TAPインターフェース](doc/manual/ja/tap-interface.md) - TAPインターフェースの管理
- [lthread管理](doc/manual/ja/lthread-management.md) - lthreadの管理
- [デバイス管理](doc/manual/ja/device-management.md) - デバイスとドライバーの管理

## 開発者ガイド

### ドキュメント

- [一般インストールガイド](doc/install-memo.txt) - 1G NICシステム用
- [技術プレゼンテーション](https://enog.jp/wordpress/wp-content/uploads/2024/11/2024-11-22-sdn-onsen-yasu.pdf)（日本語）

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

