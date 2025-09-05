# 拡張リピーターの設定

拡張リピーターは、L3ルーティング用とパケットキャプチャ用のTAPインターフェースを備えた高度なVLANスイッチング機能を提供します。仮想スイッチの作成、VLANタギングを使ったDPDKポートリンク、カーネル統合用のルーターインターフェース、監視用のキャプチャインターフェースをサポートします。

## 仮想スイッチコマンド

### set vswitch
```
set vswitch <1-4094>
```
**説明**: 指定されたVLAN IDで仮想スイッチを作成します

**パラメータ**:
- `<1-4094>`: 仮想スイッチのVLAN ID

**例**:
```
set vswitch 2031
set vswitch 2032
```

### delete vswitch
```
delete vswitch <0-3>
```
**説明**: IDで指定した仮想スイッチを削除します

**パラメータ**:
- `<0-3>`: 削除する仮想スイッチID

**例**:
```
delete vswitch 0
```

### show vswitch_rib
```
show vswitch_rib
```
**説明**: 設定と状態を含む仮想スイッチRIB情報を表示します

## 仮想スイッチリンクコマンド

### set vswitch-link
```
set vswitch-link vswitch <0-3> port <0-7> tag <0-4094>
```
**説明**: DPDKポートを仮想スイッチにVLANタギング設定でリンクします

**パラメータ**:
- `vswitch <0-3>`: 仮想スイッチID (0-3)
- `port <0-7>`: DPDKポートID (0-7)  
- `tag <0-4094>`: VLANタグID (0: ネイティブ/アンタグ、1-4094: タグ付きVLAN)

**例**:
```
# ポート0を仮想スイッチ0にVLANタグ2031でリンク
set vswitch-link vswitch 0 port 0 tag 2031

# ポート0を仮想スイッチ1にネイティブ/アンタグでリンク
set vswitch-link vswitch 1 port 0 tag 0
```

### delete vswitch-link
```
delete vswitch-link <0-7>
```
**説明**: IDで指定した仮想スイッチリンクを削除します

**パラメータ**:
- `<0-7>`: 仮想スイッチリンクID

### show vswitch-link
```
show vswitch-link
```
**説明**: すべての仮想スイッチリンク設定を表示します

## ルーターインターフェースコマンド

### set vswitch router-if
```
set vswitch <1-4094> router-if <WORD>
```
**説明**: 指定された仮想スイッチにL3接続用のルーターインターフェースを作成します

**パラメータ**:
- `<1-4094>`: 仮想スイッチのVLAN ID
- `<WORD>`: TAPインターフェース名

**例**:
```
set vswitch 2031 router-if rif2031
set vswitch 2032 router-if rif2032
```

### no set vswitch router-if
```
no set vswitch <1-4094> router-if
```
**説明**: 指定された仮想スイッチからルーターインターフェースを削除します

**パラメータ**:
- `<1-4094>`: 仮想スイッチのVLAN ID

### show rib vswitch router-if
```
show rib vswitch router-if
```
**説明**: MACアドレス、IPアドレス、インターフェース状態を含むルーターインターフェース設定を表示します

## キャプチャインターフェースコマンド

### set vswitch capture-if
```
set vswitch <1-4094> capture-if <WORD>
```
**説明**: 指定された仮想スイッチにパケット監視用のキャプチャインターフェースを作成します

**パラメータ**:
- `<1-4094>`: 仮想スイッチのVLAN ID
- `<WORD>`: TAPインターフェース名

**例**:
```
set vswitch 2031 capture-if cif2031
set vswitch 2032 capture-if cif2032
```

### no set vswitch capture-if
```
no set vswitch <1-4094> capture-if
```
**説明**: 指定された仮想スイッチからキャプチャインターフェースを削除します

**パラメータ**:
- `<1-4094>`: 仮想スイッチのVLAN ID

### show rib vswitch capture-if
```
show rib vswitch capture-if
```
**説明**: キャプチャインターフェース設定を表示します

## VLAN処理機能

拡張リピーターは高度なVLAN処理を実行します：

- **VLAN変換**: vswitch-link設定に基づいてVLAN IDを変更
- **VLAN除去**: タグが0（ネイティブ）に設定されている場合、VLANヘッダーを除去  
- **VLAN挿入**: アンタグパケットをタグ付きポートに転送する際にVLANヘッダーを追加
- **スプリットホライゾン**: 受信ポートにパケットを送り返さないことでループを防止

## 設定例

```bash
# 仮想スイッチを作成
set vswitch 2031
set vswitch 2032

# DPDKポートをVLANタグ付きで仮想スイッチにリンク
set vswitch-link vswitch 0 port 0 tag 2031
set vswitch-link vswitch 1 port 0 tag 2032

# L3処理用のルーターインターフェースを作成
set vswitch 2031 router-if rif2031
set vswitch 2032 router-if rif2032

# 監視用のキャプチャインターフェースを作成
set vswitch 2031 capture-if cif2031
set vswitch 2032 capture-if cif2032

# 拡張リピーターを使用するようにワーカーを設定
set worker lcore 1 enhanced-repeater
```

## TAPインターフェースとの統合

ルーターインターフェースとキャプチャインターフェースは、Linuxカーネルネットワークスタックと統合されるTAPインターフェースを作成します：

- **ルーターインターフェース**: L3ルーティング、IPアドレシング、カーネルネットワーク処理を可能にします
- **キャプチャインターフェース**: パケット監視、解析、デバッグを有効にします
- **リングバッファ**: データプレーンとカーネル間の効率的なパケット転送にDPDKリングを使用