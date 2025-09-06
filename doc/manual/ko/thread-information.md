# 스레드 정보

**언어 / Language:** [English](../thread-information.md) | [日本語](../ja/thread-information.md) | [Français](../fr/thread-information.md) | [中文](../zh/thread-information.md) | [Deutsch](../de/thread-information.md) | [Italiano](../it/thread-information.md) | **한국어**

스레드 정보와 모니터링을 수행하는 명령어입니다.

## 명령어 목록

### show_thread_cmd - 스레드 정보 표시
```
show thread
```

현재 스레드의 상태와 정보를 표시합니다.

**사용 예:**
```bash
show thread
```

이 명령어는 다음 정보를 표시합니다:
- 스레드 ID
- 스레드 상태
- 실행 중인 작업
- CPU 사용률
- 메모리 사용량

### show_thread_counter - 스레드 카운터 표시
```
show thread counter
```

스레드 카운터 정보를 표시합니다.

**사용 예:**
```bash
show thread counter
```

이 명령어는 다음 정보를 표시합니다:
- 처리 패킷 수
- 실행 횟수
- 오류 횟수
- 처리 시간 통계

## 스레드 시스템 개요

### sdplane의 스레드 아키텍처
sdplane에서는 다음 스레드 타입이 사용됩니다:

#### 1. 워커 스레드
- **L2 포워딩** - Layer 2 패킷 전송
- **L3 포워딩** - Layer 3 패킷 전송
- **TAP 핸들러** - TAP 인터페이스 처리
- **PKTGEN** - 패킷 생성

#### 2. 관리 스레드
- **RIB 관리자** - 라우팅 정보 관리
- **통계 수집기** - 통계 정보 수집
- **Netlink 스레드** - Netlink 통신 처리

#### 3. 시스템 스레드
- **VTY 서버** - VTY 연결 처리
- **콘솔** - 콘솔 입출력

### lthread와의 관계
sdplane은 협력적 멀티스레드(cooperative threading) 모델을 사용합니다:

- **lthread** - 경량 스레드 구현
- **협력적 스케줄링** - 명시적 yield를 통한 제어
- **고효율** - 컨텍스트 스위치 오버헤드 감소

## 스레드 정보 해석

### 기본 표시 항목
- **Thread ID** - 스레드 식별자
- **Name** - 스레드 이름
- **State** - 스레드 상태
- **lcore** - 실행 중인 CPU 코어
- **Type** - 스레드 타입

### 스레드 상태
- **Running** - 실행 중
- **Ready** - 실행 가능
- **Blocked** - 블록됨
- **Terminated** - 종료됨

### 카운터 정보
- **Packets** - 처리 패킷 수
- **Loops** - 루프 실행 횟수
- **Errors** - 오류 횟수
- **CPU Time** - CPU 사용 시간

## 사용 예

### 기본 모니터링
```bash
# 스레드 정보 표시
show thread

# 스레드 카운터 표시
show thread counter
```

### 출력 예제 해석
```bash
# show thread 출력 예
Thread ID: 1
Name: l2fwd-worker
State: Running
lcore: 1
Type: L2FWD

Thread ID: 2
Name: rib-manager
State: Running
lcore: 2
Type: RIB_MANAGER
```

```bash
# show thread counter 출력 예
Thread ID: 1
Packets: 1000000
Loops: 5000000
Errors: 0
CPU Time: 123.45s
```

## 모니터링 및 문제 해결

### 정기 모니터링
```bash
# 정기 모니터링 명령어
show thread
show thread counter
```

### 성능 분석
```bash
# 성능 관련 정보
show thread counter
show loop-count l2fwd pps
show worker
```

### 문제 해결

#### 스레드가 응답하지 않는 경우
1. 스레드 상태 확인
```bash
show thread
```

2. 워커 상태 확인
```bash
show worker
```

3. 필요시 재시작
```bash
restart worker lcore 1
```

#### 성능이 저하되는 경우
1. 카운터 정보 확인
```bash
show thread counter
```

2. 루프 카운터 확인
```bash
show loop-count l2fwd pps
```

3. 오류 횟수 확인
```bash
show thread counter
```

#### 메모리 사용량이 많은 경우
1. 메모리 풀 정보 확인
```bash
show mempool
```

2. 스레드 정보 확인
```bash
show thread
```

## 스레드 최적화

### CPU 친화성 설정
- 적절한 lcore에 워커 배치
- NUMA 노드를 고려한 배치
- CPU 사용률 균등화

### 메모리 효율성 개선
- 적절한 메모리 풀 크기
- 메모리 누수 방지
- 캐시 효율성 향상

## 고급 기능

### lthread 관리
```bash
# lthread 워커 설정
set worker lthread stat-collector
set worker lthread rib-manager
set worker lthread netlink-thread
```

### 통계 정보 활용
- 성능 모니터링
- 용량 계획
- 이상 감지

## 정의 위치

이러한 명령어는 다음 파일에 정의되어 있습니다:
- `sdplane/thread_info.c`

## 관련 항목

- [워커·lcore 관리](worker-management.md)
- [lthread 관리](lthread-management.md)
- [시스템 정보·모니터링](system-monitoring.md)