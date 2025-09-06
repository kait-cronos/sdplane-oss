# RIB・ルーティング

**言語 / Language:** [English](../routing.md) | **日本語** | [Français](../fr/routing.md) | [中文](../zh/routing.md) | [Deutsch](../de/routing.md) | [Italiano](../it/routing.md) | [한국어](../ko/routing.md) | [ไทย](../th/routing.md) | [Español](../es/routing.md)

RIB（Routing Information Base）とシステムリソース情報を管理するコマンドです。

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
- RIBバージョンとメモリポインタ
- 仮想スイッチ構成とVLAN割り当て
- DPDKポート状態とキュー構成
- lcore-to-portキュー割り当て
- L2/L3転送用のネイバーテーブル

## RIBの概要

### RIBとは
RIB（Routing Information Base）は、システムリソースとネットワーク情報を格納する中央データベースです。sdplaneでは、以下の情報を管理しています：

- **仮想スイッチ構成** - VLANスイッチングとポート割り当て
- **DPDKポート情報** - リンク状態、キュー構成、機能情報
- **lcoreキュー構成** - CPUコアごとのパケット処理割り当て
- **ネイバーテーブル** - L2/L3転送データベースエントリ

### RIBの構造
RIBは2つの主要な構造体で構成されています：

```c
struct rib {
    struct rib_info *rib_info;  // 実際のデータへのポインタ
};

struct rib_info {
    uint32_t ver;                                    // バージョン番号
    uint8_t vswitch_size;                           // 仮想スイッチ数
    uint8_t port_size;                              // DPDKポート数
    uint8_t lcore_size;                             // lcore数
    struct vswitch_conf vswitch[MAX_VSWITCH];       // 仮想スイッチ構成
    struct vswitch_link vswitch_link[MAX_VSWITCH_LINK]; // VLANポートリンク
    struct port_conf port[MAX_ETH_PORTS];           // DPDKポート構成
    struct lcore_qconf lcore_qconf[RTE_MAX_LCORE];  // lcoreキュー割り当て
    struct neigh_table neigh_tables[NEIGH_NR_TABLES]; // ネイバー/転送テーブル
};
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
rib information version: 21 (0x55555dd42010)
vswitches: 
dpdk ports: 
  dpdk_port[0]: 
    link: speed=1000Mbps duplex=full autoneg=on status=up
    nb_rxd=1024 nb_txd=1024
    queues: nrxq=1 ntxq=4
  dpdk_port[1]: 
    link: speed=0Mbps duplex=half autoneg=on status=down
    nb_rxd=1024 nb_txd=1024
    queues: nrxq=1 ntxq=4
  dpdk_port[2]: 
    link: speed=0Mbps duplex=half autoneg=off status=down
    nb_rxd=1024 nb_txd=1024
    queues: nrxq=1 ntxq=4
lcores: 
  lcore[0]: nrxq=0
  lcore[1]: nrxq=1
    rxq[0]: dpdk_port[0], queue_id=0
  lcore[2]: nrxq=1
    rxq[0]: dpdk_port[1], queue_id=0
  lcore[3]: nrxq=1
    rxq[0]: dpdk_port[2], queue_id=0
  lcore[4]: nrxq=0
  lcore[5]: nrxq=0
  lcore[6]: nrxq=0
  lcore[7]: nrxq=0
```

この例では：
- RIBバージョン21が現在のシステム状態を示す
- DPDKポート0がアクティブ（up）で1Gbpsリンク速度
- DPDKポート1、2は非アクティブ（down）でリンクなし
- lcore 1、2、3がそれぞれポート0、1、2からのパケット処理を担当
- 各ポートは1個のRXキューと4個のTXキューを使用
- RX/TXディスクリプタリングは1024エントリで設定

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

- [ワーカー・lcore管理](worker-lcore-thread-management.md)
- [lthread管理](lthread-management.md)
- [システム情報・監視](system-monitoring.md)