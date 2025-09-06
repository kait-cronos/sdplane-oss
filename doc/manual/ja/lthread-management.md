# lthread管理

**言語 / Language:** [English](../lthread-management.md) | **日本語** | [Français](../fr/lthread-management.md) | [中文](../zh/lthread-management.md) | [Deutsch](../de/lthread-management.md) | [Italiano](../it/lthread-management.md) | [한국어](../ko/lthread-management.md) | [ไทย](../th/lthread-management.md) | [Español](../es/lthread-management.md)

lthread（軽量スレッド）の管理を行うコマンドです。

## コマンド一覧

### set_worker_lthread_stat_collector - lthread統計コレクター設定
```
set worker lthread stat-collector
```

統計情報を収集するlthreadワーカーを設定します。

**使用例：**
```bash
set worker lthread stat-collector
```

### set_worker_lthread_rib_manager - lthread RIBマネージャー設定
```
set worker lthread rib-manager
```

RIB（Routing Information Base）を管理するlthreadワーカーを設定します。

**使用例：**
```bash
set worker lthread rib-manager
```

### set_worker_lthread_netlink_thread - lthread netlinkスレッド設定
```
set worker lthread netlink-thread
```

Netlink通信を処理するlthreadワーカーを設定します。

**使用例：**
```bash
set worker lthread netlink-thread
```

## lthreadの概要

### lthreadとは
lthread（lightweight thread）は、協調的マルチスレッドを実現する軽量スレッド実装です。

### 主な特徴
- **軽量性** - 低いメモリオーバーヘッド
- **高速性** - 高速なコンテキストスイッチ
- **協調性** - 明示的なyieldによる制御
- **スケーラビリティ** - 大量のスレッドを効率的に管理

### 従来のスレッドとの違い
- **プリエンプティブ vs 協調的** - 明示的な制御権の譲渡
- **カーネルスレッド vs ユーザースレッド** - カーネルを介さない実行
- **重い vs 軽い** - 作成・切り替えのオーバーヘッドが小さい

## lthreadワーカーの種類

### 統計コレクター (stat-collector)
システムの統計情報を定期的に収集するワーカーです。

**機能：**
- ポート統計の収集
- スレッド統計の収集
- システム統計の収集
- 統計データの集計

**使用例：**
```bash
# 統計コレクターを設定
set worker lthread stat-collector

# 統計情報を確認
show port statistics
show thread counter
```

### RIBマネージャー (rib-manager)
RIB（Routing Information Base）を管理するワーカーです。

**機能：**
- ルーティングテーブルの管理
- ルートの追加・削除・更新
- ルート情報の配信
- ルーティング状態の監視

**使用例：**
```bash
# RIBマネージャーを設定
set worker lthread rib-manager

# RIB情報を確認
show rib
```

### Netlinkスレッド (netlink-thread)
Netlink通信を処理するワーカーです。

**機能：**
- カーネルとのNetlink通信
- ネットワーク設定の監視
- インターフェース状態の監視
- ルーティング情報の受信

**使用例：**
```bash
# Netlinkスレッドを設定
set worker lthread netlink-thread

# ネットワーク状態を確認
show port
show vswitch
```

## lthreadの設定

### 基本的な設定手順
1. **必要なlthreadワーカーの設定**
```bash
# 統計コレクターを設定
set worker lthread stat-collector

# RIBマネージャーを設定
set worker lthread rib-manager

# Netlinkスレッドを設定
set worker lthread netlink-thread
```

2. **設定確認**
```bash
# ワーカー状態の確認
show worker

# スレッド情報の確認
show thread
```

### 推奨設定
一般的な用途では、以下の組み合わせを推奨します：
```bash
# 基本的なlthreadワーカー設定
set worker lthread stat-collector
set worker lthread rib-manager
set worker lthread netlink-thread
```

## 使用例

### 基本的な設定
```bash
# lthreadワーカーを設定
set worker lthread stat-collector
set worker lthread rib-manager
set worker lthread netlink-thread

# 設定確認
show worker
show thread
```

### 統計監視の設定
```bash
# 統計コレクターを設定
set worker lthread stat-collector

# 統計情報を定期的に確認
show port statistics
show thread counter
show mempool
```

### ルーティング管理の設定
```bash
# RIBマネージャーを設定
set worker lthread rib-manager

# ルーティング情報を確認
show rib
show vswitch
```

## 監視と管理

### lthread状態の確認
```bash
# 全体的な状態確認
show worker
show thread

# 特定の統計情報確認
show thread counter
show loop-count console pps
```

### パフォーマンス監視
```bash
# lthreadのパフォーマンス確認
show thread counter

# システム全体のパフォーマンス確認
show port statistics pps
show mempool
```

## トラブルシューティング

### lthreadが動作しない場合
1. ワーカー設定確認
```bash
show worker
```

2. スレッド状態確認
```bash
show thread
```

3. システム状態確認
```bash
show rcu
show mempool
```

### 統計情報が更新されない場合
1. 統計コレクターの状態確認
```bash
show worker
show thread
```

2. 統計情報の手動確認
```bash
show port statistics
show thread counter
```

### RIBが更新されない場合
1. RIBマネージャーの状態確認
```bash
show worker
show thread
```

2. RIB情報の確認
```bash
show rib
```

3. Netlinkスレッドの状態確認
```bash
show worker
```

## 高度な機能

### lthreadの協調動作
lthreadは協調的に動作するため、以下の点に注意が必要です：

- **明示的なyield** - 長時間の処理では明示的にyieldが必要
- **デッドロック回避** - 協調的な設計によりデッドロックを回避
- **公平性** - 全てのスレッドが適切に実行される

### パフォーマンス最適化
- **適切なワーカー配置** - CPUコアとの親和性を考慮
- **メモリ効率** - 適切なメモリプールサイズの設定
- **負荷分散** - 複数のワーカーでの負荷分散

## 定義場所

これらのコマンドは以下のファイルで定義されています：
- `sdplane/lthread_main.c`

## 関連項目

- [ワーカー・lcore管理](worker-management.md)
- [スレッド情報](thread-information.md)
- [システム情報・監視](system-monitoring.md)
- [RIB・ルーティング](routing.md)