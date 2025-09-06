# 워커·lcore 관리

**언어 / Language:** [English](../worker-lcore-thread-management.md) | [日本語](../ja/worker-lcore-thread-management.md) | [Français](../fr/worker-lcore-thread-management.md) | [中文](../zh/worker-lcore-thread-management.md) | [Deutsch](../de/worker-lcore-thread-management.md) | [Italiano](../it/worker-lcore-thread-management.md) | **한국어**

DPDK 워커 스레드와 lcore 관리를 수행하는 명령어입니다.

## 명령어 목록

### set_worker - 워커 타입 설정
```
set worker lcore <0-16> (|none|l2fwd|l3fwd|l3fwd-lpm|tap-handler|l2-repeater|enhanced-repeater|vlan-switch|pktgen|linkflap-generator)
```

지정된 lcore에 워커 타입을 설정합니다.

**워커 타입:**
- `none` - 워커 없음
- `l2fwd` - Layer 2 포워딩
- `l3fwd` - Layer 3 포워딩
- `l3fwd-lpm` - Layer 3 포워딩 (LPM)
- `tap-handler` - TAP 인터페이스 핸들러
- `l2-repeater` - Layer 2 리피터
- `enhanced-repeater` - VLAN 스위칭과 TAP 인터페이스를 갖춘 향상된 리피터
- `vlan-switch` - VLAN 스위치
- `pktgen` - 패킷 생성기
- `linkflap-generator` - 링크 플랩 생성기

**사용 예:**
```bash
# lcore 1에 L2 포워딩 워커 설정
set worker lcore 1 l2fwd

# lcore 1에 향상된 리피터 워커 설정
set worker lcore 1 enhanced-repeater

# lcore 2에 워커 없음 설정
set worker lcore 2 none

# lcore 3에 L3 포워딩(LPM) 워커 설정
set worker lcore 3 l3fwd-lpm
```

### reset_worker - 워커 재설정
```
reset worker lcore <0-16>
```

지정된 lcore의 워커를 재설정합니다.

**사용 예:**
```bash
# lcore 2의 워커를 재설정
reset worker lcore 2
```

### start_worker - 워커 시작
```
start worker lcore <0-16>
```

지정된 lcore의 워커를 시작합니다.

**사용 예:**
```bash
# lcore 1의 워커를 시작
start worker lcore 1
```

### restart_worker - 워커 재시작
```
restart worker lcore <0-16>
```

지정된 lcore의 워커를 재시작합니다.

**사용 예:**
```bash
# lcore 4의 워커를 재시작
restart worker lcore 4
```

### start_worker_all - 워커 시작(all 옵션 포함)
```
start worker lcore (<0-16>|all)
```

지정된 lcore 또는 모든 lcore의 워커를 시작합니다.

**사용 예:**
```bash
# lcore 1의 워커를 시작
start worker lcore 1

# 모든 워커를 시작
start worker lcore all
```

### stop_worker - 워커 중지
```
stop worker lcore (<0-16>|all)
```

지정된 lcore 또는 모든 lcore의 워커를 중지합니다.

**사용 예:**
```bash
# lcore 1의 워커를 중지
stop worker lcore 1

# 모든 워커를 중지
stop worker lcore all
```

### reset_worker_all - 워커 재설정(all 옵션 포함)
```
reset worker lcore (<0-16>|all)
```

지정된 lcore 또는 모든 lcore의 워커를 재설정합니다.

**사용 예:**
```bash
# lcore 2의 워커를 재설정
reset worker lcore 2

# 모든 워커를 재설정
reset worker lcore all
```

### restart_worker_all - 워커 재시작(all 옵션 포함)
```
restart worker lcore (<0-16>|all)
```

지정된 lcore 또는 모든 lcore의 워커를 재시작합니다.

**사용 예:**
```bash
# lcore 3의 워커를 재시작
restart worker lcore 3

# 모든 워커를 재시작
restart worker lcore all
```

### show_worker - 워커 정보 표시
```
show worker
```

현재 워커의 상태와 설정을 표시합니다.

**사용 예:**
```bash
show worker
```

### set_mempool - 메모리 풀 설정
```
set mempool
```

DPDK 메모리 풀을 설정합니다.

**사용 예:**
```bash
set mempool
```

### set_rte_eal_argv - RTE EAL 명령행 인수 설정
```
set rte_eal argv <WORD> <WORD> <WORD> <WORD> <WORD> <WORD>
```

RTE EAL(Environment Abstraction Layer) 초기화에 사용할 명령행 인수를 설정합니다.

**사용 예:**
```bash
# EAL 인수 설정
set rte_eal argv -c 0x1 -n 4 --socket-mem 1024,1024 --huge-dir /mnt/huge
```

### rte_eal_init - RTE EAL 초기화
```
rte_eal_init
```

RTE EAL(Environment Abstraction Layer)을 초기화합니다.

**사용 예:**
```bash
rte_eal_init
```

## 워커 타입 설명

### L2 포워딩 (l2fwd)
Layer 2 레벨에서 패킷 전달을 수행하는 워커입니다. MAC 주소 기반으로 패킷을 전달합니다.

### L3 포워딩 (l3fwd)
Layer 3 레벨에서 패킷 전달을 수행하는 워커입니다. IP 주소 기반으로 라우팅을 수행합니다.

### L3 포워딩 LPM (l3fwd-lpm)
Longest Prefix Matching(LPM)을 사용한 Layer 3 포워딩 워커입니다.

### TAP 핸들러 (tap-handler)
TAP 인터페이스와 DPDK 포트 간의 패킷 전달을 수행하는 워커입니다.

### L2 리피터 (l2-repeater)
Layer 2 레벨에서 패킷 복제·리피트를 수행하는 워커입니다.

### VLAN 스위치 (vlan-switch)
VLAN(Virtual LAN) 기능을 제공하는 스위칭 워커입니다.

### 패킷 생성기 (pktgen)
테스트용 패킷을 생성하는 워커입니다.

### 링크 플랩 생성기 (linkflap-generator)
네트워크 링크 상태 변화를 테스트하기 위한 워커입니다.

## 정의 위치

이러한 명령어는 다음 파일에 정의되어 있습니다:
- `sdplane/dpdk_lcore_cmd.c`

## 관련 항목

- [포트 관리·통계](port-management.md)
- [스레드 정보](worker-lcore-thread-management.md)
- [lthread 관리](lthread-management.md)