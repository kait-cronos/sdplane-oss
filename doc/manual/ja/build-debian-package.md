# Debianパッケージのビルド・インストール

**Language:** [English](../en/build-debian-package.md) | **日本語** | [Français](../fr/build-debian-package.md) | [中文](../zh/build-debian-package.md) | [Deutsch](../de/build-debian-package.md) | [Italiano](../it/build-debian-package.md) | [한국어](../ko/build-debian-package.md) | [ไทย](../th/build-debian-package.md) | [Español](../es/build-debian-package.md)

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