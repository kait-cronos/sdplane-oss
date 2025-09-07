# 데비안 패키지 빌드 및 설치

**Language:** [English](../en/build-debian-package.md) | [Japanese](../ja/build-debian-package.md) | [Français](../fr/build-debian-package.md) | [中文](../zh/build-debian-package.md) | [Deutsch](../de/build-debian-package.md) | [Italiano](../it/build-debian-package.md) | **한국어**

## 전제 조건 패키지 설치
```bash
sudo apt install build-essential cmake devscripts debhelper
```

## sdplane-oss 데비안 패키지 빌드
```bash
# 먼저 깨끗한 공간에서 시작하는지 확인
(cd build && make distclean)
make distclean

# 소스에서 데비안 패키지 빌드
bash build-debian.sh

# 생성된 패키지 설치 (상위 디렉토리에 생성됨)
sudo apt install ../sdplane_*.deb
```