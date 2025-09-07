# 시스템 정보·모니터링

**Language:** [English](../en/system-monitoring.md) | [日本語](../ja/system-monitoring.md) | [Français](../fr/system-monitoring.md) | [中文](../zh/system-monitoring.md) | [Deutsch](../de/system-monitoring.md) | [Italiano](../it/system-monitoring.md) | **한국어**

sdplane의 시스템 정보 및 모니터링 기능을 제공하는 명령어입니다.

## 명령어 목록

### show_version - 버전 표시
```
show version
```

sdplane의 버전 정보를 표시합니다.

**사용 예:**
```bash
show version
```

### set_locale - 로케일 설정
```
set locale (C|C.utf8|en_US.utf8|POSIX)
```

시스템 로케일을 설정합니다.

**사용 가능한 로케일:**
- `C` - 표준 C 로케일
- `C.utf8` - UTF-8 지원 C 로케일
- `en_US.utf8` - 영어 UTF-8 로케일
- `POSIX` - POSIX 로케일

**사용 예:**
```bash
# UTF-8 지원 C 로케일로 설정
set locale C.utf8

# 영어 UTF-8 로케일로 설정
set locale en_US.utf8
```

### set_argv_list_1 - argv-list 설정
```
set argv-list <0-7> <WORD>
```

명령행 인수 목록을 설정합니다.

**매개변수:**
- `<0-7>` - 인덱스(0-7)
- `<WORD>` - 설정할 문자열

**사용 예:**
```bash
# 인덱스 0에 인수 설정
set argv-list 0 "--verbose"

# 인덱스 1에 인수 설정
set argv-list 1 "--config"
```

### **show argv-list**

설정된 모든 명령행 인수 목록을 표시합니다.

**사용 예:**
```bash
# 모든 argv-list 표시
show argv-list
```

---

### **show argv-list \<0-7\>**

특정 인덱스의 argv-list를 표시합니다.

**사용 예:**
```bash
# 특정 인덱스의 argv-list 표시
show argv-list 0

# argv-list 인덱스 3 표시
show argv-list 3
```

### show_loop_count - 루프 카운터 표시
```
show loop-count (console|vty-shell|l2fwd) (pps|total)
```

각 컴포넌트의 루프 카운터를 표시합니다.

**컴포넌트:**
- `console` - 콘솔
- `vty-shell` - VTY 셸
- `l2fwd` - L2 포워딩

**통계 타입:**
- `pps` - 초당 루프 수
- `total` - 총 루프 수

**사용 예:**
```bash
# 콘솔의 PPS 표시
show loop-count console pps

# L2 포워딩의 총 루프 수 표시
show loop-count l2fwd total
```

### show_rcu - RCU 정보 표시
```
show rcu
```

RCU(Read-Copy-Update) 정보를 표시합니다.

**사용 예:**
```bash
show rcu
```

### show_fdb - FDB 정보 표시
```
show fdb
```

FDB(Forwarding Database) 정보를 표시합니다.

**사용 예:**
```bash
show fdb
```

### show_vswitch - vswitch 정보 표시
```
show vswitch
```

가상 스위치 정보를 표시합니다.

**사용 예:**
```bash
show vswitch
```

### sleep_cmd - 슬리프 명령어
```
sleep <0-300>
```

지정된 초 동안 대기합니다.

**매개변수:**
- `<0-300>` - 대기 시간(초)

**사용 예:**
```bash
# 5초간 대기
sleep 5

# 30초간 대기
sleep 30
```

### show_mempool - 메모리 풀 정보 표시
```
show mempool
```

DPDK 메모리 풀 정보를 표시합니다.

**사용 예:**
```bash
show mempool
```

## 모니터링 항목 설명

### 버전 정보
- sdplane 버전
- 빌드 정보
- 의존 라이브러리 버전

### 루프 카운터
- 각 컴포넌트의 처리 루프 횟수
- 성능 모니터링에 사용
- PPS(Packets Per Second) 계산에 사용

### RCU 정보
- Read-Copy-Update 메커니즘 상태
- 동기화 처리 상황
- 메모리 관리 상태

### FDB 정보
- MAC 주소 테이블 상태
- 학습된 MAC 주소
- 에이징아웃 정보

### vswitch 정보
- 가상 스위치 설정
- 포트 정보
- VLAN 설정

### 메모리 풀 정보
- 사용 가능한 메모리
- 사용 중인 메모리
- 메모리 풀 통계

## 모니터링 모범 사례

### 정기 모니터링
```bash
# 기본 모니터링 명령어
show version
show mempool
show vswitch
show rcu
```

### 성능 모니터링
```bash
# 루프 카운터를 통한 성능 모니터링
show loop-count console pps
show loop-count l2fwd pps
```

### 문제 해결
```bash
# 시스템 상태 확인
show fdb
show vswitch
show mempool
```

## 정의 위치

이러한 명령어는 다음 파일에 정의되어 있습니다:
- `sdplane/sdplane.c`

## 관련 항목

- [포트 관리·통계](port-management.md)
- [워커·lcore 관리](worker-lcore-thread-management.md)
- [스레드 정보](worker-lcore-thread-management.md)