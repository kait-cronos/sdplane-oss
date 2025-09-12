# パケットジェネレーター（PKTGEN）アプリケーション

Created by Keith Wiles @ Intel 2010-2025 from https://github.com/pktgen/Pktgen-DPDK

**Language:** [English](../en/packet-generator-application.md) | **日本語** | [Français](../fr/packet-generator-application.md) | [中文](../zh/packet-generator-application.md) | [Deutsch](../de/packet-generator-application.md) | [Italiano](../it/packet-generator-application.md) | [한국어](../ko/packet-generator-application.md) | [ไทย](../th/packet-generator-application.md) | [Español](../es/packet-generator-application.md)

パケットジェネレーター（PKTGEN）アプリケーションは、DPDKの最適化されたパケット処理フレームワークを使用した高性能なパケット生成とトラフィックテスト機能を提供します。

## 概要

PKTGENは、sdplane-ossに統合された洗練されたトラフィック生成ツールで、以下を実現します：
- **高速パケット生成**: マルチギガビットパケット送信レート
- **柔軟なトラフィックパターン**: カスタマイズ可能なパケットサイズ、レート、パターン
- **マルチポートサポート**: 複数ポートでの独立したトラフィック生成
- **高度な機能**: レンジテスト、レート制限、トラフィックシェーピング
- **パフォーマンステスト**: ネットワークスループットと遅延測定

## アーキテクチャ

### コアコンポーネント
- **TXエンジン**: DPDKを使用した高性能パケット送信
- **RXエンジン**: パケット受信と統計収集
- **L2Pフレームワーク**: 最適なパフォーマンスのためのlcore-to-portマッピング
- **設定管理**: 動的なトラフィックパラメータ設定
- **統計エンジン**: 包括的なパフォーマンスメトリクスとレポート

### ワーカーモデル
PKTGENは専用ワーカースレッド（lcore）を使用して動作します：
- **TXワーカー**: パケット送信専用コア
- **RXワーカー**: パケット受信専用コア
- **混合ワーカー**: 単一コア上でのTX/RX結合
- **制御スレッド**: 管理と統計収集

## 主要機能

### トラフィック生成
- **パケットレート**: インターフェース制限までのラインレートトラフィック生成
- **パケットサイズ**: 64バイトからジャンボフレームまで設定可能
- **トラフィックパターン**: 均一、バースト、カスタムパターン
- **マルチストリーム**: ポートあたり複数のトラフィックストリーム

### 高度な機能
- **レート制限**: 精密なトラフィックレート制御
- **レンジテスト**: パケットサイズスイープとレートスイープ
- **負荷パターン**: 定数、ランプアップ、バーストトラフィック
- **プロトコルサポート**: イーサネット、IPv4、IPv6、TCP、UDP

### パフォーマンス監視
- **リアルタイム統計**: TX/RXレート、パケット数、エラー数
- **遅延測定**: エンドツーエンドパケット遅延テスト
- **スループット解析**: 帯域幅利用率と効率
- **エラー検出**: パケット損失と破損検出

## 設定

### 基本セットアップ
PKTGENは特定の初期化とワーカー設定が必要です：

```bash
# PKTGEN用デバイスバインディング
set device 01:00.0 driver vfio-pci bind
set device 01:00.1 driver vfio-pci bind

# argv-listでのPKTGEN初期化
set argv-list 2 ./usr/local/bin/pktgen -l 0-7 -n 4 --proc-type auto --log-level 7 --file-prefix pg -- -v -T -P -l pktgen.log -m [4:5].0 -m [6:7].1 -f themes/black-yellow.theme

# PKTGEN初期化
pktgen init argv-list 2
```

### ワーカー割り当て
最適なパフォーマンスのための専用コア割り当て：

```bash
# 特定のlcoreにPKTGENワーカーを割り当て
set worker lcore 4 pktgen  # ポート0 TX/RX
set worker lcore 5 pktgen  # ポート0 TX/RX
set worker lcore 6 pktgen  # ポート1 TX/RX
set worker lcore 7 pktgen  # ポート1 TX/RX
```

### ポート設定
PKTGEN使用のためのDPDKポート設定：

```bash
# ポートセットアップ
stop port all
set port all dev-configure 1 4
set port all nrxdesc 1024
set port all ntxdesc 1024
set port all promiscuous enable
start port all
```

## 設定例

### 完全なPKTGENセットアップ
[`example-config/sdplane-pktgen.conf`](../../example-config/sdplane-pktgen.conf)をご覧ください：

```bash
# ログ設定
log file /var/log/sdplane.log
log stdout

# デバイスバインディング
set device 01:00.0 driver vfio-pci bind
set device 01:00.1 driver vfio-pci bind
set device 03:00.0 driver uio_pci_generic bind

# PKTGEN初期化
set argv-list 2 ./usr/local/bin/pktgen -l 0-7 -n 4 --proc-type auto --log-level 7 --file-prefix pg -- -v -T -P -l pktgen.log -m [4:5].0 -m [6:7].1 -f themes/black-yellow.theme
pktgen init argv-list 2

# メモリプールセットアップ
set mempool

# バックグラウンドワーカー
set worker lthread stat-collector
set worker lthread rib-manager
set worker lthread netlink-thread

# ポート設定
stop port all
set port all dev-configure 1 4
set port all nrxdesc 1024
set port all ntxdesc 1024

# ワーカー割り当て
set worker lcore 2 vlan-switch
set worker lcore 3 tap-handler
set worker lcore 4 pktgen
set worker lcore 5 pktgen
set worker lcore 6 pktgen
set worker lcore 7 pktgen

# プロミスキャスモードを有効化して開始
set port all promiscuous enable
start port all

# すべてのワーカーを開始
start worker lcore all
```

## PKTGENコマンドライン引数

### コアパラメータ
- `-l 0-7`: CPUコアリスト（コア0-7）
- `-n 4`: メモリチャネル数
- `--proc-type auto`: プロセスタイプ（プライマリ/セカンダリ）
- `--log-level 7`: デバッグログレベル
- `--file-prefix pg`: 共有メモリプレフィックス

### PKTGEN固有オプション
- `-v`: 詳細出力
- `-T`: カラーターミナル出力を有効化
- `-P`: プロミスキャスモードを有効化
- `-l pktgen.log`: ログファイル場所
- `-m [4:5].0`: lcore 4,5をポート0にマップ
- `-m [6:7].1`: lcore 6,7をポート1にマップ
- `-f themes/black-yellow.theme`: カラーテーマ

## lcore-to-portマッピング

### マッピング構文
マッピング形式`[lcore_list].port`でコアをポートに割り当て：
- `[4:5].0`: lcore 4と5がポート0を処理
- `[6:7].1`: lcore 6と7がポート1を処理
- `[4].0`: 単一lcore 4がポート0を処理
- `[4-7].0`: lcore 4から7がポート0を処理

### パフォーマンス最適化
- **専用コア**: TXとRXに別々のコアを割り当て
- **NUMA意識**: ネットワークインターフェースにローカルなコアを使用
- **競合回避**: システムワーカーと重複を避ける
- **負荷分散**: 複数コア間でのトラフィック分散

## 操作と監視

### ステータスコマンド
```bash
# PKTGENステータスと設定の表示
show pktgen

# ワーカー割り当てとパフォーマンスの表示
show worker statistics

# ポートレベル統計
show port statistics all
```

### ランタイム管理
PKTGENはインタラクティブCLIを通じて広範囲なランタイム設定を提供：
- **トラフィックパラメータ**: パケットサイズ、レート、パターン
- **開始/停止制御**: ポートごとのトラフィック制御
- **統計**: リアルタイムパフォーマンス監視
- **レンジテスト**: パラメータスイープの自動化

### パフォーマンス監視
```bash
# リアルタイム統計の監視
# （PKTGENインタラクティブインターフェースを通じて利用可能）

# 主要メトリクス:
# - 秒あたりTX/RXパケット数
# - 帯域幅利用率
# - パケット損失率
# - 遅延測定
```

## CLIコマンド

### PKTGEN制御コマンド

#### 初期化
```bash
# argv-list設定でPKTGENを初期化
pktgen init argv-list <0-7>
```

#### トラフィック生成開始/停止
```bash
# 特定ポートでトラフィック生成を開始
pktgen do start port <0-7>
pktgen do start port all

# トラフィック生成を停止
pktgen do stop port <0-7>
pktgen do stop port all
```

#### トラフィック設定コマンド

##### パケット数設定
```bash
# 送信するパケット数を設定
pktgen do set port <0-7> count <0-4000000000>
pktgen do set port all count <0-4000000000>
```

##### パケットサイズ設定
```bash
# パケットサイズをバイト単位で設定
pktgen do set port <0-7> size <0-9999>
pktgen do set port all size <0-9999>
```

##### 送信レート設定
```bash
# 送信レートをパーセンテージで設定
pktgen do set port <0-7> rate <0-100>
pktgen do set port all rate <0-100>
```

##### TCP/UDPポート番号設定
```bash
# TCP送信元・宛先ポートを設定
pktgen do set port <0-7> tcp src <0-65535> dst <0-65535>
pktgen do set port all tcp src <0-65535> dst <0-65535>

# UDP送信元・宛先ポートを設定
pktgen do set port <0-7> udp src <0-65535> dst <0-65535>
pktgen do set port all udp src <0-65535> dst <0-65535>
```

##### TTL値設定
```bash
# IP Time-to-Live値を設定
pktgen do set port <0-7> ttl <0-255>
pktgen do set port all ttl <0-255>
```

##### MACアドレス設定
```bash
# 送信元MACアドレスを設定
pktgen do set port <0-7> mac source <MAC>
pktgen do set port all mac source <MAC>

# 宛先MACアドレスを設定
pktgen do set port <0-7> mac destination <MAC>
pktgen do set port all mac destination <MAC>
```

##### IPv4アドレス設定
```bash
# 送信元IPv4アドレスを設定
pktgen do set port <0-7> ipv4 source <IPv4>
pktgen do set port all ipv4 source <IPv4>

# 宛先IPv4アドレスを設定
pktgen do set port <0-7> ipv4 destination <IPv4>
pktgen do set port all ipv4 destination <IPv4>
```

#### ステータス・監視コマンド
```bash
# PKTGENステータスと設定を表示
show pktgen

# ポート統計を表示
show port statistics all
show port statistics <0-7>
```

## 使用ケース

### ネットワークパフォーマンステスト
- **スループットテスト**: 最大帯域幅測定
- **遅延テスト**: エンドツーエンド遅延解析
- **負荷テスト**: 持続的トラフィック生成
- **ストレステスト**: 最大パケットレート検証

### 機器検証
- **スイッチテスト**: 転送パフォーマンス検証
- **ルーターテスト**: L3転送パフォーマンス
- **インターフェーステスト**: ポートとケーブル検証
- **プロトコルテスト**: 特定のプロトコル動作

### ネットワーク開発
- **プロトコル開発**: 新しいネットワークプロトコルのテスト
- **アプリケーションテスト**: 現実的なトラフィックパターンの生成
- **パフォーマンス調整**: ネットワーク設定の最適化
- **ベンチマーク**: 標準化されたパフォーマンス比較

## パフォーマンス調整

### コア割り当て
```bash
# 最高パフォーマンスのためのコア使用を最適化
# 可能な場合はTXとRXコアを分離
# ネットワークインターフェースにローカルなNUMAコアを使用
set worker lcore 4 pktgen  # ポート0のTXコア
set worker lcore 5 pktgen  # ポート0のRXコア
```

### メモリ設定
```bash
# トラフィックパターンに応じてディスクリプタリングを最適化
set port all nrxdesc 2048  # 高レート用に増加
set port all ntxdesc 2048  # バースト用に増加
```

### システム調整
- **CPU分離**: OSスケジューラからPKTGENコアを分離
- **割り込み親和性**: 非PKTGENコアに割り込みをバインド
- **メモリ割り当て**: 最適なパフォーマンスのためにヒュージページを使用
- **NIC調整**: ネットワークインターフェース設定を最適化

## トラブルシューティング

### 一般的な問題
- **低パフォーマンス**: コア割り当てとNUMAトポロジーを確認
- **パケット損失**: バッファサイズとシステムリソースを確認
- **初期化失敗**: デバイスバインディングと権限を確認
- **レート制限**: インターフェース能力と設定を確認

### デバッグ戦略
- **詳細ログの有効化**: 詳細出力のためにより高いログレベルを使用
- **統計確認**: 異常のためのTX/RXカウンターを監視
- **マッピング確認**: 正しいlcore-to-port割り当てを確実に
- **システム監視**: テスト中のCPU、メモリ、割り込み使用を確認

#### デバッグコマンド
```bash
# PKTGENデバッグログを有効化
debug sdplane pktgen

# 一般的なsdplaneデバッグ
debug sdplane rib
debug sdplane fdb-change
```

### パフォーマンス検証
```bash
# PKTGENが期待されるレートを達成しているか確認
show pktgen
show port statistics all

# エラーやドロップを確認
# テスト中のシステムリソースを監視
```

## 高度な機能

### レンジテスト
PKTGENはパラメータ範囲での自動テストをサポート：
- **パケットサイズスイープ**: 64から1518バイトまでのテスト
- **レートスイープ**: ラインレート1%から100%までのテスト
- **自動レポート**: 包括的なテストレポートの生成

### トラフィックパターン
- **定数レート**: 安定したトラフィック生成
- **バーストパターン**: アイドル期間のあるトラフィックバースト
- **ランプパターン**: 徐々に増加/減少するレート
- **カスタムパターン**: ユーザー定義のトラフィックプロファイル

## 関連ドキュメント

- [パケット生成コマンド](packet-generation.md) - コマンドリファレンス
- [ワーカー管理](worker-lcore-thread-management.md) - ワーカー設定
- [ポート管理](port-management.md) - DPDKポートセットアップ
- [パフォーマンス調整ガイド](#) - システム最適化のヒント

## 外部リソース

- [DPDK Pktgenドキュメント](http://pktgen-dpdk.readthedocs.io/) - 公式PKTGENドキュメント
- [DPDKパフォーマンスガイド](https://doc.dpdk.org/guides/prog_guide/) - DPDK最適化ガイド