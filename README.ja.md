# sdplane-dev (ソフトデータプレーン)

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

## クイックスタート（Debianパッケージ）

（準備中）

## ソースからのビルド

### システム要件
- **OS**：
  Ubuntu Linux（現在サポート中）
- **NIC**：
  4つのネットワークインターフェース（仮想環境ではvirtio-net対応）
- **メモリ**：
  ヒュージページサポートが必要
- **CPU**：
  マルチコアプロセッサ推奨

## ハードウェアプラットフォーム

本プロジェクトは以下でテスト済みです：
- **Topton**：10G NIC搭載ミニPC
- **Wiretap**：1G NIC搭載ミニPC

### 必須Ubuntuパッケージ
```bash
sudo apt install build-essential cmake \
                 autotools-dev autoconf automake libtool pkg-config
```

### オプションUbuntuパッケージ
```bash
sudo apt install etckeeper tig bridge-utils \
                 iptables-persistent fail2ban dmidecode screen ripgrep
```

### 依存関係
- **DPDK**：Data Plane Development Kit
- **libsdplane-dev**：[kait-cronos/libsdplane-dev](https://github.com/kait-cronos/libsdplane-dev)（別途ビルド・インストール）
- **lthread**：[yasuhironet/lthread](https://github.com/yasuhironet/lthread)（DPDKベース協調スレッド）
- **liburcu-qsbr**：ユーザー空間RCUライブラリ
- **libpcap**：パケットキャプチャライブラリ

### 1. 依存関係のインストール

まず、必要なライブラリをインストール・ビルドします：
```bash
# libsdplane-devのインストール（ソースからビルド）
git clone https://github.com/kait-cronos/libsdplane-dev
cd libsdplane-dev
# そのリポジトリのビルド手順に従ってください

# lthreadのインストール
git clone https://github.com/yasuhironet/lthread
cd lthread
# そのリポジトリのビルド手順に従ってください
```

### 2. sdplane-devのビルド

```bash
# リポジトリのクローン
git clone https://github.com/kait-cronos/sdplane-dev
cd sdplane-dev

# ビルドファイルの生成
./autogen.sh

# 設定とビルド
mkdir build
cd build
CFLAGS="-g -O0" sh ../configure
make
```

### 3. ソフトウェアルーターの実行

```bash
# フォアグラウンドで実行
sudo ./sdplane/sdplane
  または
# dpkgでインストールした場合、バックグラウンドで実行
sudo systemctl start sdplane

# CLIに接続
telnet localhost 9882
```

## 設定

### システム設定
- **ヒュージページ**：DPDK用システムヒュージページの設定
- **ネットワーク**：ネットワークインターフェース設定にnetplanを使用
- **ファイアウォール**：必要に応じてiptablesルールを設定

### 設定ファイル

#### OS設定（`etc/`）
- `etc/sdplane.conf.sample`：メイン設定テンプレート
- `etc/sdplane.service`：systemdサービスファイル
- `etc/modules-load.d/`：カーネルモジュール読み込み設定

#### アプリケーション設定（`example-config/`）
- `example-config/sdplane-pktgen.conf`：パケットジェネレーター設定
- `example-config/sdplane-topton.conf`：Toptonハードウェア設定
- `example-config/sdplane_l2_repeater.conf`：L2リピーター設定
- `example-config/sdplane_l2fwd.conf`：L2フォワーディング設定
- `example-config/sdplane_l3fwd-lpm.conf`：LPM付きL3フォワーディング設定
- `example-config/sdplane-nettlp.conf`：NetTLP設定

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
- [NetTLP](doc/manual/ja/nettlp.md) - Network TLP機能
- [lthread管理](doc/manual/ja/lthread-management.md) - lthreadの管理
- [デバイス管理](doc/manual/ja/device-management.md) - デバイスとドライバーの管理

## 開発者ガイド

### ドキュメント

- [Topton インストールガイド](doc/install-memo-topton.txt) - 10G NICシステム用
- [一般インストールガイド](doc/install-memo.txt) - 1G NICシステム用
- [NetTLP 設定ガイド](doc/nettlp-memo.txt) - NetTLP設定手順
- [技術プレゼンテーション](https://enog.jp/wordpress/wp-content/uploads/2024/11/2024-11-22-sdn-onsen-yasu.pdf)（日本語）

### コードスタイル
本プロジェクトはGNU コーディング標準に従います。提供されたスクリプトを使用してコードの確認とフォーマットを行ってください：

```bash
# フォーマットの確認
./style/check_gnu_style.sh check

# コードの自動フォーマット
./style/check_gnu_style.sh update
```

## ライセンス

本プロジェクトはオープンソースです。ライセンスの詳細についてはLICENSEファイルをご覧ください。

## 作者

小原泰弘（Yasuhiro Ohara）(yasu1976@gmail.com)