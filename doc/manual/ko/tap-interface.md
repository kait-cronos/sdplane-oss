# TAP 인터페이스

**Language:** [English](../en/tap-interface.md) | [日本語](../ja/tap-interface.md) | [Français](../fr/tap-interface.md) | [中文](../zh/tap-interface.md) | [Deutsch](../de/tap-interface.md) | [Italiano](../it/tap-interface.md) | **한국어**

TAP 인터페이스 관리를 수행하는 명령어입니다.

## 명령어 목록

### set_tap_capture_ifname - TAP 캡처 인터페이스명 설정
```
set tap capture ifname <WORD>
```

TAP 캡처 기능에서 사용할 인터페이스명을 설정합니다.

**매개변수:**
- `<WORD>` - 인터페이스명

**사용 예:**
```bash
# tap0 인터페이스 설정
set tap capture ifname tap0

# tap1 인터페이스 설정
set tap capture ifname tap1
```

### set_tap_capture_persistent - TAP 캡처 영속화 활성화
```
set tap capture persistent
```

TAP 캡처의 영속화를 활성화합니다.

**사용 예:**
```bash
# 영속화 활성화
set tap capture persistent
```

### no_tap_capture_persistent - TAP 캡처 영속화 비활성화
```
no tap capture persistent
```

TAP 캡처의 영속화를 비활성화합니다.

**사용 예:**
```bash
# 영속화 비활성화
no tap capture persistent
```

### unset_tap_capture_persistent - TAP 캡처 영속화 설정 삭제
```
unset tap capture persistent
```

TAP 캡처의 영속화 설정을 삭제합니다.

**사용 예:**
```bash
# 영속화 설정 삭제
unset tap capture persistent
```

## TAP 인터페이스 개요

### TAP 인터페이스란
TAP(Network TAP) 인터페이스는 네트워크 트래픽의 모니터링과 테스트에 사용되는 가상 네트워크 인터페이스입니다.

### 주요 기능
- **패킷 캡처** - 네트워크 트래픽 캡처
- **패킷 주입** - 테스트 패킷 주입
- **브리지 기능** - 다른 네트워크 간 브리지
- **모니터링 기능** - 트래픽 모니터링 및 분석

### sdplane에서의 용도
- **디버깅** - 패킷 플로우 디버깅
- **테스트** - 네트워크 기능 테스트
- **모니터링** - 트래픽 모니터링
- **개발** - 새 기능 개발 및 테스트

## TAP 인터페이스 설정

### 기본 설정 절차
1. **TAP 인터페이스 생성**
```bash
# 시스템 레벨에서 TAP 인터페이스 생성
sudo ip tuntap add tap0 mode tap
sudo ip link set tap0 up
```

2. **sdplane에서의 설정**
```bash
# TAP 캡처 인터페이스명 설정
set tap capture ifname tap0

# 영속화 활성화
set tap capture persistent
```

3. **TAP 핸들러 워커 설정**
```bash
# TAP 핸들러 워커 설정
set worker lcore 2 tap-handler
start worker lcore 2
```

### 설정 예

#### 기본 TAP 설정
```bash
# TAP 인터페이스 설정
set tap capture ifname tap0
set tap capture persistent

# 워커 설정
set worker lcore 2 tap-handler
start worker lcore 2

# 설정 확인
show worker
```

#### 다중 TAP 인터페이스 설정
```bash
# 여러 TAP 인터페이스 설정
set tap capture ifname tap0
set tap capture ifname tap1

# 영속화 활성화
set tap capture persistent
```

## 영속화 기능

### 영속화란
영속화 기능을 활성화하면 TAP 인터페이스 설정이 시스템 재시작 후에도 유지됩니다.

### 영속화의 장점
- **설정 유지** - 재시작 후에도 설정 유지
- **자동 복구** - 시스템 장애로부터 자동 복구
- **운영 효율성** - 수동 설정 감소

### 영속화 설정
```bash
# 영속화 활성화
set tap capture persistent

# 영속화 비활성화
no tap capture persistent

# 영속화 설정 삭제
unset tap capture persistent
```

## 사용 예

### 디버깅 용도
```bash
# 디버깅용 TAP 인터페이스 설정
set tap capture ifname debug-tap
set tap capture persistent

# TAP 핸들러 워커 설정
set worker lcore 3 tap-handler
start worker lcore 3

# 패킷 캡처 시작
tcpdump -i debug-tap
```

### 테스트 용도
```bash
# 테스트용 TAP 인터페이스 설정
set tap capture ifname test-tap
set tap capture persistent

# 테스트 패킷 주입 준비
set worker lcore 4 tap-handler
start worker lcore 4
```

## 모니터링 및 관리

### TAP 인터페이스 상태 확인
```bash
# 워커 상태 확인
show worker

# 스레드 정보 확인
show thread

# 시스템 레벨에서 확인
ip link show tap0
```

### 트래픽 모니터링
```bash
# tcpdump를 사용한 모니터링
tcpdump -i tap0

# Wireshark를 사용한 모니터링
wireshark -i tap0
```

## 문제 해결

### TAP 인터페이스가 생성되지 않는 경우
1. 시스템 레벨에서 확인
```bash
# TAP 인터페이스 존재 확인
ip link show tap0

# 권한 확인
sudo ip tuntap add tap0 mode tap
```

2. sdplane에서 확인
```bash
# 설정 확인
show worker

# 워커 상태 확인
show thread
```

### 패킷이 캡처되지 않는 경우
1. 인터페이스 상태 확인
```bash
ip link show tap0
```

2. 워커 상태 확인
```bash
show worker
```

3. TAP 핸들러 재시작
```bash
restart worker lcore 2
```

### 영속화가 작동하지 않는 경우
1. 영속화 설정 확인
```bash
# 현재 설정 확인 (show 계열 명령어로 확인)
show worker
```

2. 시스템 설정 확인
```bash
# 시스템 레벨에서 설정 확인
systemctl status sdplane
```

## 고급 기능

### VLAN과의 연동
TAP 인터페이스는 VLAN 기능과 연동하여 사용할 수 있습니다:
```bash
# VLAN 스위치 워커와의 연동
set worker lcore 5 vlan-switch
start worker lcore 5
```

### 브리지 기능
여러 TAP 인터페이스를 브리지하여 사용:
```bash
# 여러 TAP 인터페이스 설정
set tap capture ifname tap0
set tap capture ifname tap1
```

## 정의 위치

이러한 명령어는 다음 파일에 정의되어 있습니다:
- `sdplane/tap_cmd.c`

## 관련 항목

- [워커·lcore 관리](worker-lcore-thread-management.md)
- [VTY·셸 관리](vty-shell.md)
- [디버그·로그](debug-logging.md)