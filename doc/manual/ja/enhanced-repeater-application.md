# 拡張リピーターアプリケーション

**Language:** [English](../en/enhanced-repeater-application.md) | **日本語** | [Français](../fr/enhanced-repeater-application.md) | [中文](../zh/enhanced-repeater-application.md) | [Deutsch](../de/enhanced-repeater-application.md) | [Italiano](../it/enhanced-repeater-application.md) | [한국어](../ko/enhanced-repeater-application.md) | [ไทย](../th/enhanced-repeater-application.md) | [Español](../es/enhanced-repeater-application.md)

拡張リピーターアプリケーションは、L3ルーティングとパケットキャプチャ機能用のTAPインターフェース統合を備えた、高度なVLAN対応レイヤー2スイッチングを提供します。

## 概要

拡張リピーターは、基本的なL2転送を以下の機能で拡張する洗練されたレイヤー2転送アプリケーションです：
- **VLAN対応スイッチング**: IEEE 802.1Q VLANの完全サポート（タギング/アンタギング）
- **仮想スイッチ抽象化**: 独立した転送テーブルを持つ複数の仮想スイッチ
- **TAPインターフェース統合**: カーネル統合用のルーターインターフェースとキャプチャインターフェース
- **高度なVLAN処理**: VLAN変換、挿入、除去機能

## アーキテクチャ

### 仮想スイッチフレームワーク
拡張リピーターは仮想スイッチアーキテクチャを実装します：
- **複数VLAN**: 複数のVLANドメイン（1-4094）のサポート
- **ポート集約**: 仮想スイッチあたり複数の物理ポート
- **分離転送**: VLANごとの独立した転送ドメイン
- **柔軟なタギング**: ポートごとのネイティブ、タグ付き、変換モード

### TAPインターフェース統合
- **ルーターインターフェース**: L3処理のためのカーネルネットワークスタック統合
- **キャプチャインターフェース**: パケット監視と解析機能
- **リングバッファ**: データプレーンとカーネル間の効率的なパケット転送
- **双方向**: 入出力両方向のパケット処理

## 主要機能

### VLAN処理
- **VLANタギング**: アンタグフレームへの802.1Qヘッダーの追加
- **VLANアンタギング**: タグ付きフレームからの802.1Qヘッダーの除去
- **VLAN変換**: 入出力間でのVLAN IDの変更
- **ネイティブVLAN**: トランクポートでのアンタグトラフィックの処理

### 仮想スイッチング
- **学習**: VLANごとの自動MACアドレス学習
- **フラッディング**: 未知のユニキャストとブロードキャストの適切な処理
- **スプリットホライゾン**: 仮想スイッチ内でのループ防止
- **マルチドメイン**: VLANごとの独立した転送テーブル

### パケット処理
- **ゼロコピー**: 最小限のオーバーヘッドでの効率的なDPDKパケット処理
- **バースト処理**: 高パケットレート用に最適化
- **ヘッダー操作**: 効率的なVLANヘッダー挿入/除去
- **コピー最適化**: TAPインターフェース用の選択的パケットコピー

## 設定

### 仮想スイッチセットアップ
異なるVLANドメイン用の仮想スイッチを作成：

```bash
# VLAN IDを持つ仮想スイッチを作成
set vswitch 2031
set vswitch 2032
```

### ポートから仮想スイッチへのリンク
物理DPDKポートを仮想スイッチにリンク：

```bash
# ポート0を仮想スイッチ0にVLANタグ2031でリンク
set vswitch-link vswitch 0 port 0 tag 2031

# ポート0を仮想スイッチ1にネイティブ/アンタグでリンク
set vswitch-link vswitch 1 port 0 tag 0

# VLAN変換例
set vswitch-link vswitch 0 port 1 tag 2040
```

### ルーターインターフェース
L3接続用のルーターインターフェースを作成：

```bash
# L3処理用のルーターインターフェースを作成
set vswitch 2031 router-if rif2031
set vswitch 2032 router-if rif2032
```

### キャプチャインターフェース
監視用のキャプチャインターフェースをセットアップ：

```bash
# パケット監視用のキャプチャインターフェースを作成
set vswitch 2031 capture-if cif2031
set vswitch 2032 capture-if cif2032
```

### ワーカー設定
拡張リピーターをlcoreに割り当て：

```bash
# 拡張リピーターワーカーを設定
set worker lcore 1 enhanced-repeater

# キュー割り当てを設定
set thread 1 port 0 queue 0
```

## 設定例

### 完全なセットアップ
[`example-config/sdplane_enhanced_repeater.conf`](../../example-config/sdplane_enhanced_repeater.conf)をご覧ください：

```bash
# デバイスセットアップ
set device 03:00.0 driver vfio-pci bind

# DPDK初期化
set rte_eal argv -c 0x7
rte_eal_init
set mempool

# ポート設定
stop port all
set port all dev-configure 1 4
set port all nrxdesc 1024
set port all ntxdesc 1024

# ワーカーセットアップ
set worker lthread stat-collector
set worker lthread rib-manager
set worker lcore 1 enhanced-repeater
set worker lcore 2 l3-tap-handler

# プロミスキャスモードを有効化
set port all promiscuous enable
start port all

# キュー設定
set thread 1 port 0 queue 0

# 仮想スイッチセットアップ
set vswitch 2031
set vswitch 2032

# リンク設定
set vswitch-link vswitch 0 port 0 tag 2031
set vswitch-link vswitch 1 port 0 tag 2032

# インターフェース作成
set vswitch 2031 router-if rif2031
set vswitch 2032 router-if rif2032
set vswitch 2031 capture-if cif2031
set vswitch 2032 capture-if cif2032

# ワーカー開始
start worker lcore all
```

## VLAN処理の詳細

### タギングモード

#### タグ付きモード
- **入力**: 特定のVLANタグを持つパケットを受け入れ
- **出力**: VLANタグの維持または変換
- **用途**: トランクポート、VLAN変換

#### ネイティブモード（tag 0）
- **入力**: アンタグパケットを受け入れ
- **出力**: VLANヘッダーの除去
- **用途**: アクセスポート、VLAN非対応デバイス

#### 変換モード
- **機能**: ポート間でのVLAN IDの変更
- **設定**: 入出力リンクでの異なるタグ
- **用途**: VLANマッピング、サービスプロバイダーネットワーク

### パケットフロー

1. **入力処理**:
   - DPDKポートでのパケット受信
   - タグまたはネイティブ設定に基づくVLAN判定
   - 宛先仮想スイッチの検索

2. **仮想スイッチ検索**:
   - MACアドレス学習と検索
   - VLANドメイン内での出力ポート決定
   - 未知のユニキャスト/ブロードキャストフラッディングの処理

3. **出力処理**:
   - ポート設定ごとのVLANヘッダー操作
   - 複数宛先のパケットコピー
   - TAPインターフェース統合

4. **TAPインターフェース処理**:
   - ルーターインターフェース: カーネルL3処理
   - キャプチャインターフェース: 監視と解析

## 監視と管理

### ステータスコマンド
```bash
# 仮想スイッチ設定の表示
show vswitch_rib

# 仮想スイッチリンクの表示
show vswitch-link

# ルーターインターフェースの表示
show rib vswitch router-if

# キャプチャインターフェースの確認
show rib vswitch capture-if
```

### 統計とパフォーマンス
```bash
# ポートレベル統計
show port statistics all

# ワーカーパフォーマンス
show worker statistics

# スレッド情報
show thread information
```

### デバッグコマンド
```bash
# 拡張リピーターデバッグ
debug sdplane enhanced-repeater

# VLANスイッチデバッグ（代替ワーカータイプ）
debug sdplane vlan-switch

# RIBと転送デバッグ
debug sdplane rib
debug sdplane fdb-change
```

## 使用ケース

### VLAN集約
- 単一物理リンク上での複数VLANの統合
- プロバイダーエッジ機能
- サービス多重化

### L3統合
- Router-on-a-stick設定
- TAPインターフェースを介したVLAN間ルーティング
- ハイブリッドL2/L3転送

### ネットワーク監視
- VLANごとのパケットキャプチャ
- トラフィック解析とデバッグ
- サービス監視

### サービスプロバイダーネットワーク
- 顧客分離のためのVLAN変換
- マルチテナントネットワーキング
- トラフィックエンジニアリング

## パフォーマンス考慮事項

### スケーリング
- **ワーカー割り当て**: 最適なパフォーマンスのためlcoreを専用化
- **キュー設定**: コア間でのキュー割り当てのバランス
- **メモリプール**: パケットレートとバッファニーズに適切にサイズ設定

### VLAN処理オーバーヘッド
- **ヘッダー操作**: VLAN操作の最小限のオーバーヘッド
- **パケットコピー**: 必要な場合のみの選択的コピー
- **バースト最適化**: 効率性のためのバースト単位でのパケット処理

### TAPインターフェースパフォーマンス
- **リングバッファサイジング**: 適切なリングサイズの設定
- **選択的転送**: TAPインターフェースへの関連パケットのみ送信
- **カーネル統合**: カーネル処理オーバーヘッドを考慮

## トラブルシューティング

### 一般的な問題
- **VLAN不一致**: タグ設定がネットワークセットアップと一致するか確認
- **TAPインターフェース作成**: 適切な権限とカーネルサポートを確認
- **パフォーマンス問題**: キュー割り当てとワーカー分散を確認

### デバッグ戦略
- **デバッグログの有効化**: 詳細なパケットフローのためのデバッグコマンドを使用
- **統計監視**: ポートとワーカー統計を監視
- **パケットキャプチャ**: トラフィック解析にキャプチャインターフェースを使用

## 関連ドキュメント

- [拡張リピーターコマンド](enhanced-repeater.md) - 完全なコマンドリファレンス
- [ワーカー管理](worker-lcore-thread-management.md) - ワーカー設定の詳細
- [ポート管理](port-management.md) - DPDK ポート設定
- [TAPインターフェース管理](tap-interface.md) - TAPインターフェースの詳細