# ポート管理・統計

**言語 / Language:** [English](../port-management.md) | **日本語** | [Français](../fr/port-management.md) | [中文](../zh/port-management.md)

DPDKポートの管理と統計情報を扱うコマンドです。

## コマンド一覧

### **start port**

DPDKポートを開始します（デフォルト動作）。

**使用例：**
```bash
# ポートを開始（デフォルト）
start port
```

---

### **start port \<0-16\>**

特定のDPDKポートを開始します。

**使用例：**
```bash
# ポート0を開始
start port 0

# ポート1を開始
start port 1
```

---

### **start port all**

全てのDPDKポートを開始します。

**使用例：**
```bash
# 全ポートを開始
start port all
```

---

### **stop port**

DPDKポートを停止します（デフォルト動作）。

**使用例：**
```bash
# ポートを停止（デフォルト）
stop port
```

---

### **stop port \<0-16\>**

特定のDPDKポートを停止します。

**使用例：**
```bash
# ポート0を停止
stop port 0

# ポート1を停止
stop port 1
```

---

### **stop port all**

全てのDPDKポートを停止します。

**使用例：**
```bash
# 全ポートを停止
stop port all
```

---

### **reset port**

DPDKポートをリセットします（デフォルト動作）。

**使用例：**
```bash
# ポートをリセット（デフォルト）
reset port
```

---

### **reset port \<0-16\>**

特定のDPDKポートをリセットします。

**使用例：**
```bash
# ポート0をリセット
reset port 0

# ポート1をリセット
reset port 1
```

---

### **reset port all**

全てのDPDKポートをリセットします。

**使用例：**
```bash
# 全ポートをリセット
reset port all
```

---

### **show port**

全ポートの基本情報を表示します（デフォルト動作）。

**使用例：**
```bash
# 全ポートの情報を表示
show port
```

---

### **show port \<0-16\>**

特定のポートの基本情報を表示します。

**使用例：**
```bash
# ポート0の情報を表示
show port 0

# ポート1の情報を表示
show port 1
```

---

### **show port all**

全ポートの情報を明示的に表示します。

**使用例：**
```bash
# 全ポートの情報を明示的に表示
show port all
```

---

### **show port statistics**

全てのポート統計情報を表示します。

**使用例：**
```bash
# 全統計情報を表示
show port statistics
```

---

### **show port statistics pps**

パケット/秒の統計を表示します。

**使用例：**
```bash
# PPS統計のみ表示
show port statistics pps
```

---

### **show port statistics total**

総パケット数の統計を表示します。

**使用例：**
```bash
# 総パケット数を表示
show port statistics total
```

---

### **show port statistics bps**

ビット/秒の統計を表示します。

**使用例：**
```bash
# ビット/秒を表示
show port statistics bps
```

---

### **show port statistics Bps**

バイト/秒の統計を表示します。

**使用例：**
```bash
# バイト/秒を表示
show port statistics Bps
```

---

### **show port statistics total-bytes**

総バイト数の統計を表示します。

**使用例：**
```bash
# 総バイト数を表示
show port statistics total-bytes
```

---

### **show port \<0-16\> promiscuous**

指定されたポートのプロミスキャスモードの状態を表示します。

**使用例：**
```bash
# ポート0のプロミスキャスモードを表示
show port 0 promiscuous

# ポート1のプロミスキャスモードを表示
show port 1 promiscuous
```

---

### **show port all promiscuous**

全ポートのプロミスキャスモードの状態を表示します。

**使用例：**
```bash
# 全ポートのプロミスキャスモードを表示
show port all promiscuous
```

---

### **show port \<0-16\> flowcontrol**

指定されたポートのフロー制御設定を表示します。

**使用例：**
```bash
# ポート0のフロー制御設定を表示
show port 0 flowcontrol

# ポート1のフロー制御設定を表示
show port 1 flowcontrol
```

---

### **show port all flowcontrol**

全ポートのフロー制御設定を表示します。

**使用例：**
```bash
# 全ポートのフロー制御設定を表示
show port all flowcontrol
```

---

### **set port \<0-16\> promiscuous enable**

指定されたポートのプロミスキャスモードを有効化します。

**使用例：**
```bash
# ポート0のプロミスキャスモードを有効化
set port 0 promiscuous enable

# ポート1のプロミスキャスモードを有効化
set port 1 promiscuous enable
```

---

### **set port \<0-16\> promiscuous disable**

指定されたポートのプロミスキャスモードを無効化します。

**使用例：**
```bash
# ポート0のプロミスキャスモードを無効化
set port 0 promiscuous disable

# ポート1のプロミスキャスモードを無効化
set port 1 promiscuous disable
```

---

### **set port all promiscuous enable**

全ポートのプロミスキャスモードを有効化します。

**使用例：**
```bash
# 全ポートのプロミスキャスモードを有効化
set port all promiscuous enable
```

---

### **set port all promiscuous disable**

全ポートのプロミスキャスモードを無効化します。

**使用例：**
```bash
# 全ポートのプロミスキャスモードを無効化
set port all promiscuous disable
```

---

### **set port \<0-16\> flowcontrol (rx|tx|autoneg|send-xon|fwd-mac-ctrl) (on|off)**

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

# ポート1の自動ネゴシエーションを無効化
set port 1 flowcontrol autoneg off
```

---

### **set port all flowcontrol (rx|tx|autoneg|send-xon|fwd-mac-ctrl) (on|off)**

全ポートのフロー制御設定を変更します。

**オプション：**
- `rx` - 受信フロー制御
- `tx` - 送信フロー制御
- `autoneg` - 自動ネゴシエーション
- `send-xon` - XON送信
- `fwd-mac-ctrl` - MAC制御フレーム転送

**使用例：**
```bash
# 全ポートの自動ネゴシエーションを無効化
set port all flowcontrol autoneg off

# 全ポートの送信フロー制御を有効化
set port all flowcontrol tx on
```

---

### **set port \<0-16\> dev-configure \<0-64\> \<0-64\>**


指定されたポートのDPDKデバイス設定を行います。

**パラメータ：**
- 第1引数: 受信キュー数 (0-64)
- 第2引数: 送信キュー数 (0-64)

**使用例：**
```bash
# ポート0を受信キュー4、送信キュー4で設定
set port 0 dev-configure 4 4

# ポート1を受信キュー2、送信キュー2で設定
set port 1 dev-configure 2 2
```

---

### **set port all dev-configure \<0-64\> \<0-64\>**

全ポートのDPDKデバイス設定を行います。

**パラメータ：**
- 第1引数: 受信キュー数 (0-64)
- 第2引数: 送信キュー数 (0-64)

**使用例：**
```bash
# 全ポートを受信キュー1、送信キュー1で設定
set port all dev-configure 1 1
```

---

### **set port \<0-16\> nrxdesc \<0-16384\>**


指定したポートの受信ディスクリプタ数を設定します。

**使用例：**
```bash
# ポート0の受信ディスクリプタ数を1024に設定
set port 0 nrxdesc 1024

# ポート1の受信ディスクリプタ数を512に設定
set port 1 nrxdesc 512
```

---

### **set port all nrxdesc \<0-16384\>**

全ポートの受信ディスクリプタ数を設定します。

**使用例：**
```bash
# 全ポートの受信ディスクリプタ数を512に設定
set port all nrxdesc 512
```

---

### **set port \<0-16\> ntxdesc \<0-16384\>**


指定したポートの送信ディスクリプタ数を設定します。

**使用例：**
```bash
# ポート0の送信ディスクリプタ数を1024に設定
set port 0 ntxdesc 1024

# ポート1の送信ディスクリプタ数を512に設定
set port 1 ntxdesc 512
```

---

### **set port all ntxdesc \<0-16384\>**

全ポートの送信ディスクリプタ数を設定します。

**使用例：**
```bash
# 全ポートの送信ディスクリプタ数を512に設定
set port all ntxdesc 512
```

---

### **set port \<0-16\> link up**


指定されたポートのリンクをアップします。

**使用例：**
```bash
# ポート0のリンクをアップ
set port 0 link up

# ポート1のリンクをアップ
set port 1 link up
```

---

### **set port \<0-16\> link down**

指定されたポートのリンクをダウンします。

**使用例：**
```bash
# ポート0のリンクをダウン
set port 0 link down

# ポート1のリンクをダウン
set port 1 link down
```

---

### **set port all link up**

全ポートのリンクをアップします。

**使用例：**
```bash
# 全ポートのリンクをアップ
set port all link up
```

---

### **set port all link down**

全ポートのリンクをダウンします。

**使用例：**
```bash
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