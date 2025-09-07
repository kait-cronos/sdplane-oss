# 构建和安装Debian包

**Language:** [English](../en/build-debian-package.md) | [Japanese](../ja/build-debian-package.md) | [Français](../fr/build-debian-package.md) | **中文** | [Deutsch](../de/build-debian-package.md) | [Italiano](../it/build-debian-package.md) | [한국어](../ko/build-debian-package.md) | [ไทย](../th/build-debian-package.md) | [Español](../es/build-debian-package.md)

## 安装前提包
```bash
sudo apt install build-essential cmake devscripts debhelper
```

## 构建sdplane-oss Debian包
```bash
# 首先确保从干净空间开始
(cd build && make distclean)
make distclean

# 从源码构建Debian包
bash build-debian.sh

# 安装生成的包（将在父目录中生成）
sudo apt install ../sdplane_*.deb
```