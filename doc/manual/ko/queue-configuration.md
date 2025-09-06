# 큐 설정

**언어 / Language:** [English](../queue-configuration.md) | [日本語](../ja/queue-configuration.md) | [Français](../fr/queue-configuration.md) | [中文](../zh/queue-configuration.md) | [Deutsch](../de/queue-configuration.md) | [Italiano](../it/queue-configuration.md) | **한국어**

DPDK 큐의 설정과 관리를 수행하는 명령어입니다.

## 명령어 목록

### update_port_status - 포트 상태 업데이트
```
update port status
```

모든 포트의 상태를 업데이트합니다.

**사용 예:**
```bash
update port status
```

이 명령어는 다음을 실행합니다:
- 각 포트의 링크 상태 확인
- 큐 설정 업데이트
- 포트 통계 정보 업데이트

### set_thread_lcore_port_queue - 스레드 큐 설정
```
set thread <0-128> port <0-128> queue <0-128>
```

지정된 스레드에 포트와 큐를 할당합니다.

**매개변수:**
- `<0-128>` (thread) - 스레드 번호
- `<0-128>` (port) - 포트 번호
- `<0-128>` (queue) - 큐 번호

**사용 예:**
```bash
# 스레드 0에 포트 0의 큐 0 할당
set thread 0 port 0 queue 0

# 스레드 1에 포트 1의 큐 1 할당
set thread 1 port 1 queue 1

# 스레드 2에 포트 0의 큐 1 할당
set thread 2 port 0 queue 1
```

### show_thread_qconf - 스레드 큐 설정 표시
```
show thread qconf
```

현재 스레드 큐 설정을 표시합니다.

**사용 예:**
```bash
show thread qconf
```

## 큐 시스템 개요

### DPDK 큐 개념
DPDK에서는 각 포트에 여러 송신 큐와 수신 큐를 설정할 수 있습니다:

- **수신 큐(RX Queue)** - 수신 패킷 받기
- **송신 큐(TX Queue)** - 송신 패킷 전송
- **멀티큐** - 여러 큐의 병렬 처리

### 큐 설정의 중요성
적절한 큐 설정으로 다음이 실현됩니다:
- **성능 향상** - 병렬 처리로 고속화
- **부하 분산** - 여러 워커에서 처리 분산
- **CPU 효율성** - CPU 코어의 효율적 활용

## 큐 설정 방법

### 기본 설정 절차
1. **포트 상태 업데이트**
```bash
update port status
```

2. **스레드 큐 설정**
```bash
set thread 0 port 0 queue 0
set thread 1 port 0 queue 1
```

3. **설정 확인**
```bash
show thread qconf
```

### 권장 설정 패턴

#### 단일 포트, 단일 큐
```bash
set thread 0 port 0 queue 0
```

#### 단일 포트, 여러 큐
```bash
set thread 0 port 0 queue 0
set thread 1 port 0 queue 1
set thread 2 port 0 queue 2
```

#### 여러 포트, 여러 큐
```bash
set thread 0 port 0 queue 0
set thread 1 port 1 queue 0
set thread 2 port 2 queue 0
```

## 설정 예

### 고성능 설정(4코어, 4포트)
```bash
# 포트 상태 업데이트
update port status

# 각 코어에 다른 포트 할당
set thread 0 port 0 queue 0
set thread 1 port 1 queue 0
set thread 2 port 2 queue 0
set thread 3 port 3 queue 0

# 설정 확인
show thread qconf
```

### 부하 분산 설정(2코어, 1포트)
```bash
# 포트 상태 업데이트
update port status

# 하나의 포트에 여러 큐 설정
set thread 0 port 0 queue 0
set thread 1 port 0 queue 1

# 설정 확인
show thread qconf
```

## 성능 튜닝

### 큐 수 결정
- **CPU 코어 수** - 사용 가능한 CPU 코어 수에 따른 설정
- **포트 수** - 물리 포트 수 고려
- **트래픽 특성** - 예상 트래픽 패턴

### 최적화 포인트
1. **CPU 친화성** - CPU 코어와 큐의 적절한 배치
2. **메모리 배치** - NUMA 노드를 고려한 메모리 배치
3. **인터럽트 처리** - 효율적인 인터럽트 처리

## 문제 해결

### 큐 설정이 반영되지 않는 경우
1. 포트 상태 업데이트
```bash
update port status
```

2. 워커 상태 확인
```bash
show worker
```

3. 포트 상태 확인
```bash
show port
```

### 성능이 향상되지 않는 경우
1. 큐 설정 확인
```bash
show thread qconf
```

2. 스레드 부하 확인
```bash
show thread counter
```

3. 포트 통계 확인
```bash
show port statistics
```

## 정의 위치

이러한 명령어는 다음 파일에 정의되어 있습니다:
- `sdplane/queue_config.c`

## 관련 항목

- [포트 관리·통계](port-management.md)
- [워커·lcore 관리](worker-management.md)
- [스레드 정보](thread-information.md)