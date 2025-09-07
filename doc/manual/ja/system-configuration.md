# システム設定

- **ヒュージページ**：DPDK用システムヒュージページの設定
- **ネットワーク**：ネットワークインターフェース設定にnetplanを使用
- **ファイアウォール**： CLIのために telnet 9882/tcp portが必要 

**⚠️ CLIに認証がありません。localhostからのみ接続を許可することを推奨 ⚠️**

## ヒュージページの設定
```bash
# GRUB設定の編集
sudo vi /etc/default/grub

# 以下のいずれかの行を追加:
# 2MBヒュージページの場合 (1536ページ = 約3GB):
GRUB_CMDLINE_LINUX="hugepages=1536"

# または1GBヒュージページの場合 (8ページ = 8GB):
# (8GB未満のRAMの場合、hugepages=4などに調整してください.)
GRUB_CMDLINE_LINUX="default_hugepagesz=1G hugepagesz=1G hugepages=8"

# GRUBを更新して再起動
sudo update-grub
sudo reboot
```

## DPDK IGBカーネルモジュールのインストール（オプション）

vfio-pciで動作しないNICの場合は、オプションでigb_uioをインストールしてください。

```bash
git clone http://dpdk.org/git/dpdk-kmods
cd dpdk-kmods/linux/igb_uio
make
sudo mkdir -p /lib/modules/`uname -r`/extra/dpdk/
sudo cp igb_uio.ko /lib/modules/`uname -r`/extra/dpdk/
echo igb_uio | sudo tee /etc/modules-load.d/igb_uio.conf
```

## DPDK UIOドライバーセットアップ

DPDKがユーザー空間アプリケーションからネットワークインターフェースにアクセスするためには、ユーザー空間I/O（UIO）ドライバーが必要です。

### 標準NIC

ほとんどの標準的なネットワークインターフェースカードでは、3つのUIOドライバーオプションがあります：

1. **vfio-pci**（推奨）
   - 最も安全で現代的なオプション
   - IOMMU サポート（Intel VT-d または AMD-Vi）が必要
   - 追加のカーネルモジュールコンパイルは不要

2. **uio_pci_generic**
   - Linuxカーネルに含まれる汎用UIOドライバー
   - 追加インストール不要
   - vfio-pciと比べて機能が限定的

3. **igb_uio**
   - DPDK専用UIOドライバー
   - 手動コンパイルとインストールが必要（上記参照）
   - 古いハードウェア用の追加機能を提供

### 二股ドライバーNIC

**Mellanox ConnectXシリーズ**などの一部のNICでは、同じ物理ポートをカーネルネットワークとDPDKアプリケーションの両方で同時に使用できる二股ドライバーが提供されています。これらのNICでは：

- UIOドライバー設定は不要
- NICはカーネルドライバーにバインドされたまま使用可能
- DPDKアプリケーションが直接ハードウェアにアクセス可能

### UIOドライバー設定

UIOドライバーを設定するには、以下が必要です：

1. **ドライバーモジュールの読み込み**：
```bash
# vfio-pci用（BIOSでIOMMUを有効にする必要があります）
sudo modprobe vfio-pci

# uio_pci_generic用
sudo modprobe uio_pci_generic

# igb_uio用（インストール後）
sudo modprobe igb_uio
```

2. **起動時のドライバー読み込み**のため、`/etc/modules-load.d/`に設定ファイルを作成：
```bash
# vfio-pciの例
echo "vfio-pci" | sudo tee /etc/modules-load.d/vfio-pci.conf

# uio_pci_genericの例
echo "uio_pci_generic" | sudo tee /etc/modules-load.d/uio_pci_generic.conf

# igb_uioの例
echo "igb_uio" | sudo tee /etc/modules-load.d/igb_uio.conf
```

3. **NICをUIOドライバーにバインド**するためDPDKの`dpdk-devbind.py`スクリプトを使用：
```bash
# カーネルドライバーからアンバインドし、UIOドライバーにバインド
sudo dpdk-devbind.py --bind=vfio-pci 0000:01:00.0
sudo dpdk-devbind.py --bind=uio_pci_generic 0000:01:00.1  
sudo dpdk-devbind.py --bind=igb_uio 0000:01:00.2
```

**注意**: `0000:01:00.0`を実際のPCIデバイスアドレスに置き換えてください。`lspci`や`dpdk-devbind.py --status`を使用してNICを識別できます。

**代替方法**: `dpdk-devbind.py`を手動で使用する代わりに、`sdplane.conf`ファイルの最初に`set device XX:XX.X driver ...`コマンドを使用してデバイスバインディングを設定できます。これにより、sdplaneが起動時にデバイスバインディングを自動的に処理します。