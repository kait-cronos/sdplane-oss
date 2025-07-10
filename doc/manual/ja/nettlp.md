# NetTLP

**言語 / Language:** [English](../nettlp.md) | **日本語**

Network TLP（Transaction Layer Protocol）機能を管理するコマンドです。

## コマンド一覧

### nettlp_send_dma_write_read - NetTLP DMA送信
```
nettlp-send (dma-write|dma-read)
```

NetTLPを使用してDMA（Direct Memory Access）の書き込みまたは読み込みを実行します。

**コマンド：**
- `dma-write` - DMA書き込み
- `dma-read` - DMA読み込み

**使用例：**
```bash
# DMA書き込みを実行
nettlp-send dma-write

# DMA読み込みを実行
nettlp-send dma-read
```

### show_nettlp - NetTLP情報表示
```
show nettlp
```

NetTLPの現在の設定と状態を表示します。

**使用例：**
```bash
show nettlp
```

### set_nettlp_ether_local_remote - NetTLPイーサネットアドレス設定
```
set nettlp ether (local-addr|remote-addr) <WORD>
```

NetTLPで使用するローカルまたはリモートのイーサネットアドレスを設定します。

**パラメータ：**
- `local-addr` - ローカルイーサネットアドレス
- `remote-addr` - リモートイーサネットアドレス
- `<WORD>` - MACアドレス（例：00:11:22:33:44:55）

**使用例：**
```bash
# ローカルイーサネットアドレスを設定
set nettlp ether local-addr 00:11:22:33:44:55

# リモートイーサネットアドレスを設定
set nettlp ether remote-addr 00:aa:bb:cc:dd:ee
```

### set_nettlp_ipv4_local_remote - NetTLP IPv4アドレス設定
```
set nettlp ipv4 (local-addr|remote-addr) A.B.C.D
```

NetTLPで使用するローカルまたはリモートのIPv4アドレスを設定します。

**パラメータ：**
- `local-addr` - ローカルIPアドレス
- `remote-addr` - リモートIPアドレス
- `A.B.C.D` - IPv4アドレス

**使用例：**
```bash
# ローカルIPアドレスを設定
set nettlp ipv4 local-addr 192.168.1.10

# リモートIPアドレスを設定
set nettlp ipv4 remote-addr 192.168.1.20
```

### set_nettlp_bus_number - NetTLPバス番号設定
```
set nettlp bus-number <0-65535> device-number <0-65535>
```

NetTLPで使用するPCIバス番号とデバイス番号を設定します。

**パラメータ：**
- `<0-65535>` (bus-number) - PCIバス番号
- `<0-65535>` (device-number) - PCIデバイス番号

**使用例：**
```bash
# バス番号1、デバイス番号0を設定
set nettlp bus-number 1 device-number 0

# バス番号2、デバイス番号1を設定
set nettlp bus-number 2 device-number 1
```

### set_nettlp_pci_tag - NetTLP PCIタグ設定
```
set nettlp pci-tag <0-255>
```

NetTLPで使用するPCIタグを設定します。

**パラメータ：**
- `<0-255>` - PCIタグ値

**使用例：**
```bash
# PCIタグを10に設定
set nettlp pci-tag 10

# PCIタグを255に設定
set nettlp pci-tag 255
```

### set_nettlp_txportid - NetTLP送信ポートID設定
```
set nettlp tx-portid <0-128>
```

NetTLPで使用する送信ポートIDを設定します。

**パラメータ：**
- `<0-128>` - 送信ポートID

**使用例：**
```bash
# 送信ポートIDを0に設定
set nettlp tx-portid 0

# 送信ポートIDを1に設定
set nettlp tx-portid 1
```

### set_nettlp_udp_port - NetTLP UDPポート設定
```
set nettlp udp (src-port|dst-port) <0-65535>
```

NetTLPで使用するUDPの送信元ポートまたは宛先ポートを設定します。

**パラメータ：**
- `src-port` - 送信元ポート
- `dst-port` - 宛先ポート
- `<0-65535>` - ポート番号

**使用例：**
```bash
# 送信元ポートを16384に設定
set nettlp udp src-port 16384

# 宛先ポートを16385に設定
set nettlp udp dst-port 16385
```

### set_nettlp_memory_addr - NetTLPメモリアドレス設定
```
set nettlp memory-address <WORD>
```

NetTLPで使用するメモリアドレスを設定します。

**パラメータ：**
- `<WORD>` - メモリアドレス（16進数）

**使用例：**
```bash
# メモリアドレスを設定
set nettlp memory-address 0x1000000

# 別のメモリアドレスを設定
set nettlp memory-address 0x2000000
```

### set_nettlp_payload_size - NetTLPペイロードサイズ設定
```
set nettlp payload-size <0-4096>
```

NetTLPで使用するペイロードサイズを設定します。

**パラメータ：**
- `<0-4096>` - ペイロードサイズ（バイト）

**使用例：**
```bash
# ペイロードサイズを1024バイトに設定
set nettlp payload-size 1024

# ペイロードサイズを64バイトに設定
set nettlp payload-size 64
```

### set_nettlp_max_payload_size - NetTLP最大ペイロードサイズ設定
```
set nettlp max-payload-size <0-4096>
```

NetTLPで使用する最大ペイロードサイズを設定します。

**パラメータ：**
- `<0-4096>` - 最大ペイロードサイズ（バイト）

**使用例：**
```bash
# 最大ペイロードサイズを2048バイトに設定
set nettlp max-payload-size 2048

# 最大ペイロードサイズを4096バイトに設定
set nettlp max-payload-size 4096
```

### set_nettlp_payload_string - NetTLPペイロード文字列設定
```
set nettlp payload-string <LINE>
```

NetTLPで使用するペイロードの文字列を設定します。

**パラメータ：**
- `<LINE>` - ペイロード文字列

**使用例：**
```bash
# ペイロード文字列を設定
set nettlp payload-string "Hello NetTLP"

# 別のペイロード文字列を設定
set nettlp payload-string "Test Data 123"
```

### set_nettlp_psmem_addr - NetTLP疑似メモリアドレス設定
```
set nettlp psmem-address <WORD>
```

NetTLPで使用する疑似メモリアドレスを設定します。

**パラメータ：**
- `<WORD>` - 疑似メモリアドレス（16進数）

**使用例：**
```bash
# 疑似メモリアドレスを設定
set nettlp psmem-address 0x10000000

# 別の疑似メモリアドレスを設定
set nettlp psmem-address 0x20000000
```

### set_nettlp_psmem_size - NetTLP疑似メモリサイズ設定
```
set nettlp psmem-size (<WORD>|256M)
```

NetTLPで使用する疑似メモリサイズを設定します。

**パラメータ：**
- `<WORD>` - メモリサイズ（数値）
- `256M` - プリセット値（256MB）

**使用例：**
```bash
# 疑似メモリサイズを256MBに設定
set nettlp psmem-size 256M

# 疑似メモリサイズを1GBに設定
set nettlp psmem-size 1024M
```

### show_nettlp_psmem - NetTLP疑似メモリ表示
```
show nettlp psmem (|<WORD>)
```

NetTLPの疑似メモリ内容を表示します。

**パラメータ：**
- `<WORD>` - 表示するアドレス（省略時は全体）

**使用例：**
```bash
# 疑似メモリ全体を表示
show nettlp psmem

# 特定のアドレスを表示
show nettlp psmem 0x1000
```

## NetTLPの概要

### NetTLPとは
NetTLP（Network Transaction Layer Protocol）は、PCIe TLP（Transaction Layer Protocol）をネットワーク経由で転送する技術です。

### 主な機能
- **リモートメモリアクセス** - ネットワーク越しのメモリアクセス
- **DMA転送** - Direct Memory Access転送
- **PCIeエミュレーション** - PCIe機能のネットワーク越し実現
- **高速データ転送** - 低レイテンシでの高速転送

### 用途
- **分散システム** - システム間の高速通信
- **仮想化環境** - 仮想PCIeデバイスの実現
- **テスト環境** - PCIeデバイスのエミュレーション
- **研究開発** - 新しいネットワークアーキテクチャの研究

## NetTLPの設定

### 基本設定手順
1. **ネットワーク設定**
```bash
# イーサネットアドレス設定
set nettlp ether local-addr 00:11:22:33:44:55
set nettlp ether remote-addr 00:aa:bb:cc:dd:ee

# IPアドレス設定
set nettlp ipv4 local-addr 192.168.1.10
set nettlp ipv4 remote-addr 192.168.1.20

# UDPポート設定
set nettlp udp src-port 16384
set nettlp udp dst-port 16385
```

2. **PCIe設定**
```bash
# バス・デバイス番号設定
set nettlp bus-number 1 device-number 0

# PCIタグ設定
set nettlp pci-tag 10

# 送信ポートID設定
set nettlp tx-portid 0
```

3. **メモリ設定**
```bash
# メモリアドレス設定
set nettlp memory-address 0x1000000

# ペイロードサイズ設定
set nettlp payload-size 1024
set nettlp max-payload-size 2048

# 疑似メモリ設定
set nettlp psmem-address 0x10000000
set nettlp psmem-size 256M
```

4. **ワーカー設定**
```bash
# NetTLPワーカーを設定
set worker lcore 3 nettlp-thread
start worker lcore 3
```

### 設定例

#### 基本的なNetTLP設定
```bash
# ネットワーク設定
set nettlp ether local-addr 00:11:22:33:44:55
set nettlp ether remote-addr 00:aa:bb:cc:dd:ee
set nettlp ipv4 local-addr 192.168.1.10
set nettlp ipv4 remote-addr 192.168.1.20
set nettlp udp src-port 16384
set nettlp udp dst-port 16385

# PCIe設定
set nettlp bus-number 1 device-number 0
set nettlp pci-tag 10
set nettlp tx-portid 0

# メモリ設定
set nettlp memory-address 0x1000000
set nettlp payload-size 1024
set nettlp max-payload-size 2048

# 疑似メモリ設定
set nettlp psmem-address 0x10000000
set nettlp psmem-size 256M

# ワーカー設定
set worker lcore 3 nettlp-thread
start worker lcore 3

# 設定確認
show nettlp
```

## 使用例

### DMA転送の実行
```bash
# DMA書き込み
nettlp-send dma-write

# DMA読み込み
nettlp-send dma-read

# 結果確認
show nettlp
```

### 疑似メモリの操作
```bash
# 疑似メモリ内容の確認
show nettlp psmem

# 特定アドレスの確認
show nettlp psmem 0x1000

# ペイロード文字列の設定
set nettlp payload-string "Test Data"

# DMA書き込みでデータ転送
nettlp-send dma-write
```

## 監視と管理

### NetTLP状態の確認
```bash
# NetTLP設定と状態の表示
show nettlp

# 疑似メモリの表示
show nettlp psmem

# ワーカーの状態確認
show worker
```

### パフォーマンス監視
```bash
# スレッドカウンター確認
show thread counter

# ポート統計確認
show port statistics
```

## トラブルシューティング

### NetTLPが動作しない場合
1. 設定確認
```bash
show nettlp
```

2. ワーカー状態確認
```bash
show worker
```

3. ネットワーク接続確認
```bash
show port
```

### DMA転送が失敗する場合
1. メモリアドレス設定確認
2. ペイロードサイズ確認
3. ネットワーク設定確認

### 疑似メモリにアクセスできない場合
1. アドレス設定確認
2. メモリサイズ確認
3. 権限設定確認

## 定義場所

これらのコマンドは以下のファイルで定義されています：
- `sdplane/nettlp.c`

## 関連項目

- [ワーカー・lcore管理](worker-management.md)
- [ポート管理・統計](port-management.md)
- [システム情報・監視](system-monitoring.md)