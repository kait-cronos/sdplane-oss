# ワーカー・lcore管理・スレッド情報

**言語 / Language:** [English](../worker-lcore-thread-management.md) | **日本語** | [Français](../fr/worker-lcore-thread-management.md) | [中文](../zh/worker-lcore-thread-management.md) | [Deutsch](../de/worker-lcore-thread-management.md) | [Italiano](../it/worker-lcore-thread-management.md) | [한국어](../ko/worker-lcore-thread-management.md) | [ไทย](../th/worker-lcore-thread-management.md) | [Español](../es/worker-lcore-thread-management.md)

DPDKワーカースレッド、lcore、およびスレッド情報の管理・監視を行うコマンドです。

## ワーカー管理コマンド

### set_worker - ワーカータイプ設定
```
set worker lcore <0-16> (|none|l2fwd|l3fwd|l3fwd-lpm|tap-handler|l2-repeater|enhanced-repeater|vlan-switch|pktgen|linkflap-generator)
```

指定されたlcoreにワーカータイプを設定します。

**ワーカータイプ：**
- `none` - ワーカーなし
- `l2fwd` - Layer 2フォワーディング
- `l3fwd` - Layer 3フォワーディング
- `l3fwd-lpm` - Layer 3フォワーディング (LPM)
- `tap-handler` - TAPインターフェースハンドラー
- `l2-repeater` - Layer 2リピーター
- `enhanced-repeater` - VLANスイッチングとTAPインターフェースを備えた拡張リピーター
- `vlan-switch` - VLANスイッチ
- `pktgen` - パケットジェネレーター
- `linkflap-generator` - リンクフラップジェネレーター

**使用例：**
```bash
# lcore 1にL2フォワーディングワーカーを設定
set worker lcore 1 l2fwd

# lcore 1に拡張リピーターワーカーを設定
set worker lcore 1 enhanced-repeater

# lcore 2にワーカーなしを設定
set worker lcore 2 none
```

### reset_worker - ワーカーリセット
```
reset worker lcore (<0-16>|all)
```

指定されたlcoreまたは全lcoreのワーカーをリセットします。

### start_worker - ワーカー開始
```
start worker lcore (<0-16>|all)
```

指定されたlcoreまたは全lcoreのワーカーを開始します。

### stop_worker - ワーカー停止
```
stop worker lcore (<0-16>|all)
```

指定されたlcoreまたは全lcoreのワーカーを停止します。

### restart_worker - ワーカー再開始
```
restart worker lcore (<0-16>|all)
```

指定されたlcoreまたは全lcoreのワーカーを再開始します。

**使用例：**
```bash
# lcore 1のワーカーを開始
start worker lcore 1

# 全ワーカーを停止
stop worker lcore all

# lcore 3のワーカーを再開始
restart worker lcore 3
```

### show_worker - ワーカー情報の表示
```
show worker
```

現在のワーカーの状態と設定を表示します。

## スレッド情報コマンド

### show_thread - スレッド情報表示
```
show thread
```

現在のスレッドの状態と情報を表示します。

このコマンドは以下の情報を表示します：
- スレッドID
- スレッドの状態
- 実行中のタスク
- CPU使用率
- メモリ使用量

### show_thread_counter - スレッドカウンター表示
```
show thread counter
```

スレッドのカウンター情報を表示します。

このコマンドは以下の情報を表示します：
- 処理パケット数
- 実行回数
- エラー回数
- 処理時間統計

## システム初期化コマンド

### set_mempool - メモリプールの設定
```
set mempool
```

DPDKメモリプールを設定します。

### set_rte_eal_argv - RTE EALコマンドライン引数設定
```
set rte_eal argv <WORD> <WORD> <WORD> <WORD> <WORD> <WORD>
```

RTE EAL（Environment Abstraction Layer）の初期化に使用するコマンドライン引数を設定します。

**使用例：**
```bash
# EAL引数を設定
set rte_eal argv -c 0x1 -n 4 --socket-mem 1024,1024 --huge-dir /mnt/huge
```

### rte_eal_init - RTE EAL初期化
```
rte_eal_init
```

RTE EAL（Environment Abstraction Layer）を初期化します。

## ワーカータイプとスレッドアーキテクチャ

### ワーカータイプの詳細

#### L2フォワーディング (l2fwd)
Layer 2レベルでのパケット転送を行うワーカーです。MACアドレスベースでパケットを転送します。

#### L3フォワーディング (l3fwd/l3fwd-lpm)
Layer 3レベルでのパケット転送を行うワーカーです。IPアドレスベースでルーティングを行います。LPMは Longest Prefix Matching を使用します。

#### TAPハンドラー (tap-handler)
TAPインターフェースとDPDKポート間でのパケット転送を行うワーカーです。

#### 拡張リピーター (enhanced-repeater)
VLANスイッチングとTAPインターフェースを備えた高機能リピーターワーカーです。

#### パケットジェネレーター (pktgen)
テスト用のパケットを生成するワーカーです。

### sdplaneのスレッドアーキテクチャ

sdplaneでは以下のスレッドタイプが協調的マルチスレッドモデルで動作します：

#### 1. ワーカースレッド（lcore上で実行）
- **L2/L3フォワーディング** - パケット転送処理
- **TAPハンドラー** - TAPインターフェース処理
- **PKTGEN** - パケット生成

#### 2. 管理スレッド（lthread）
- **RIBマネージャー** - ルーティング情報管理
- **統計コレクター** - 統計情報収集
- **Netlinkスレッド** - Netlink通信処理

#### 3. システムスレッド
- **VTYサーバー** - VTY接続処理
- **コンソール** - コンソール入出力

### lthreadとの関係
sdplaneは協調的マルチスレッド（cooperative threading）モデルを使用しています：
- **lthread** - 軽量スレッド実装
- **協調的スケジューリング** - 明示的なyieldによる制御
- **高効率** - コンテキストスイッチのオーバーヘッドを削減

## スレッド情報の見方

### 基本的な表示項目
- **Thread ID** - スレッドの識別子
- **Name** - スレッドの名前
- **State** - スレッドの状態（Running/Ready/Blocked/Terminated）
- **lcore** - 実行中のCPUコア
- **Type** - スレッドタイプ

### カウンター情報
- **Packets** - 処理パケット数
- **Loops** - ループ実行回数
- **Errors** - エラー回数
- **CPU Time** - CPU使用時間

## 使用例とワークフロー

### 基本的な設定とワーカー起動
```bash
# 1. EAL初期化設定
set rte_eal argv -c 0xf -n 4

# 2. EAL初期化実行
rte_eal_init

# 3. メモリプール設定
set mempool

# 4. ワーカー設定
set worker lcore 1 l2fwd
set worker lcore 2 l3fwd

# 5. ワーカー開始
start worker lcore all

# 6. 状態確認
show worker
show thread
```

### 監視とトラブルシューティング

#### 定期監視
```bash
# ワーカーとスレッド状態確認
show worker
show thread
show thread counter
```

#### パフォーマンス分析
```bash
# パフォーマンス関連情報
show thread counter
show loop-count l2fwd pps
```

#### トラブルシューティング手順

**スレッドが応答しない場合**
```bash
# 1. スレッド状態確認
show thread

# 2. ワーカー状態確認
show worker

# 3. 必要に応じて再起動
restart worker lcore 1
```

**パフォーマンス低下の場合**
```bash
# 1. カウンター情報確認
show thread counter

# 2. エラー回数確認
show thread counter

# 3. ワーカー再起動
restart worker lcore all
```

## 最適化のポイント

### CPU親和性の設定
- 適切なlcoreにワーカーを配置
- NUMAノードを考慮した配置
- CPU使用率の均等化

### メモリ効率の改善
- 適切なメモリプールサイズ
- メモリリークの防止
- キャッシュ効率の向上

### 統計情報の活用
- パフォーマンス監視
- 容量計画
- 異常検知

## 定義場所

これらのコマンドは以下のファイルで定義されています：
- `sdplane/dpdk_lcore_cmd.c` - ワーカー管理コマンド
- `sdplane/thread_info.c` - スレッド情報コマンド

## 関連項目

- [ポート管理・統計](port-management.md)
- [lthread管理](lthread-management.md)
- [システム情報・監視](system-monitoring.md)
- [デバッグ・ログ](debug-logging.md)