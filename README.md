<div align="center">
<img src="sdplane-logo.png" alt="sdplane-oss Logo" width="160">
</div>

# sdplane-oss (ソフトデータプレーン)

DPDKスレッドの動作を対話的に制御できるShellと、DPDKスレッド実行環境（sd-plane）で構成された「DPDK-dock開発環境」

**Language:** [English](https://github.com/kait-cronos/sdplane-doc/blob/main/doc/README.en.md) | **日本語**

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
- **Topton (N305/N100)**：10G NIC搭載ミニPC (tested)
- **Partaker (J3160)**：1G NIC搭載ミニPC (tested)
- **Intel汎用PC**：Intel x520 / Mellanox ConnectX5搭載
- **その他のCPU**：AMD、ARM CPU等でも動作するはずです。

## 始めるには

インストール、設定、実行までの手順は[始めるには](https://github.com/kait-cronos/sdplane-doc/blob/main/doc/getting-started.ja.md)を参照してください。

## Tips

設定・運用に関するヒント集は[Tips](tips.ja.md)を参照してください。

## ユーザーガイド（マニュアル）

詳細なユーザーガイドとコマンドリファレンスは以下をご覧ください：

- [ユーザーガイド](manual/ja/README.md) - 全機能の概要とコマンド分類

**シナリオガイド:**
- [L2リピーターアプリケーション](manual/ja/l2-repeater-application.md) - MACラーニングによるシンプルなL2パケット転送
- [拡張リピーターアプリケーション](manual/ja/enhanced-repeater-application.md) - TAPインターフェース付きVLAN対応スイッチング
- [パケットジェネレーターアプリケーション](manual/ja/packet-generator-application.md) - 高性能トラフィック生成とテスト
- [スイッチを使う](manual/ja/scenario-switch.md) - VLANベースのL2スイッチングを構成する
- [ルータを設定する：静的経路のみ](manual/ja/scenario-static-router.md) - 静的経路によるIPルーターを構成する

**管理・設定ガイド:**
- [ポート管理・統計](manual/ja/port-management.md) - DPDKポートの管理と統計情報
- [ワーカー・lcore管理・スレッド情報](manual/ja/worker-lcore-thread-management.md) - ワーカースレッド、lcore、スレッド情報の管理
- [デバッグ・ログ](manual/ja/debug-logging.md) - デバッグとログ機能
- [VTY・シェル管理](manual/ja/vty-shell.md) - VTYとシェルの管理
- [システム情報・監視](manual/ja/system-monitoring.md) - システム情報と監視機能
- [RIB・ルーティング](manual/ja/routing.md) - RIBとルーティング機能
- [キュー設定](manual/ja/queue-configuration.md) - キューの設定と管理
- [パケット生成](manual/ja/packet-generation.md) - PKTGENを使用したパケット生成
- [TAPインターフェース](manual/ja/tap-interface.md) - TAPインターフェースの管理
- [lthread管理](manual/ja/lthread-management.md) - lthreadの管理
- [デバイス管理](manual/ja/device-management.md) - デバイスとドライバーの管理
- [拡張リピーター](manual/ja/enhanced-repeater.md) - 仮想スイッチング、VLAN処理、TAPインターフェース
- [rte-flow](manual/ja/rte-flow.md) - ハードウェアフロー分類の設定

**コマンド一覧:**
- [全コマンド一覧（アルファベット順）](manual/ja/command-list.md) - 全112コマンドの索引

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

本プロジェクトはGNU General Public License v3.0の下でライセンスされています。詳細については[LICENSE](../LICENSE)ファイルをご覧ください。

## お問い合わせ

ご質問、問題、貢献については、こちらまでご連絡ください：**sdplane [at] nwlab.org**

## 評価用機器の購入

評価用機器には追加機能やソフトウェアの修正が含まれる場合があります。詳細については販売ページをご覧ください：

**[https://www.rca.co.jp/sdplane/](https://www.rca.co.jp/sdplane/)**

