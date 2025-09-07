# 패킷 생성

**언어 / Language:** [English](../en/packet-generation.md) | [日本語](../ja/packet-generation.md) | [Français](../fr/packet-generation.md) | [中文](../zh/packet-generation.md) | [Deutsch](../de/packet-generation.md) | [Italiano](../it/packet-generation.md) | **한국어**

PKTGEN(Packet Generator)을 사용한 패킷 생성 기능의 명령어입니다.

## 명령어 목록

### show_pktgen - PKTGEN 정보 표시
```
show pktgen
```

PKTGEN(패킷 생성기)의 현재 상태와 설정을 표시합니다.

**사용 예:**
```bash
show pktgen
```

이 명령어는 다음 정보를 표시합니다:
- PKTGEN 초기화 상태
- 현재 설정 매개변수
- 실행 중인 작업
- 통계 정보

### pktgen_init - PKTGEN 초기화
```
pktgen init argv-list <0-7>
```

PKTGEN을 지정된 argv-list를 사용하여 초기화합니다.

**매개변수:**
- `<0-7>` - 사용할 argv-list의 인덱스

**사용 예:**
```bash
# argv-list 0을 사용하여 PKTGEN 초기화
pktgen init argv-list 0

# argv-list 2를 사용하여 PKTGEN 초기화
pktgen init argv-list 2
```

### pktgen_do_start - PKTGEN 시작
```
pktgen do start (<0-7>|all)
```

지정된 포트에서 패킷 생성을 시작합니다.

**대상:**
- `<0-7>` - 특정 포트 번호
- `all` - 모든 포트

**사용 예:**
```bash
# 포트 0에서 패킷 생성 시작
pktgen do start 0

# 모든 포트에서 패킷 생성 시작
pktgen do start all
```

### pktgen_do_stop - PKTGEN 중지
```
pktgen do stop (<0-7>|all)
```

지정된 포트에서 패킷 생성을 중지합니다.

**대상:**
- `<0-7>` - 특정 포트 번호
- `all` - 모든 포트

**사용 예:**
```bash
# 포트 1에서 패킷 생성 중지
pktgen do stop 1

# 모든 포트에서 패킷 생성 중지
pktgen do stop all
```

## PKTGEN 개요

### PKTGEN이란
PKTGEN(Packet Generator)은 네트워크 테스트를 위한 패킷 생성 도구입니다. 다음 기능을 제공합니다:

- **고속 패킷 생성** - 고성능 패킷 생성
- **다양한 패킷 형식** - 다양한 프로토콜 지원
- **유연한 설정** - 세부 패킷 설정 가능
- **통계 기능** - 상세한 통계 정보 제공

### 주요 용도
- **네트워크 성능 테스트** - 처리량과 지연 시간 측정
- **부하 테스트** - 시스템 부하 내성 테스트
- **기능 테스트** - 네트워크 기능 검증
- **벤치마크** - 성능 비교 테스트

## PKTGEN 설정

### 기본 설정 절차
1. **argv-list 설정**
```bash
# PKTGEN용 매개변수 설정
set argv-list 0 "-c 0x3 -n 4"
set argv-list 1 "--socket-mem 1024"
set argv-list 2 "--huge-dir /mnt/huge"
```

2. **PKTGEN 초기화**
```bash
pktgen init argv-list 0
```

3. **워커 설정**
```bash
set worker lcore 1 pktgen
start worker lcore 1
```

4. **패킷 생성 시작**
```bash
pktgen do start 0
```

### 설정 매개변수
argv-list에서 설정 가능한 매개변수 예:

- **-c** - CPU 마스크
- **-n** - 메모리 채널 수
- **--socket-mem** - 소켓 메모리 크기
- **--huge-dir** - 휴지 페이지 디렉터리
- **--file-prefix** - 파일 접두사

## 사용 예

### 기본 패킷 생성
```bash
# 설정
set argv-list 0 "-c 0x3 -n 4 --socket-mem 1024"

# 초기화
pktgen init argv-list 0

# 워커 설정
set worker lcore 1 pktgen
start worker lcore 1

# 패킷 생성 시작
pktgen do start 0

# 상태 확인
show pktgen

# 패킷 생성 중지
pktgen do stop 0
```

### 다중 포트 생성
```bash
# 다중 포트에서 시작
pktgen do start all

# 상태 확인
show pktgen

# 다중 포트에서 중지
pktgen do stop all
```

## 모니터링 및 통계

### 통계 정보 확인
```bash
# PKTGEN 통계 표시
show pktgen

# 포트 통계 표시
show port statistics

# 워커 통계 표시
show worker
```

### 성능 모니터링
```bash
# PPS(Packets Per Second) 확인
show port statistics pps

# 총 패킷 수 확인
show port statistics total

# 바이트/초 확인
show port statistics Bps
```

## 문제 해결

### PKTGEN이 시작되지 않는 경우
1. 초기화 상태 확인
```bash
show pktgen
```

2. 워커 상태 확인
```bash
show worker
```

3. 포트 상태 확인
```bash
show port
```

### 패킷 생성이 중지되지 않는 경우
1. 명시적으로 중지
```bash
pktgen do stop all
```

2. 워커 재시작
```bash
restart worker lcore 1
```

### 성능이 낮은 경우
1. CPU 사용률 확인
2. 메모리 설정 확인
3. 포트 설정 확인

## 고급 기능

### 패킷 형식 설정
PKTGEN에서는 다양한 패킷 형식을 생성할 수 있습니다:
- **Ethernet** - 기본 Ethernet 프레임
- **IP** - IPv4/IPv6 패킷
- **UDP/TCP** - UDP/TCP 패킷
- **VLAN** - VLAN 태그 패킷

### 부하 제어
- **속도 제어** - 패킷 생성 속도 제어
- **버스트 제어** - 버스트 패킷 생성
- **크기 제어** - 패킷 크기 제어

## 정의 위치

이러한 명령어는 다음 파일에 정의되어 있습니다:
- `sdplane/pktgen_cmd.c`

## 관련 항목

- [워커·lcore 관리](worker-lcore-thread-management.md)
- [포트 관리·통계](port-management.md)
- [시스템 정보·모니터링](system-monitoring.md)