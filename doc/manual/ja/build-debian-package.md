# Debianパッケージのビルド・インストール

## 前提パッケージのインストール
```bash
sudo apt install build-essential cmake devscripts debhelper
```

## sdplane-oss Debianパッケージのビルド
```bash
# まずクリーンな状態から始める
(cd build && make distclean)
make distclean

# ソースからDebianパッケージをビルド
bash build-debian.sh

# 生成されたパッケージをインストール（親ディレクトリに生成される）
sudo apt install ../sdplane_*.deb
```