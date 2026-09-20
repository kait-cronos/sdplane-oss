# マルチキャスト転送テスト（物理環境）

sdplane をスイッチとして経由したマルチキャストUDPの転送を、確認するスクリプトです。
`.33` 開発機上で実行します。

```
traffic gen（.21）
    │ マルチキャストUDP送信
    ▼
開発機 .33：sdplane
    ├─ port0 → 開発機 .34：rx1（IGMP JOIN）
    └─ port1 → 開発機 .35：rx2（非JOIN）
```

## 実行手順

### 1. SSH の準備

スクリプトは `.33` から `.34` / `.35` / `.21` へ SSH します。
事前に `ssh-add` したうえで、`ssh -A`で `.33` に入ってください。

### 2. sdplane のビルド

SSHの準備をした後は、`.33`上の自身のユーザ環境でsdplaneをビルドしてください。

```bash
mkdir -p build && cd build
../configure
make -j4
```

### 3. テストの実行

ビルド完了後、テストスクリプトを実行してください。

```bash
./tests/mcast/test.sh
```

