# ポート管理・統計

**言語 / Language:** [English](../port-management.md) | **日本語**

DPDKポートの管理と統計情報を扱うコマンドです。

## コマンド一覧

### start_port - ポート開始
```
start port
```

DPDKポートを開始します（デフォルト動作）。

**使用例：**
```bash
# ポートを開始（デフォルト）
start port
```

### start_port_specific - 特定ポート開始
```
start port <0-16>
```

特定のDPDKポートを開始します。

**使用例：**
```bash
# ポート0を開始
start port 0

# ポート1を開始
start port 1
```

### start_port_all - 全ポート開始
```
start port all
```

全てのDPDKポートを開始します。

**使用例：**
```bash
# 全ポートを開始
start port all
```

### stop_port - ポート停止
```
stop port
```

DPDKポートを停止します（デフォルト動作）。

**使用例：**
```bash
# ポートを停止（デフォルト）
stop port
```

### stop_port_specific - 特定ポート停止
```
stop port <0-16>
```

特定のDPDKポートを停止します。

**使用例：**
```bash
# ポート0を停止
stop port 0

# ポート1を停止
stop port 1
```

### stop_port_all - 全ポート停止
```
stop port all
```

全てのDPDKポートを停止します。

**使用例：**
```bash
# 全ポートを停止
stop port all
```

### reset_port - ポートリセット
```
reset port
```

DPDKポートをリセットします（デフォルト動作）。

**使用例：**
```bash
# ポートをリセット（デフォルト）
reset port
```

### reset_port_specific - 特定ポートリセット
```
reset port <0-16>
```

特定のDPDKポートをリセットします。

**使用例：**
```bash
# ポート0をリセット
reset port 0

# ポート1をリセット
reset port 1
```

### reset_port_all - 全ポートリセット
```
reset port all
```

全てのDPDKポートをリセットします。

**使用例：**
```bash
# 全ポートをリセット
reset port all
```

### show_port - ポート情報の表示
```
show port
```

全ポートの基本情報を表示します（デフォルト動作）。

**使用例：**
```bash
# 全ポートの情報を表示
show port
```

### show_port_specific - 特定ポート情報の表示
```
show port <0-16>
```

特定のポートの基本情報を表示します。

**使用例：**
```bash
# ポート0の情報を表示
show port 0

# ポート1の情報を表示
show port 1
```

### show_port_all - 全ポート情報の表示
```
show port all
```

全ポートの情報を明示的に表示します。

**使用例：**
```bash
# 全ポートの情報を明示的に表示
show port all
```

### show_port_statistics - ポート統計情報の表示
```
show port statistics
```

全てのポート統計情報を表示します。

**使用例：**
```bash
# 全統計情報を表示
show port statistics
```

### show_port_statistics_pps - PPS統計表示
```
show port statistics pps
```

パケット/秒の統計を表示します。

**使用例：**
```bash
# PPS統計のみ表示
show port statistics pps
```

### show_port_statistics_total - 総パケット統計表示
```
show port statistics total
```

総パケット数の統計を表示します。

**使用例：**
```bash
# 総パケット数を表示
show port statistics total
```

### show_port_statistics_bps - BPS統計表示
```
show port statistics bps
```

ビット/秒の統計を表示します。

**使用例：**
```bash
# ビット/秒を表示
show port statistics bps
```

### show_port_statistics_Bps - バイト/秒統計表示
```
show port statistics Bps
```

バイト/秒の統計を表示します。

**使用例：**
```bash
# バイト/秒を表示
show port statistics Bps
```

### show_port_statistics_total_bytes - 総バイト数統計表示
```
show port statistics total-bytes
```

総バイト数の統計を表示します。

**使用例：**
```bash
# 総バイト数を表示
show port statistics total-bytes
```

### show_port_promiscuous - プロミスキャスモード表示
```
show port (<0-16>|all) promiscuous
```

指定されたポートのプロミスキャスモードの状態を表示します。

**使用例：**
```bash
# ポート0のプロミスキャスモードを表示
show port 0 promiscuous

# 全ポートのプロミスキャスモードを表示
show port all promiscuous
```

### show_port_flowcontrol - フロー制御設定表示
```
show port (<0-16>|all) flowcontrol
```

指定されたポートのフロー制御設定を表示します。

**使用例：**
```bash
# ポート0のフロー制御設定を表示
show port 0 flowcontrol

# 全ポートのフロー制御設定を表示
show port all flowcontrol
```

### set_port_promiscuous - プロミスキャスモード設定
```
set port (<0-16>|all) promiscuous (enable|disable)
```

指定されたポートのプロミスキャスモードを有効または無効にします。

**使用例：**
```bash
# ポート0のプロミスキャスモードを有効化
set port 0 promiscuous enable

# 全ポートのプロミスキャスモードを無効化
set port all promiscuous disable
```

### set_port_flowcontrol - フロー制御設定
```
set port (<0-16>|all) flowcontrol (rx|tx|autoneg|send-xon|fwd-mac-ctrl) (on|off)
```

指定されたポートのフロー制御設定を変更します。

**オプション：**
- `rx` - 受信フロー制御
- `tx` - 送信フロー制御
- `autoneg` - 自動ネゴシエーション
- `send-xon` - XON送信
- `fwd-mac-ctrl` - MAC制御フレーム転送

**使用例：**
```bash
# ポート0の受信フロー制御を有効化
set port 0 flowcontrol rx on

# 全ポートの自動ネゴシエーションを無効化
set port all flowcontrol autoneg off
```

### set_port_dev_configure - デバイス設定
```
set port (<0-16>|all) dev-configure <0-64> <0-64>
```

DPDKポートのデバイス設定を行います。

**パラメータ：**
- 第1引数: 受信キュー数 (0-64)
- 第2引数: 送信キュー数 (0-64)

**使用例：**
```bash
# ポート0を受信キュー4、送信キュー4で設定
set port 0 dev-configure 4 4

# 全ポートを受信キュー1、送信キュー1で設定
set port all dev-configure 1 1
```

### set_port_nrxdesc - 受信ディスクリプタ数設定
```
set port (<0-16>|all) nrxdesc <0-16384>
```

指定したポートの受信ディスクリプタ数を設定します。

**使用例：**
```bash
# ポート0の受信ディスクリプタ数を1024に設定
set port 0 nrxdesc 1024

# 全ポートの受信ディスクリプタ数を512に設定
set port all nrxdesc 512
```

### set_port_ntxdesc - 送信ディスクリプタ数設定
```
set port (<0-16>|all) ntxdesc <0-16384>
```

指定したポートの送信ディスクリプタ数を設定します。

**使用例：**
```bash
# ポート0の送信ディスクリプタ数を1024に設定
set port 0 ntxdesc 1024

# 全ポートの送信ディスクリプタ数を512に設定
set port all ntxdesc 512
```

### set_port_link_updown - リンクアップ・ダウン設定
```
set port (<0-16>|all) link (up|down)
```

指定されたポートのリンクを強制的にアップまたはダウンします。

**使用例：**
```bash
# ポート0のリンクをアップ
set port 0 link up

# 全ポートのリンクをダウン
set port all link down
```

## 定義場所

これらのコマンドは以下のファイルで定義されています：
- `sdplane/dpdk_port_cmd.c`

## 関連項目

- [ワーカー・lcore管理](worker-management.md)
- [システム情報・監視](system-monitoring.md)
- [キュー設定](queue-configuration.md)