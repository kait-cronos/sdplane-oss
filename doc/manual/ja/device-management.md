# デバイス管理

**言語 / Language:** [English](../en/device-management.md) | **日本語** | [Français](../fr/device-management.md) | [中文](../zh/device-management.md) | [Deutsch](../de/device-management.md) | [Italiano](../it/device-management.md) | [한국어](../ko/device-management.md) | [ไทย](../th/device-management.md) | [Español](../es/device-management.md)

DPDKデバイスとドライバーの管理を行うコマンドです。

## コマンド一覧

### show_devices - デバイス情報表示
```
show devices
```

システムで利用可能なデバイスの情報を表示します。

**使用例：**
```bash
show devices
```

このコマンドは以下の情報を表示します：
- デバイス名
- デバイスタイプ
- 現在のドライバー
- デバイスの状態
- PCIアドレス

### set_device_driver - デバイスドライバー設定
```
set device <WORD> driver (ixgbe|igb|igc|uio_pci_generic|igb_uio|vfio-pci|unbound) (|bind|driver_override)
```

指定されたデバイスにドライバーを設定します。

**パラメータ：**
- `<WORD>` - デバイス名またはPCIアドレス
- ドライバー種類：
  - `ixgbe` - Intel 10GbE ixgbeドライバー
  - `igb` - Intel 1GbE igbドライバー
  - `igc` - Intel 2.5GbE igcドライバー
  - `uio_pci_generic` - 汎用UIOドライバー
  - `igb_uio` - DPDK UIOドライバー
  - `vfio-pci` - VFIO PCIドライバー
  - `unbound` - ドライバーを削除
- 操作モード：
  - `bind` - ドライバーをバインド
  - `driver_override` - ドライバーオーバーライド

**使用例：**
```bash
# デバイスをvfio-pciドライバーにバインド
set device 0000:01:00.0 driver vfio-pci bind

# デバイスをigb_uioドライバーに設定
set device eth0 driver igb_uio

# デバイスからドライバーを削除
set device 0000:01:00.0 driver unbound
```

## デバイス管理の概要

### DPDKデバイス管理
DPDKでは、ネットワークデバイスを効率的に使用するために専用のドライバーを使用します。

### ドライバーの種類

#### ネットワークドライバー
- **ixgbe** - Intel 10GbE ネットワークカード用
- **igb** - Intel 1GbE ネットワークカード用
- **igc** - Intel 2.5GbE ネットワークカード用

#### UIO（Userspace I/O）ドライバー
- **uio_pci_generic** - 汎用的なUIOドライバー
- **igb_uio** - DPDK専用UIOドライバー

#### VFIO（Virtual Function I/O）ドライバー
- **vfio-pci** - 仮想化環境での高性能I/O

#### 特殊設定
- **unbound** - ドライバーを削除してデバイスを無効化

## デバイス設定の手順

### 基本的な設定手順
1. **デバイス確認**
```bash
show devices
```

2. **ドライバー設定**
```bash
set device <device> driver <driver> bind
```

3. **設定確認**
```bash
show devices
```

4. **ポート設定**
```bash
show port
```

### 設定例

#### Intel 10GbE カードの設定
```bash
# デバイス確認
show devices

# ixgbeドライバーをバインド
set device 0000:01:00.0 driver ixgbe bind
set device 0000:01:00.1 driver ixgbe bind

# 設定確認
show devices
show port
```

#### DPDK UIOドライバーの設定
```bash
# デバイス確認
show devices

# igb_uioドライバーをバインド
set device 0000:02:00.0 driver igb_uio bind
set device 0000:02:00.1 driver igb_uio bind

# 設定確認
show devices
show port
```

#### VFIO設定（仮想化環境）
```bash
# デバイス確認
show devices

# vfio-pciドライバーをバインド
set device 0000:03:00.0 driver vfio-pci bind
set device 0000:03:00.1 driver vfio-pci bind

# 設定確認
show devices
show port
```

## ドライバーの選択指針

### ixgbe（Intel 10GbE）
- **用途**: Intel 10GbE ネットワークカード
- **利点**: 高性能、安定性
- **条件**: Intel 10GbE カードが必要

### igb（Intel 1GbE）
- **用途**: Intel 1GbE ネットワークカード
- **利点**: 広い互換性、安定性
- **条件**: Intel 1GbE カードが必要

### igc（Intel 2.5GbE）
- **用途**: Intel 2.5GbE ネットワークカード
- **利点**: 中間的な性能、新しい規格
- **条件**: Intel 2.5GbE カードが必要

### uio_pci_generic
- **用途**: 汎用的なデバイス
- **利点**: 広い互換性
- **欠点**: 一部の機能制限

### igb_uio
- **用途**: DPDK専用環境
- **利点**: DPDK最適化
- **欠点**: 別途インストールが必要

### vfio-pci
- **用途**: 仮想化環境、セキュリティ重視
- **利点**: セキュリティ、仮想化サポート
- **条件**: IOMMU有効化が必要

## トラブルシューティング

### デバイスが認識されない場合
1. デバイス確認
```bash
show devices
```

2. システムレベルでの確認
```bash
lspci | grep Ethernet
```

3. カーネルモジュールの確認
```bash
lsmod | grep uio
lsmod | grep vfio
```

### ドライバーバインドに失敗する場合
1. 現在のドライバーを確認
```bash
show devices
```

2. 既存のドライバーを削除
```bash
set device <device> driver unbound
```

3. 目的のドライバーをバインド
```bash
set device <device> driver <target_driver> bind
```

### ポートが利用できない場合
1. デバイス状態確認
```bash
show devices
show port
```

2. ドライバーの再バインド
```bash
set device <device> driver unbound
set device <device> driver <driver> bind
```

3. ポート設定の確認
```bash
show port
update port status
```

## 高度な機能

### ドライバーオーバーライド
```bash
# ドライバーオーバーライドの使用
set device <device> driver <driver> driver_override
```

### 複数デバイスの一括設定
```bash
# 複数デバイスを順次設定
set device 0000:01:00.0 driver vfio-pci bind
set device 0000:01:00.1 driver vfio-pci bind
set device 0000:02:00.0 driver vfio-pci bind
set device 0000:02:00.1 driver vfio-pci bind
```

## セキュリティ考慮事項

### VFIO使用時の注意点
- IOMMU有効化が必要
- セキュリティグループの設定
- 権限の適切な設定

### UIO使用時の注意点
- root権限が必要
- セキュリティリスクの理解
- 適切なアクセス制御

## システム統合

### systemdサービスとの連携
```bash
# systemdサービスでの自動設定
# /etc/systemd/system/sdplane.service に設定
```

### 起動時の自動設定
```bash
# 起動スクリプトでの設定
# /etc/init.d/sdplane または systemd unit file
```

## 定義場所

これらのコマンドは以下のファイルで定義されています：
- `sdplane/dpdk_devbind.c`

## 関連項目

- [ポート管理・統計](port-management.md)
- [ワーカー・lcore管理](worker-lcore-thread-management.md)
- [システム情報・監視](system-monitoring.md)