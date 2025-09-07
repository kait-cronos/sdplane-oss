# 소스에서 빌드 및 설치

**Language:** [English](../en/build-install-source.md) | [日本語](../ja/build-install-source.md) | [Français](../fr/build-install-source.md) | [中文](../zh/build-install-source.md) | [Deutsch](../de/build-install-source.md) | [Italiano](../it/build-install-source.md) | **한국어** | [ไทย](../th/build-install-source.md) | [Español](../es/build-install-source.md)

**일반적으로 이 절차를 따르시기 바랍니다.**

## Ubuntu 전제 조건 패키지 설치

### 소스에서 빌드용
```bash
# 필수 빌드 도구
sudo apt install build-essential cmake autotools-dev autoconf automake libtool pkg-config

# DPDK 전제 조건
sudo apt install python3 python3-pip meson ninja-build python3-pyelftools libnuma-dev pkgconf
```

### 선택적 패키지
```bash
sudo apt install etckeeper tig bridge-utils \
                 iptables-persistent fail2ban dmidecode screen ripgrep
```

## 소스에서 sdplane-oss 빌드

```bash
# 저장소 복제
git clone https://github.com/kait-cronos/sdplane-oss
cd sdplane-oss

# 빌드 파일 생성
sh autogen.sh

# 구성 및 컴파일
mkdir build
cd build
CFLAGS="-g -O0" sh ../configure
make

# 시스템에 설치
# $prefix에 설치됩니다 (기본값: /usr/local/sbin)
sudo make install
```