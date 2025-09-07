# 장치 관리

**언어 / Language:** [English](../en/device-management.md) | [日本語](../ja/device-management.md) | [Français](../fr/device-management.md) | [中文](../zh/device-management.md) | [Deutsch](../de/device-management.md) | [Italiano](../it/device-management.md) | **한국어**

DPDK 장치와 드라이버 관리를 수행하는 명령어입니다.

## 명령어 목록

### show_devices - 장치 정보 표시
```
show devices
```

시스템에서 사용 가능한 장치 정보를 표시합니다.

**사용 예:**
```bash
show devices
```

이 명령어는 다음 정보를 표시합니다:
- 장치명
- 장치 타입
- 현재 드라이버
- 장치 상태
- PCI 주소

### set_device_driver - 장치 드라이버 설정
```
set device <WORD> driver (ixgbe|igb|igc|uio_pci_generic|igb_uio|vfio-pci|unbound) (|bind|driver_override)
```

지정된 장치에 드라이버를 설정합니다.

**매개변수:**
- `<WORD>` - 장치명 또는 PCI 주소
- 드라이버 종류:
  - `ixgbe` - Intel 10GbE ixgbe 드라이버
  - `igb` - Intel 1GbE igb 드라이버
  - `igc` - Intel 2.5GbE igc 드라이버
  - `uio_pci_generic` - 범용 UIO 드라이버
  - `igb_uio` - DPDK UIO 드라이버
  - `vfio-pci` - VFIO PCI 드라이버
  - `unbound` - 드라이버 제거
- 동작 모드:
  - `bind` - 드라이버 바인딩
  - `driver_override` - 드라이버 오버라이드

**사용 예:**
```bash
# 장치를 vfio-pci 드라이버에 바인딩
set device 0000:01:00.0 driver vfio-pci bind

# 장치를 igb_uio 드라이버로 설정
set device eth0 driver igb_uio

# 장치에서 드라이버 제거
set device 0000:01:00.0 driver unbound
```

## 장치 관리 개요

### DPDK 장치 관리
DPDK에서는 네트워크 장치를 효율적으로 사용하기 위해 전용 드라이버를 사용합니다.

### 드라이버 종류

#### 네트워크 드라이버
- **ixgbe** - Intel 10GbE 네트워크 카드용
- **igb** - Intel 1GbE 네트워크 카드용
- **igc** - Intel 2.5GbE 네트워크 카드용

#### UIO(Userspace I/O) 드라이버
- **uio_pci_generic** - 범용 UIO 드라이버
- **igb_uio** - DPDK 전용 UIO 드라이버

#### VFIO(Virtual Function I/O) 드라이버
- **vfio-pci** - 가상화 환경에서 고성능 I/O

#### 특수 설정
- **unbound** - 드라이버를 제거하여 장치 비활성화

## 장치 설정 절차

### 기본 설정 절차
1. **장치 확인**
```bash
show devices
```

2. **드라이버 설정**
```bash
set device <device> driver <driver> bind
```

3. **설정 확인**
```bash
show devices
```

4. **포트 설정**
```bash
show port
```

### 설정 예

#### Intel 10GbE 카드 설정
```bash
# 장치 확인
show devices

# ixgbe 드라이버 바인딩
set device 0000:01:00.0 driver ixgbe bind
set device 0000:01:00.1 driver ixgbe bind

# 설정 확인
show devices
show port
```

#### DPDK UIO 드라이버 설정
```bash
# 장치 확인
show devices

# igb_uio 드라이버 바인딩
set device 0000:02:00.0 driver igb_uio bind
set device 0000:02:00.1 driver igb_uio bind

# 설정 확인
show devices
show port
```

#### VFIO 설정 (가상화 환경)
```bash
# 장치 확인
show devices

# vfio-pci 드라이버 바인딩
set device 0000:03:00.0 driver vfio-pci bind
set device 0000:03:00.1 driver vfio-pci bind

# 설정 확인
show devices
show port
```

## 드라이버 선택 지침

### ixgbe (Intel 10GbE)
- **용도**: Intel 10GbE 네트워크 카드
- **장점**: 고성능, 안정성
- **조건**: Intel 10GbE 카드 필요

### igb (Intel 1GbE)
- **용도**: Intel 1GbE 네트워크 카드
- **장점**: 광범위한 호환성, 안정성
- **조건**: Intel 1GbE 카드 필요

### igc (Intel 2.5GbE)
- **용도**: Intel 2.5GbE 네트워크 카드
- **장점**: 중간 성능, 새로운 표준
- **조건**: Intel 2.5GbE 카드 필요

### uio_pci_generic
- **용도**: 범용 장치
- **장점**: 광범위한 호환성
- **단점**: 일부 기능 제한

### igb_uio
- **용도**: DPDK 전용 환경
- **장점**: DPDK 최적화
- **단점**: 별도 설치 필요

### vfio-pci
- **용도**: 가상화 환경, 보안 중시
- **장점**: 보안, 가상화 지원
- **조건**: IOMMU 활성화 필요

## 문제 해결

### 장치가 인식되지 않는 경우
1. 장치 확인
```bash
show devices
```

2. 시스템 레벨에서 확인
```bash
lspci | grep Ethernet
```

3. 커널 모듈 확인
```bash
lsmod | grep uio
lsmod | grep vfio
```

### 드라이버 바인딩에 실패하는 경우
1. 현재 드라이버 확인
```bash
show devices
```

2. 기존 드라이버 제거
```bash
set device <device> driver unbound
```

3. 목적 드라이버 바인딩
```bash
set device <device> driver <target_driver> bind
```

### 포트를 사용할 수 없는 경우
1. 장치 상태 확인
```bash
show devices
show port
```

2. 드라이버 재바인딩
```bash
set device <device> driver unbound
set device <device> driver <driver> bind
```

3. 포트 설정 확인
```bash
show port
update port status
```

## 고급 기능

### 드라이버 오버라이드
```bash
# 드라이버 오버라이드 사용
set device <device> driver <driver> driver_override
```

### 다중 장치 일괄 설정
```bash
# 여러 장치를 순차적으로 설정
set device 0000:01:00.0 driver vfio-pci bind
set device 0000:01:00.1 driver vfio-pci bind
set device 0000:02:00.0 driver vfio-pci bind
set device 0000:02:00.1 driver vfio-pci bind
```

## 보안 고려사항

### VFIO 사용 시 주의점
- IOMMU 활성화 필요
- 보안 그룹 설정
- 적절한 권한 설정

### UIO 사용 시 주의점
- root 권한 필요
- 보안 위험 이해
- 적절한 접근 제어

## 시스템 통합

### systemd 서비스와의 연동
```bash
# systemd 서비스에서 자동 설정
# /etc/systemd/system/sdplane.service에 설정
```

### 부팅 시 자동 설정
```bash
# 부팅 스크립트에서 설정
# /etc/init.d/sdplane 또는 systemd unit file
```

## 정의 위치

이러한 명령어는 다음 파일에 정의되어 있습니다:
- `sdplane/dpdk_devbind.c`

## 관련 항목

- [포트 관리·통계](port-management.md)
- [워커·lcore 관리](worker-lcore-thread-management.md)
- [시스템 정보·모니터링](system-monitoring.md)