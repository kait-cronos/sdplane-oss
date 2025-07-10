# RIB・ルーティング

**言語 / Language:** [English](../routing.md) | **日本語**

RIB（Routing Information Base）とルーティング機能を管理するコマンドです。

## コマンド一覧

### show_rib - RIB情報表示
```
show rib
```

RIB（Routing Information Base）の情報を表示します。

**使用例：**
```bash
show rib
```

このコマンドは以下の情報を表示します：
- 現在のルーティングテーブル
- 各ルートの状態
- ネクストホップ情報
- メトリック情報

## RIBの概要

### RIBとは
RIB（Routing Information Base）は、ルーティング情報を格納するデータベースです。sdplaneでは、以下の情報を管理しています：

- **ルーティングテーブル** - IP宛先とネクストホップの対応
- **ルートの状態** - アクティブ、非アクティブ、削除予定など
- **メトリック** - ルートの優先度やコスト
- **インターフェース情報** - 出力インターフェースの情報

### RIBの構造
```
Destination Network → Next Hop → Interface → Metric
```

## RIB情報の見方

### 基本的な表示項目
- **Destination** - 宛先ネットワークアドレス
- **Netmask** - ネットマスク
- **Gateway** - ゲートウェイ（ネクストホップ）
- **Interface** - 出力インターフェース
- **Metric** - ルートのメトリック値
- **Status** - ルートの状態

### ルートの状態
- **Active** - アクティブなルート
- **Inactive** - 非アクティブなルート
- **Pending** - 設定中のルート
- **Invalid** - 無効なルート

## 使用例

### 基本的な使用方法
```bash
# RIB情報の表示
show rib
```

### 出力例の解釈
```
Destination     Netmask         Gateway         Interface    Metric  Status
192.168.1.0     255.255.255.0   192.168.1.1     eth0         1       Active
10.0.0.0        255.0.0.0       10.0.0.1        eth1         1       Active
0.0.0.0         0.0.0.0         192.168.1.1     eth0         1       Active
```

この例では：
- 192.168.1.0/24 ネットワークはeth0インターフェースを通じてアクセス可能
- 10.0.0.0/8 ネットワークはeth1インターフェースを通じてアクセス可能
- デフォルトルート（0.0.0.0/0）は192.168.1.1を通じてeth0へ

## RIBの管理

### 自動更新
RIBは以下のタイミングで自動的に更新されます：
- インターフェースの状態変更
- ネットワーク設定の変更
- ルーティングプロトコルからの情報受信

### 手動更新
RIB情報を手動で確認するには：
```bash
show rib
```

## トラブルシューティング

### ルーティングが正しく動作しない場合
1. RIB情報を確認
```bash
show rib
```

2. インターフェース状態を確認
```bash
show port
```

3. ワーカーの状態を確認
```bash
show worker
```

### RIBにルートが表示されない場合
- ネットワーク設定を確認
- インターフェースの状態を確認
- RIBマネージャーの動作を確認

## 高度な機能

### RIBマネージャー
RIBマネージャーは独立したスレッドとして動作し、以下の機能を提供します：
- ルーティング情報の自動更新
- ルートの有効性チェック
- ネットワーク状態の監視

### 関連するワーカー
- **rib-manager** - RIBの管理を行うワーカー
- **l3fwd** - Layer 3フォワーディングでRIBを利用
- **l3fwd-lpm** - LPMテーブルとRIBの連携

## 定義場所

このコマンドは以下のファイルで定義されています：
- `sdplane/rib.c`

## 関連項目

- [ワーカー・lcore管理](worker-management.md)
- [lthread管理](lthread-management.md)
- [システム情報・監視](system-monitoring.md)