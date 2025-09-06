# lthread 관리

**언어 / Language:** [English](../lthread-management.md) | [日本語](../ja/lthread-management.md) | [Français](../fr/lthread-management.md) | [中文](../zh/lthread-management.md) | [Deutsch](../de/lthread-management.md) | [Italiano](../it/lthread-management.md) | **한국어**

lthread(경량 스레드) 관리를 수행하는 명령어입니다.

## 명령어 목록

### set_worker_lthread_stat_collector - lthread 통계 수집기 설정
```
set worker lthread stat-collector
```

통계 정보를 수집하는 lthread 워커를 설정합니다.

**사용 예:**
```bash
set worker lthread stat-collector
```

### set_worker_lthread_rib_manager - lthread RIB 관리자 설정
```
set worker lthread rib-manager
```

RIB(Routing Information Base)를 관리하는 lthread 워커를 설정합니다.

**사용 예:**
```bash
set worker lthread rib-manager
```

### set_worker_lthread_netlink_thread - lthread netlink 스레드 설정
```
set worker lthread netlink-thread
```

Netlink 통신을 처리하는 lthread 워커를 설정합니다.

**사용 예:**
```bash
set worker lthread netlink-thread
```

## lthread 개요

### lthread란
lthread(lightweight thread)는 협력적 멀티스레드를 구현하는 경량 스레드 구현입니다.

### 주요 특징
- **경량성** - 낮은 메모리 오버헤드
- **고속성** - 고속 컨텍스트 스위치
- **협력성** - 명시적 yield를 통한 제어
- **확장성** - 대량의 스레드를 효율적으로 관리

### 기존 스레드와의 차이점
- **선점형 vs 협력형** - 명시적 제어권 양도
- **커널 스레드 vs 사용자 스레드** - 커널을 거치지 않는 실행
- **무거움 vs 가벼움** - 생성·전환 오버헤드가 작음

## lthread 워커 종류

### 통계 수집기 (stat-collector)
시스템 통계 정보를 정기적으로 수집하는 워커입니다.

**기능:**
- 포트 통계 수집
- 스레드 통계 수집
- 시스템 통계 수집
- 통계 데이터 집계

**사용 예:**
```bash
# 통계 수집기 설정
set worker lthread stat-collector

# 통계 정보 확인
show port statistics
show thread counter
```

### RIB 관리자 (rib-manager)
RIB(Routing Information Base)를 관리하는 워커입니다.

**기능:**
- 라우팅 테이블 관리
- 루트 추가·삭제·업데이트
- 루트 정보 배포
- 라우팅 상태 모니터링

**사용 예:**
```bash
# RIB 관리자 설정
set worker lthread rib-manager

# RIB 정보 확인
show rib
```

### Netlink 스레드 (netlink-thread)
Netlink 통신을 처리하는 워커입니다.

**기능:**
- 커널과의 Netlink 통신
- 네트워크 설정 모니터링
- 인터페이스 상태 모니터링
- 라우팅 정보 수신

**사용 예:**
```bash
# Netlink 스레드 설정
set worker lthread netlink-thread

# 네트워크 상태 확인
show port
show vswitch
```

## lthread 설정

### 기본 설정 절차
1. **필요한 lthread 워커 설정**
```bash
# 통계 수집기 설정
set worker lthread stat-collector

# RIB 관리자 설정
set worker lthread rib-manager

# Netlink 스레드 설정
set worker lthread netlink-thread
```

2. **설정 확인**
```bash
# 워커 상태 확인
show worker

# 스레드 정보 확인
show thread
```

### 권장 설정
일반적인 용도에서는 다음 조합을 권장합니다:
```bash
# 기본 lthread 워커 설정
set worker lthread stat-collector
set worker lthread rib-manager
set worker lthread netlink-thread
```

## 사용 예

### 기본 설정
```bash
# lthread 워커 설정
set worker lthread stat-collector
set worker lthread rib-manager
set worker lthread netlink-thread

# 설정 확인
show worker
show thread
```

### 통계 모니터링 설정
```bash
# 통계 수집기 설정
set worker lthread stat-collector

# 통계 정보 정기 확인
show port statistics
show thread counter
show mempool
```

### 라우팅 관리 설정
```bash
# RIB 관리자 설정
set worker lthread rib-manager

# 라우팅 정보 확인
show rib
show vswitch
```

## 모니터링 및 관리

### lthread 상태 확인
```bash
# 전체 상태 확인
show worker
show thread

# 특정 통계 정보 확인
show thread counter
show loop-count console pps
```

### 성능 모니터링
```bash
# lthread 성능 확인
show thread counter

# 시스템 전체 성능 확인
show port statistics pps
show mempool
```

## 문제 해결

### lthread가 동작하지 않는 경우
1. 워커 설정 확인
```bash
show worker
```

2. 스레드 상태 확인
```bash
show thread
```

3. 시스템 상태 확인
```bash
show rcu
show mempool
```

### 통계 정보가 업데이트되지 않는 경우
1. 통계 수집기 상태 확인
```bash
show worker
show thread
```

2. 통계 정보 수동 확인
```bash
show port statistics
show thread counter
```

### RIB가 업데이트되지 않는 경우
1. RIB 관리자 상태 확인
```bash
show worker
show thread
```

2. RIB 정보 확인
```bash
show rib
```

3. Netlink 스레드 상태 확인
```bash
show worker
```

## 고급 기능

### lthread 협력 동작
lthread는 협력적으로 동작하므로 다음 사항에 주의가 필요합니다:

- **명시적 yield** - 장시간 처리 시 명시적 yield 필요
- **데드락 회피** - 협력적 설계로 데드락 회피
- **공정성** - 모든 스레드가 적절히 실행되도록 보장

### 성능 최적화
- **적절한 워커 배치** - CPU 코어와의 친화성 고려
- **메모리 효율성** - 적절한 메모리 풀 크기 설정
- **부하 분산** - 여러 워커를 통한 부하 분산

## 정의 위치

이러한 명령어는 다음 파일에 정의되어 있습니다:
- `sdplane/lthread_main.c`

## 관련 항목

- [워커·lcore 관리](worker-management.md)
- [스레드 정보](thread-information.md)
- [시스템 정보·모니터링](system-monitoring.md)
- [RIB·라우팅](routing.md)