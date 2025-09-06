# TAPインターフェース

**言語 / Language:** [English](../tap-interface.md) | **日本語** | [Français](../fr/tap-interface.md) | [中文](../zh/tap-interface.md) | [Deutsch](../de/tap-interface.md) | [Italiano](../it/tap-interface.md) | [한국어](../ko/tap-interface.md) | [ไทย](../th/tap-interface.md) | [Español](../es/tap-interface.md)

TAPインターフェースの管理を行うコマンドです。

## コマンド一覧

### set_tap_capture_ifname - TAPキャプチャインターフェース名設定
```
set tap capture ifname <WORD>
```

TAPキャプチャ機能で使用するインターフェース名を設定します。

**パラメータ：**
- `<WORD>` - インターフェース名

**使用例：**
```bash
# tap0インターフェースを設定
set tap capture ifname tap0

# tap1インターフェースを設定
set tap capture ifname tap1
```

### set_tap_capture_persistent - TAPキャプチャ永続化有効化
```
set tap capture persistent
```

TAPキャプチャの永続化を有効化します。

**使用例：**
```bash
# 永続化を有効化
set tap capture persistent
```

### no_tap_capture_persistent - TAPキャプチャ永続化無効化
```
no tap capture persistent
```

TAPキャプチャの永続化を無効化します。

**使用例：**
```bash
# 永続化を無効化
no tap capture persistent
```

### unset_tap_capture_persistent - TAPキャプチャ永続化設定削除
```
unset tap capture persistent
```

TAPキャプチャの永続化設定を削除します。

**使用例：**
```bash
# 永続化設定を削除
unset tap capture persistent
```

## TAPインターフェースの概要

### TAPインターフェースとは
TAP（Network TAP）インターフェースは、ネットワークトラフィックの監視やテストに使用される仮想ネットワークインターフェースです。

### 主な機能
- **パケットキャプチャ** - ネットワークトラフィックのキャプチャ
- **パケット注入** - テストパケットの注入
- **ブリッジ機能** - 異なるネットワーク間のブリッジ
- **監視機能** - トラフィックの監視と分析

### sdplaneでの用途
- **デバッグ** - パケットフローのデバッグ
- **テスト** - ネットワーク機能のテスト
- **監視** - トラフィックの監視
- **開発** - 新機能の開発とテスト

## TAPインターフェースの設定

### 基本的な設定手順
1. **TAPインターフェースの作成**
```bash
# システムレベルでTAPインターフェースを作成
sudo ip tuntap add tap0 mode tap
sudo ip link set tap0 up
```

2. **sdplaneでの設定**
```bash
# TAPキャプチャインターフェース名を設定
set tap capture ifname tap0

# 永続化を有効化
set tap capture persistent
```

3. **TAPハンドラーワーカーの設定**
```bash
# TAPハンドラーワーカーを設定
set worker lcore 2 tap-handler
start worker lcore 2
```

### 設定例

#### 基本的なTAP設定
```bash
# TAPインターフェースの設定
set tap capture ifname tap0
set tap capture persistent

# ワーカーの設定
set worker lcore 2 tap-handler
start worker lcore 2

# 設定確認
show worker
```

#### 複数TAPインターフェースの設定
```bash
# 複数のTAPインターフェースを設定
set tap capture ifname tap0
set tap capture ifname tap1

# 永続化を有効化
set tap capture persistent
```

## 永続化機能

### 永続化とは
永続化機能を有効にすると、TAPインターフェースの設定がシステム再起動後も保持されます。

### 永続化の利点
- **設定の保持** - 再起動後も設定が有効
- **自動復旧** - システム障害からの自動復旧
- **運用効率** - 手動設定の削減

### 永続化の設定
```bash
# 永続化を有効化
set tap capture persistent

# 永続化を無効化
no tap capture persistent

# 永続化設定を削除
unset tap capture persistent
```

## 使用例

### デバッグ用途
```bash
# デバッグ用TAPインターフェースの設定
set tap capture ifname debug-tap
set tap capture persistent

# TAPハンドラーワーカーを設定
set worker lcore 3 tap-handler
start worker lcore 3

# パケットキャプチャを開始
tcpdump -i debug-tap
```

### テスト用途
```bash
# テスト用TAPインターフェースの設定
set tap capture ifname test-tap
set tap capture persistent

# テストパケットの注入準備
set worker lcore 4 tap-handler
start worker lcore 4
```

## 監視と管理

### TAPインターフェースの状態確認
```bash
# ワーカーの状態確認
show worker

# スレッド情報の確認
show thread

# システムレベルでの確認
ip link show tap0
```

### トラフィック監視
```bash
# tcpdumpを使用した監視
tcpdump -i tap0

# Wiresharkを使用した監視
wireshark -i tap0
```

## トラブルシューティング

### TAPインターフェースが作成されない場合
1. システムレベルでの確認
```bash
# TAPインターフェースの存在確認
ip link show tap0

# 権限の確認
sudo ip tuntap add tap0 mode tap
```

2. sdplaneでの確認
```bash
# 設定確認
show worker

# ワーカーの状態確認
show thread
```

### パケットがキャプチャされない場合
1. インターフェースの状態確認
```bash
ip link show tap0
```

2. ワーカーの状態確認
```bash
show worker
```

3. TAPハンドラーの再起動
```bash
restart worker lcore 2
```

### 永続化が機能しない場合
1. 永続化設定の確認
```bash
# 現在の設定を確認（show系コマンドで確認）
show worker
```

2. システム設定の確認
```bash
# システムレベルでの設定確認
systemctl status sdplane
```

## 高度な機能

### VLANとの連携
TAPインターフェースはVLAN機能と連携して使用できます：
```bash
# VLANスイッチワーカーとの連携
set worker lcore 5 vlan-switch
start worker lcore 5
```

### ブリッジ機能
複数のTAPインターフェースをブリッジして使用：
```bash
# 複数のTAPインターフェース設定
set tap capture ifname tap0
set tap capture ifname tap1
```

## 定義場所

これらのコマンドは以下のファイルで定義されています：
- `sdplane/tap_cmd.c`

## 関連項目

- [ワーカー・lcore管理](worker-management.md)
- [VTY・シェル管理](vty-shell.md)
- [デバッグ・ログ](debug-logging.md)