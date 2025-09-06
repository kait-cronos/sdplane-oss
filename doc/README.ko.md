<div align="center">
<img src="../sdplane-logo.png" alt="sdplane-oss Logo" width="160">
</div>

# sdplane-oss (소프트웨어 데이터 플레인)

DPDK 스레드 작업을 제어할 수 있는 대화형 쉘과 DPDK 스레드 실행 환경(sd-plane)으로 구성된 "DPDK-dock 개발 환경"

**Language:** [English](../README.md) | [日本語](README.ja.md) | [Français](README.fr.md) | [中文](README.zh.md) | [Deutsch](README.de.md) | [Italiano](README.it.md) | **한국어**

## 특징

- **고성능 패킷 처리**:
  DPDK를 사용한 사용자 공간 제로 카피 패킷 처리
- **레이어 2/3 전달**:
  ACL, LPM, FIB 지원을 통한 통합 L2 및 L3 전달
- **패킷 생성**:
  테스트 및 벤치마킹을 위한 내장 패킷 생성기
- **네트워크 가상화**:
  TAP 인터페이스 지원 및 VLAN 스위칭 기능
- **CLI 관리**:
  구성 및 모니터링을 위한 대화형 명령줄 인터페이스
- **멀티스레딩**:
  코어당 워커를 사용한 협력적 스레딩 모델

### 아키텍처
- **메인 애플리케이션**: 중앙 라우터 로직 및 초기화
- **DPDK 모듈**: L2/L3 전달 및 패킷 생성
- **CLI 시스템**: 완성 및 도움말 기능이 있는 명령줄 인터페이스
- **스레딩**: lthread 기반 협력적 멀티태스킹
- **가상화**: TAP 인터페이스 및 가상 스위칭

## 지원 시스템

### 소프트웨어 요구사항
- **OS**:
  Ubuntu 24.04 LTS (현재 지원)
- **NIC**:
  [드라이버](https://doc.dpdk.org/guides/nics/) | [지원 NIC](https://core.dpdk.org/supported/)
- **메모리**:
  Hugepage 지원 필요
- **CPU**:
  멀티 코어 프로세서 권장

### 대상 하드웨어 플랫폼

이 프로젝트는 다음에서 테스트되었습니다:
- **Topton (N305/N100)**: 10G NIC를 갖춘 미니 PC
- **Partaker (J3160)**: 1G NIC를 갖춘 미니 PC
- **Intel 일반 PC**: Intel x520 / Mellanox ConnectX5 사용
- **기타 CPU**: AMD, ARM 프로세서 등에서도 작동해야 함

## 1. 의존성 설치

### 의존성

sdplane-oss는 다음 구성 요소가 필요합니다:
- **lthread** (yasuhironet/lthread): 경량 협력적 스레딩
- **liburcu-qsbr**: 사용자 공간 RCU 라이브러리  
- **libpcap**: 패킷 캡처 라이브러리
- **DPDK 23.11.1**: Data Plane Development Kit

### sdplane 의존성 데비안 패키지 설치

```bash
sudo apt update && sudo apt install liburcu-dev libpcap-dev
```

### 빌드 도구 및 DPDK 전제 조건 설치

```bash
sudo apt install build-essential cmake autotools-dev autoconf automake \
                 libtool pkg-config python3 python3-pip meson ninja-build \
                 python3-pyelftools libnuma-dev pkgconf
```

### lthread 설치

```bash
git clone https://github.com/yasuhironet/lthread
cd lthread
cmake .
make
sudo make install
cd ..
```

### DPDK 23.11.1 설치

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
# 출력: 23.11.1
```

## 2. Intel Core i3-n305/Celeron j3160용 데비안 패키지로 빠른 시작

Intel Core i3-n305/Celeron j3160의 경우 데비안 패키지를 사용하여 빠른 설치가 가능합니다.

미리 컴파일된 데비안 패키지 다운로드 및 설치:

```bash
# n305용 최신 패키지 다운로드
wget https://www.yasuhironet.net/download/n305/sdplane_0.1.4-36_amd64.deb
wget https://www.yasuhironet.net/download/n305/sdplane-dbgsym_0.1.4-36_amd64.ddeb

# 또는 j3160용
wget https://www.yasuhironet.net/download/j3160/sdplane_0.1.4-35_amd64.deb
wget https://www.yasuhironet.net/download/j3160/sdplane-dbgsym_0.1.4-35_amd64.ddeb

# 패키지 설치
sudo apt install ./sdplane_0.1.4-*_amd64.deb
sudo apt install ./sdplane-dbgsym_0.1.4-*_amd64.ddeb
```

**참고**: 최신 패키지 버전은 [yasuhironet.net 다운로드](https://www.yasuhironet.net/download/)에서 확인하세요.

5. 시스템 구성으로 이동하세요.

## 3. 소스에서 빌드

**일반적으로 이 절차를 따르십시오.**

### 필수 Ubuntu 패키지 설치

#### 소스에서 빌드용
```bash
# 핵심 빌드 도구
sudo apt install build-essential cmake autotools-dev autoconf automake libtool pkg-config

# DPDK 전제 조건
sudo apt install python3 python3-pip meson ninja-build python3-pyelftools libnuma-dev pkgconf
```

#### 선택적 패키지
```bash
sudo apt install etckeeper tig bridge-utils \
                 iptables-persistent fail2ban dmidecode screen ripgrep
```

### 소스에서 sdplane-oss 빌드

```bash
# 저장소 클론
git clone https://github.com/kait-cronos/sdplane-oss
cd sdplane-oss

# 빌드 파일 생성
sh autogen.sh

# 구성 및 빌드
mkdir build
cd build
CFLAGS="-g -O0" sh ../configure
make
```

## 4. sdplane-oss 데비안 패키지 생성 및 설치

### 필수 패키지 설치
```bash
sudo apt install build-essential cmake devscripts debhelper
```

### sdplane-oss 데비안 패키지 빌드
```bash
# 먼저 깨끗한 작업 공간에서 시작하는지 확인
(cd build && make distclean)
make distclean

# 소스에서 데비안 패키지 빌드
bash build-debian.sh

# 생성된 패키지 설치 (상위 디렉터리에 생성됨)
sudo apt install ../sdplane_*.deb
```

## 5. 시스템 구성

- **Hugepages**: DPDK용 시스템 hugepage 구성
- **네트워크**: 네트워크 인터페이스 구성에 netplan 사용
- **방화벽**: CLI용 텔넷 9882/tcp 포트 필요

**⚠️ CLI에는 인증이 없습니다. localhost에서만 연결을 허용하는 것이 좋습니다 ⚠️**

### Hugepages 구성
```bash
# GRUB 구성 편집
sudo vi /etc/default/grub

# GRUB_CMDLINE_LINUX 매개변수에 hugepages 추가
# hugepages=1024 추가 예시:
GRUB_CMDLINE_LINUX="hugepages=1024"

# GRUB 업데이트
sudo update-grub

# 시스템 재시작
sudo reboot

# 재시작 후 hugepages 확인
cat /proc/meminfo | grep -E "^HugePages|^Hugepagesize"
```

### 선택적 DPDK IGB 커널 모듈 설치

NIC가 vfio-pci에서 작동하지 않는 경우 igb_uio를 설치하세요.

```bash
git clone http://dpdk.org/git/dpdk-kmods
cd dpdk-kmods/linux/igb_uio
make
sudo make install
cd ../../..

# 모듈이 /lib/modules/$(uname -r)/extra/igb_uio.ko에 설치됨
```

## 6. sdplane 구성

### 구성 파일

sdplane은 구성 파일을 사용하여 시작 동작과 실행 환경을 정의합니다.

#### OS 구성 예시 (`etc/`)
- `systemd/sdplane.service`: systemd 서비스 파일
- `netplan/01-netcfg.yaml`: netplan을 사용한 네트워크 구성

#### 애플리케이션 구성 예시 (`example-config/`)
- 다양한 애플리케이션용 예시 구성 파일
- 시작 스크립트 및 구성 프로필

## 7. sdplane-oss를 사용하여 애플리케이션 실행

```bash
# 기본 실행
sudo ./sdplane/sdplane

# 구성 파일로 실행
sudo ./sdplane/sdplane -f /path/to/config-file

# CLI에 연결 (다른 터미널에서)
telnet localhost 9882

# 대화형 쉘이 이제 사용 가능
sdplane> help
sdplane> show version
```

## 팁

### NIC 드라이버로 vfio-pci 사용 시 IOMMU 필요

가상화 기능이 활성화되어야 합니다:
- Intel: Intel VT-d
- AMD: AMD IOMMU / AMD-V

이러한 옵션은 BIOS 설정에서 활성화해야 합니다.
GRUB 구성도 변경해야 할 수 있습니다:

```conf
# /etc/default/grub
GRUB_CMDLINE_LINUX="iommu=pt intel_iommu=on"
```

변경 사항 적용:
```bash
sudo update-grub
sudo reboot
```

### Linux 커널 모듈 vfio-pci를 영구적으로 로드하는 구성

```bash
# 자동 로드용 구성 파일 생성
sudo tee /etc/modules-load.d/vfio-pci.conf > /dev/null <<EOF
vfio-pci
EOF
```

### Mellanox ConnectX 시리즈용

다음 링크에서 드라이버 설치가 필요합니다:

https://network.nvidia.com/products/ethernet-drivers/linux/mlnx_en/

설치 중에 `./install --dpdk`를 실행하세요.
**`--dpdk` 옵션은 필수입니다.**

다음 설정은 필요하지 않으므로 sdplane.conf에서 주석 처리하세요:

```conf
#set device {pcie-id} driver unbind
#set device {pcie-id} driver {driver-name} driver_override  
#set device {pcie-id} driver {driver-name} bind
```

Mellanox NIC의 경우 포트 정보를 업데이트하려면 `update port status` 명령을 실행해야 합니다.

### PCIe 버스 번호 확인 방법

DPDK의 dpdk-devbind.py 명령을 사용하여 NIC의 PCIe 버스 번호를 확인할 수 있습니다:

```bash
# 네트워크 장치 상태 표시  
dpdk-devbind.py -s

# 출력 예시:
Network devices using kernel driver
===================================
0000:04:00.0 'NetXtreme BCM5720 Gigabit Ethernet PCIe 165f' numa_node=0 if=eno8303 drv=tg3 unused= *Active*
0000:b1:00.0 'MT27800 Family [ConnectX-5] 1017' numa_node=1 if=enp177s0np0 drv=mlx5_core unused= *Active*
```

### 구성 파일의 워커 스레드 순서

`rib-manager`, `neigh-manager` 및 `netlink-thread` 워커를 구성하는 경우 사용하면 이 순서로 구성해야 합니다.

### DPDK 초기화

구성 파일에서 `rte_eal_init()`를 호출하는 명령은 하나만 호출해야 합니다. `rte_eal_init()` 함수는 `rte_eal_init`, `pktgen init`, `l2fwd init` 및 `l3fwd init`과 같은 명령에 의해 호출됩니다.

## 사용자 가이드 (매뉴얼)

포괄적인 사용자 가이드 및 명령 참조를 사용할 수 있습니다:

- [사용자 가이드](manual/ko/README.md) - 완전한 개요 및 명령 분류

**애플리케이션 가이드:**
- [L2 리피터 애플리케이션](manual/ko/l2-repeater-application.md) - MAC 학습을 통한 간단한 레이어 2 패킷 전달
- [고급 리피터 애플리케이션](manual/ko/enhanced-repeater-application.md) - TAP 인터페이스를 통한 VLAN 인식 스위칭  
- [패킷 생성기 애플리케이션](manual/ko/packet-generator-application.md) - 고성능 트래픽 생성 및 테스트

**구성 가이드:**
- [RIB & 라우팅](manual/ko/routing.md) - RIB 및 라우팅 기능
- [고급 리피터](manual/ko/enhanced-repeater.md) - 고급 리피터 구성
- [포트 관리 & 통계](manual/ko/port-management.md) - DPDK 포트 관리 및 통계
- [Worker & lcore 관리 & 스레드 정보](manual/ko/worker-lcore-thread-management.md) - Worker 스레드, lcore 및 스레드 정보 관리
- [시스템 정보 & 모니터링](manual/ko/system-monitoring.md) - 시스템 정보 및 모니터링
- [장치 관리](manual/ko/device-management.md) - 장치 및 드라이버 관리

**개발자 가이드:**
- [DPDK 통합 가이드](manual/ko/dpdk-integration-guide.md) - DPDK 애플리케이션 통합 방법
- [디버그 & 로깅](manual/ko/debug-logging.md) - 디버그 및 로깅 기능
- [큐 구성](manual/ko/queue-configuration.md) - 큐 구성 및 관리
- [TAP 인터페이스](manual/ko/tap-interface.md) - TAP 인터페이스 관리
- [VTY & 셸 관리](manual/ko/vty-shell.md) - VTY 및 셸 관리
- [lthread 관리](manual/ko/lthread-management.md) - 협력적 경량 스레드 관리
- [패킷 생성](manual/ko/packet-generation.md) - PKTGEN을 사용한 패킷 생성

## 개발자 가이드

### 통합 가이드
- [DPDK 애플리케이션 통합 가이드](manual/ko/dpdk-integration-guide.md) - DPDK-dock 접근 방식을 사용하여 DPDK 애플리케이션을 sdplane과 통합하는 방법

### 문서
- 모든 개발자 문서는 `doc/`에 포함되어 있습니다
- 통합 가이드 및 예시는 `doc/manual/ko/`에 있습니다

### 코드 스타일

이 프로젝트는 GNU 코딩 표준을 따릅니다. 코드를 확인하고 포맷하려면:

```bash
# 스타일 확인 (clang-format 18.1.3+ 필요)
./style/check_gnu_style.sh check

# 코드 자동 포맷
./style/check_gnu_style.sh update  

# 차이점 표시
./style/check_gnu_style.sh diff
```

필요한 도구 설치:
```bash
# Ubuntu 24.04용
sudo apt install clang-format-18
```

## 라이선스

이 프로젝트는 Apache 2.0 라이선스 하에 있습니다 - 자세한 내용은 [LICENSE](LICENSE) 파일을 참조하세요.

## 연락처

- GitHub: https://github.com/kait-cronos/sdplane-oss  
- Issues: https://github.com/kait-cronos/sdplane-oss/issues

## 평가용 장비 구매

평가용 장비에는 추가 기능 및 소프트웨어 수정이 포함될 수 있습니다.

평가용 장비에 대한 문의는 GitHub Issues 또는 이메일을 통해 직접 연락하세요.