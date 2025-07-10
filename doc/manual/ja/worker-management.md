# ワーカー・lcore管理

**言語 / Language:** [English](../worker-management.md) | **日本語**

DPDKワーカースレッドとlcoreの管理を行うコマンドです。

## コマンド一覧

### set_worker - ワーカーの設定
```
(set|reset|start|restart) worker lcore <0-16> (|none|l2fwd|l3fwd|l3fwd-lpm|tap-handler|l2-repeater|nettlp-thread|vlan-switch|pktgen|linkflap-generator)
```

指定されたlcoreにワーカーを設定・リセット・開始・再開始します。

**ワーカータイプ：**
- `none` - ワーカーなし
- `l2fwd` - Layer 2フォワーディング
- `l3fwd` - Layer 3フォワーディング
- `l3fwd-lpm` - Layer 3フォワーディング (LPM)
- `tap-handler` - TAPインターフェースハンドラー
- `l2-repeater` - Layer 2リピーター
- `nettlp-thread` - NetTLPスレッド
- `vlan-switch` - VLANスイッチ
- `pktgen` - パケットジェネレーター
- `linkflap-generator` - リンクフラップジェネレーター

**使用例：**
```bash
# lcore 1にL2フォワーディングワーカーを設定
set worker lcore 1 l2fwd

# lcore 2のワーカーをリセット
reset worker lcore 2

# lcore 3にL3フォワーディング(LPM)ワーカーを開始
start worker lcore 3 l3fwd-lpm

# lcore 4のワーカーを再開始
restart worker lcore 4
```

### start_stop_worker - ワーカーの開始・停止
```
(start|stop|reset|restart) worker lcore (<0-16>|all)
```

指定されたlcoreのワーカーを開始・停止・リセット・再開始します。

**使用例：**
```bash
# lcore 1のワーカーを開始
start worker lcore 1

# 全ワーカーを停止
stop worker lcore all

# lcore 2のワーカーをリセット
reset worker lcore 2

# lcore 3のワーカーを再開始
restart worker lcore 3
```

### show_worker - ワーカー情報の表示
```
show worker
```

現在のワーカーの状態と設定を表示します。

**使用例：**
```bash
show worker
```

### set_mempool - メモリプールの設定
```
set mempool
```

DPDKメモリプールを設定します。

**使用例：**
```bash
set mempool
```

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

**使用例：**
```bash
rte_eal_init
```

## ワーカータイプの説明

### L2フォワーディング (l2fwd)
Layer 2レベルでのパケット転送を行うワーカーです。MACアドレスベースでパケットを転送します。

### L3フォワーディング (l3fwd)
Layer 3レベルでのパケット転送を行うワーカーです。IPアドレスベースでルーティングを行います。

### L3フォワーディング LPM (l3fwd-lpm)
Longest Prefix Matching（LPM）を使用したLayer 3フォワーディングワーカーです。

### TAPハンドラー (tap-handler)
TAPインターフェースとDPDKポート間でのパケット転送を行うワーカーです。

### L2リピーター (l2-repeater)
Layer 2レベルでのパケット複製・リピートを行うワーカーです。

### NetTLPスレッド (nettlp-thread)
Network TLP（Transaction Layer Protocol）機能を処理するワーカーです。

### VLANスイッチ (vlan-switch)
VLAN（Virtual LAN）機能を提供するスイッチングワーカーです。

### パケットジェネレーター (pktgen)
テスト用のパケットを生成するワーカーです。

### リンクフラップジェネレーター (linkflap-generator)
ネットワークリンクの状態変化をテストするためのワーカーです。

## 定義場所

これらのコマンドは以下のファイルで定義されています：
- `sdplane/dpdk_lcore_cmd.c`

## 関連項目

- [ポート管理・統計](port-management.md)
- [スレッド情報](thread-information.md)
- [lthread管理](lthread-management.md)