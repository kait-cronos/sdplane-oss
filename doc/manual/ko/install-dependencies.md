# 의존성 설치

**Language:** [English](../en/install-dependencies.md) | [Japanese](../ja/install-dependencies.md) | [Français](../fr/install-dependencies.md) | [中文](../zh/install-dependencies.md) | [Deutsch](../de/install-dependencies.md) | [Italiano](../it/install-dependencies.md) | **한국어**

## 의존성

sdplane-oss는 다음 구성 요소가 필요합니다:
- **lthread** (yasuhironet/lthread): 경량 협력적 스레딩
- **liburcu-qsbr**: 사용자 공간 RCU 라이브러리  
- **libpcap**: 패킷 캡처 라이브러리
- **DPDK 23.11.1**: Data Plane Development Kit

## sdplane 의존성 데비안 패키지 설치

```bash
sudo apt update && sudo apt install liburcu-dev libpcap-dev
```

## 빌드 도구 및 DPDK 전제 조건 설치

```bash
sudo apt install build-essential cmake autotools-dev autoconf automake \
                 libtool pkg-config python3 python3-pip meson ninja-build \
                 python3-pyelftools libnuma-dev pkgconf
```

## lthread 설치

```bash
git clone https://github.com/yasuhironet/lthread
cd lthread
cmake .
make
sudo make install
cd ..
```

## DPDK 23.11.1 설치

```bash
# DPDK 23.11.1 다운로드
wget https://fast.dpdk.org/rel/dpdk-23.11.1.tar.xz
tar xf dpdk-23.11.1.tar.xz
cd dpdk-23.11.1

# DPDK 컴파일 및 설치
meson setup -Dprefix=/usr/local build
cd build
ninja install
cd ../..

# 설치 확인
pkg-config --modversion libdpdk
# 23.11.1이 표시되어야 함
```