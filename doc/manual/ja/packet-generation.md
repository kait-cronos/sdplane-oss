# パケット生成

**言語 / Language:** [English](../packet-generation.md) | **日本語**

PKTGEN（Packet Generator）を使用したパケット生成機能のコマンドです。

## コマンド一覧

### show_pktgen - PKTGEN情報表示
```
show pktgen
```

PKTGEN（パケットジェネレーター）の現在の状態と設定を表示します。

**使用例：**
```bash
show pktgen
```

このコマンドは以下の情報を表示します：
- PKTGEN の初期化状態
- 現在の設定パラメータ
- 実行中のタスク
- 統計情報

### pktgen_init - PKTGEN初期化
```
pktgen init argv-list <0-7>
```

PKTGENを指定されたargv-listを使用して初期化します。

**パラメータ：**
- `<0-7>` - 使用するargv-listのインデックス

**使用例：**
```bash
# argv-list 0を使用してPKTGENを初期化
pktgen init argv-list 0

# argv-list 2を使用してPKTGENを初期化
pktgen init argv-list 2
```

### pktgen_do - PKTGENコマンド実行
```
pktgen do (start|stop) (<0-7>|all)
```

PKTGENの開始または停止を実行します。

**コマンド：**
- `start` - パケット生成を開始
- `stop` - パケット生成を停止

**対象：**
- `<0-7>` - 特定のポート番号
- `all` - 全ポート

**使用例：**
```bash
# ポート0でパケット生成を開始
pktgen do start 0

# 全ポートでパケット生成を開始
pktgen do start all

# ポート1でパケット生成を停止
pktgen do stop 1

# 全ポートでパケット生成を停止
pktgen do stop all
```

## PKTGENの概要

### PKTGENとは
PKTGEN（Packet Generator）は、ネットワークテストのためのパケット生成ツールです。以下の機能を提供します：

- **高速パケット生成** - 高性能なパケット生成
- **多様なパケット形式** - 様々なプロトコルに対応
- **柔軟な設定** - 詳細なパケット設定が可能
- **統計機能** - 詳細な統計情報の提供

### 主な用途
- **ネットワーク性能テスト** - スループットやレイテンシの測定
- **負荷テスト** - システムの負荷耐性テスト
- **機能テスト** - ネットワーク機能の検証
- **ベンチマーク** - 性能比較テスト

## PKTGENの設定

### 基本的な設定手順
1. **argv-listの設定**
```bash
# PKTGEN用のパラメータを設定
set argv-list 0 "-c 0x3 -n 4"
set argv-list 1 "--socket-mem 1024"
set argv-list 2 "--huge-dir /mnt/huge"
```

2. **PKTGENの初期化**
```bash
pktgen init argv-list 0
```

3. **ワーカーの設定**
```bash
set worker lcore 1 pktgen
start worker lcore 1
```

4. **パケット生成の開始**
```bash
pktgen do start 0
```

### 設定パラメータ
argv-listで設定可能なパラメータの例：

- **-c** - CPUマスク
- **-n** - メモリチャンネル数
- **--socket-mem** - ソケットメモリサイズ
- **--huge-dir** - ヒュージページディレクトリ
- **--file-prefix** - ファイルプレフィックス

## 使用例

### 基本的なパケット生成
```bash
# 設定
set argv-list 0 "-c 0x3 -n 4 --socket-mem 1024"

# 初期化
pktgen init argv-list 0

# ワーカー設定
set worker lcore 1 pktgen
start worker lcore 1

# パケット生成開始
pktgen do start 0

# 状態確認
show pktgen

# パケット生成停止
pktgen do stop 0
```

### 複数ポートでの生成
```bash
# 複数ポートで開始
pktgen do start all

# 状態確認
show pktgen

# 複数ポートで停止
pktgen do stop all
```

## 監視と統計

### 統計情報の確認
```bash
# PKTGEN統計の表示
show pktgen

# ポート統計の表示
show port statistics

# ワーカー統計の表示
show worker
```

### パフォーマンス監視
```bash
# PPS（Packets Per Second）の確認
show port statistics pps

# 総パケット数の確認
show port statistics total

# バイト/秒の確認
show port statistics Bps
```

## トラブルシューティング

### PKTGENが開始されない場合
1. 初期化状態を確認
```bash
show pktgen
```

2. ワーカーの状態確認
```bash
show worker
```

3. ポートの状態確認
```bash
show port
```

### パケット生成が停止しない場合
1. 明示的に停止
```bash
pktgen do stop all
```

2. ワーカーの再起動
```bash
restart worker lcore 1
```

### パフォーマンスが低い場合
1. CPU使用率を確認
2. メモリ設定を確認
3. ポート設定を確認

## 高度な機能

### パケット形式の設定
PKTGENでは、様々なパケット形式を生成できます：
- **Ethernet** - 基本的なEthernetフレーム
- **IP** - IPv4/IPv6パケット
- **UDP/TCP** - UDP/TCPパケット
- **VLAN** - VLANタグ付きパケット

### 負荷制御
- **レート制御** - パケット生成レートの制御
- **バースト制御** - バーストパケットの生成
- **サイズ制御** - パケットサイズの制御

## 定義場所

これらのコマンドは以下のファイルで定義されています：
- `sdplane/pktgen_cmd.c`

## 関連項目

- [ワーカー・lcore管理](worker-management.md)
- [ポート管理・統計](port-management.md)
- [システム情報・監視](system-monitoring.md)