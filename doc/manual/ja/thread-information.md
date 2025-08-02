# スレッド情報

**言語 / Language:** [English](../thread-information.md) | **日本語**

スレッドの情報と監視を行うコマンドです。

## コマンド一覧

### show_thread_cmd - スレッド情報表示
```
show thread
```

現在のスレッドの状態と情報を表示します。

**使用例：**
```bash
show thread
```

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

**使用例：**
```bash
show thread counter
```

このコマンドは以下の情報を表示します：
- 処理パケット数
- 実行回数
- エラー回数
- 処理時間統計

## スレッドシステムの概要

### sdplaneのスレッドアーキテクチャ
sdplaneでは以下のスレッドタイプが使用されています：

#### 1. ワーカースレッド
- **L2フォワーディング** - Layer 2パケット転送
- **L3フォワーディング** - Layer 3パケット転送
- **TAPハンドラー** - TAPインターフェース処理
- **PKTGEN** - パケット生成

#### 2. 管理スレッド
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
- **State** - スレッドの状態
- **lcore** - 実行中のCPUコア
- **Type** - スレッドタイプ

### スレッドの状態
- **Running** - 実行中
- **Ready** - 実行可能
- **Blocked** - ブロック中
- **Terminated** - 終了済み

### カウンター情報
- **Packets** - 処理パケット数
- **Loops** - ループ実行回数
- **Errors** - エラー回数
- **CPU Time** - CPU使用時間

## 使用例

### 基本的な監視
```bash
# スレッド情報の表示
show thread

# スレッドカウンターの表示
show thread counter
```

### 出力例の解釈
```bash
# show threadの出力例
Thread ID: 1
Name: l2fwd-worker
State: Running
lcore: 1
Type: L2FWD

Thread ID: 2
Name: rib-manager
State: Running
lcore: 2
Type: RIB_MANAGER
```

```bash
# show thread counterの出力例
Thread ID: 1
Packets: 1000000
Loops: 5000000
Errors: 0
CPU Time: 123.45s
```

## 監視とトラブルシューティング

### 定期監視
```bash
# 定期的な監視コマンド
show thread
show thread counter
```

### パフォーマンス分析
```bash
# パフォーマンス関連情報
show thread counter
show loop-count l2fwd pps
show worker
```

### トラブルシューティング

#### スレッドが応答しない場合
1. スレッド状態を確認
```bash
show thread
```

2. ワーカーの状態確認
```bash
show worker
```

3. 必要に応じて再起動
```bash
restart worker lcore 1
```

#### パフォーマンスが低下している場合
1. カウンター情報を確認
```bash
show thread counter
```

2. ループカウンターを確認
```bash
show loop-count l2fwd pps
```

3. エラー回数を確認
```bash
show thread counter
```

#### メモリ使用量が多い場合
1. メモリプール情報を確認
```bash
show mempool
```

2. スレッド情報を確認
```bash
show thread
```

## スレッドの最適化

### CPU親和性の設定
- 適切なlcoreにワーカーを配置
- NUMAノードを考慮した配置
- CPU使用率の均等化

### メモリ効率の改善
- 適切なメモリプールサイズ
- メモリリークの防止
- キャッシュ効率の向上

## 高度な機能

### lthread管理
```bash
# lthreadワーカーの設定
set worker lthread stat-collector
set worker lthread rib-manager
set worker lthread netlink-thread
```

### 統計情報の活用
- パフォーマンス監視
- 容量計画
- 異常検知

## 定義場所

これらのコマンドは以下のファイルで定義されています：
- `sdplane/thread_info.c`

## 関連項目

- [ワーカー・lcore管理](worker-management.md)
- [lthread管理](lthread-management.md)
- [システム情報・監視](system-monitoring.md)