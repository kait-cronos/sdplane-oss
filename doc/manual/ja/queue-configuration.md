# キュー設定

**言語 / Language:** [English](../queue-configuration.md) | **日本語** | [Français](../fr/queue-configuration.md) | [中文](../zh/queue-configuration.md) | [Deutsch](../de/queue-configuration.md) | [Italiano](../it/queue-configuration.md) | [한국어](../ko/queue-configuration.md) | [ไทย](../th/queue-configuration.md) | [Español](../es/queue-configuration.md)

DPDKキューの設定と管理を行うコマンドです。

## コマンド一覧

### update_port_status - ポートステータス更新
```
update port status
```

全ポートのステータスを更新します。

**使用例：**
```bash
update port status
```

このコマンドは以下を実行します：
- 各ポートのリンク状態を確認
- キュー設定を更新
- ポートの統計情報を更新

### set_thread_lcore_port_queue - スレッドキュー設定
```
set thread <0-128> port <0-128> queue <0-128>
```

指定されたスレッドにポートとキューを割り当てます。

**パラメータ：**
- `<0-128>` (thread) - スレッド番号
- `<0-128>` (port) - ポート番号
- `<0-128>` (queue) - キュー番号

**使用例：**
```bash
# スレッド0にポート0のキュー0を割り当て
set thread 0 port 0 queue 0

# スレッド1にポート1のキュー1を割り当て
set thread 1 port 1 queue 1

# スレッド2にポート0のキュー1を割り当て
set thread 2 port 0 queue 1
```

### show_thread_qconf - スレッドキュー設定表示
```
show thread qconf
```

現在のスレッドキュー設定を表示します。

**使用例：**
```bash
show thread qconf
```

## キューシステムの概要

### DPDKキューの概念
DPDKでは、各ポートに複数の送信キューと受信キューを設定できます：

- **受信キュー（RX Queue）** - 着信パケットを受信
- **送信キュー（TX Queue）** - 発信パケットを送信
- **マルチキュー** - 複数のキューを並列処理

### キュー設定の重要性
適切なキュー設定により以下が実現されます：
- **パフォーマンス向上** - 並列処理による高速化
- **負荷分散** - 複数のワーカーでの処理分散
- **CPU効率** - CPUコアの有効活用

## キュー設定の方法

### 基本的な設定手順
1. **ポートステータス更新**
```bash
update port status
```

2. **スレッドキュー設定**
```bash
set thread 0 port 0 queue 0
set thread 1 port 0 queue 1
```

3. **設定確認**
```bash
show thread qconf
```

### 推奨設定パターン

#### 単一ポート、単一キュー
```bash
set thread 0 port 0 queue 0
```

#### 単一ポート、複数キュー
```bash
set thread 0 port 0 queue 0
set thread 1 port 0 queue 1
set thread 2 port 0 queue 2
```

#### 複数ポート、複数キュー
```bash
set thread 0 port 0 queue 0
set thread 1 port 1 queue 0
set thread 2 port 2 queue 0
```

## 設定例

### 高性能設定（4コア、4ポート）
```bash
# ポートステータス更新
update port status

# 各コアに異なるポートを割り当て
set thread 0 port 0 queue 0
set thread 1 port 1 queue 0
set thread 2 port 2 queue 0
set thread 3 port 3 queue 0

# 設定確認
show thread qconf
```

### 負荷分散設定（2コア、1ポート）
```bash
# ポートステータス更新
update port status

# 1つのポートに複数のキューを設定
set thread 0 port 0 queue 0
set thread 1 port 0 queue 1

# 設定確認
show thread qconf
```

## パフォーマンスチューニング

### キュー数の決定
- **CPUコア数** - 利用可能なCPUコア数に応じて設定
- **ポート数** - 物理ポート数を考慮
- **トラフィック特性** - 予想されるトラフィックパターン

### 最適化のポイント
1. **CPU親和性** - CPUコアとキューの適切な配置
2. **メモリ配置** - NUMAノードを考慮したメモリ配置
3. **割り込み処理** - 効率的な割り込み処理

## トラブルシューティング

### キュー設定が反映されない場合
1. ポートステータスを更新
```bash
update port status
```

2. ワーカーの状態確認
```bash
show worker
```

3. ポートの状態確認
```bash
show port
```

### パフォーマンスが向上しない場合
1. キュー設定を確認
```bash
show thread qconf
```

2. スレッドの負荷確認
```bash
show thread counter
```

3. ポート統計の確認
```bash
show port statistics
```

## 定義場所

これらのコマンドは以下のファイルで定義されています：
- `sdplane/queue_config.c`

## 関連項目

- [ポート管理・統計](port-management.md)
- [ワーカー・lcore管理](worker-management.md)
- [スレッド情報](thread-information.md)