# 패킷 생성기(PKTGEN) 애플리케이션

Created by Keith Wiles @ Intel 2010-2025 from https://github.com/pktgen/Pktgen-DPDK

**Language:** [English](../en/packet-generator-application.md) | [日本語](../ja/packet-generator-application.md) | [Français](../fr/packet-generator-application.md) | [中文](../zh/packet-generator-application.md) | [Deutsch](../de/packet-generator-application.md) | [Italiano](../it/packet-generator-application.md) | **한국어**

패킷 생성기(PKTGEN) 애플리케이션은 DPDK의 최적화된 패킷 처리 프레임워크를 사용한 고성능 패킷 생성 및 트래픽 테스트 기능을 제공합니다.

## 개요

PKTGEN은 sdplane-oss에 통합된 정교한 트래픽 생성 도구로 다음을 구현합니다:
- **고속 패킷 생성**: 멀티기가비트 패킷 전송 속도
- **유연한 트래픽 패턴**: 사용자 정의 가능한 패킷 크기, 속도, 패턴
- **다중 포트 지원**: 여러 포트에서 독립적인 트래픽 생성
- **고급 기능**: 범위 테스트, 속도 제한, 트래픽 셰이핑
- **성능 테스트**: 네트워크 처리량 및 지연 시간 측정

## 아키텍처

### 핵심 구성 요소
- **TX 엔진**: DPDK를 사용한 고성능 패킷 전송
- **RX 엔진**: 패킷 수신 및 통계 수집
- **L2P 프레임워크**: 최적 성능을 위한 lcore-to-port 매핑
- **설정 관리**: 동적 트래픽 매개변수 설정
- **통계 엔진**: 포괄적인 성능 메트릭 및 보고

### 워커 모델
PKTGEN은 전용 워커 스레드(lcore)를 사용하여 동작합니다:
- **TX 워커**: 패킷 전송 전용 코어
- **RX 워커**: 패킷 수신 전용 코어
- **혼합 워커**: 단일 코어에서 TX/RX 결합
- **제어 스레드**: 관리 및 통계 수집

## 주요 기능

### 트래픽 생성
- **패킷 속도**: 인터페이스 제한까지의 라인 레이트 트래픽 생성
- **패킷 크기**: 64바이트부터 점보 프레임까지 설정 가능
- **트래픽 패턴**: 균등, 버스트, 커스텀 패턴
- **멀티 스트림**: 포트당 여러 트래픽 스트림

### 고급 기능
- **속도 제한**: 정밀한 트래픽 속도 제어
- **범위 테스트**: 패킷 크기 스위프 및 속도 스위프
- **부하 패턴**: 상수, 램프업, 버스트 트래픽
- **프로토콜 지원**: 이더넷, IPv4, IPv6, TCP, UDP

### 성능 모니터링
- **실시간 통계**: TX/RX 속도, 패킷 수, 오류 수
- **지연 시간 측정**: 엔드투엔드 패킷 지연 시간 테스트
- **처리량 분석**: 대역폭 활용률 및 효율성
- **오류 감지**: 패킷 손실 및 손상 감지

## 설정

### 기본 설정
PKTGEN은 특정 초기화 및 워커 설정이 필요합니다:

```bash
# PKTGEN용 장치 바인딩
set device 01:00.0 driver vfio-pci bind
set device 01:00.1 driver vfio-pci bind

# argv-list를 통한 PKTGEN 초기화
set argv-list 2 ./usr/local/bin/pktgen -l 0-7 -n 4 --proc-type auto --log-level 7 --file-prefix pg -- -v -T -P -l pktgen.log -m [4:5].0 -m [6:7].1 -f themes/black-yellow.theme

# PKTGEN 초기화
pktgen init argv-list 2
```

### 워커 할당
최적 성능을 위한 전용 코어 할당:

```bash
# 특정 lcore에 PKTGEN 워커 할당
set worker lcore 4 pktgen  # 포트 0 TX/RX
set worker lcore 5 pktgen  # 포트 0 TX/RX
set worker lcore 6 pktgen  # 포트 1 TX/RX
set worker lcore 7 pktgen  # 포트 1 TX/RX
```

### 포트 설정
PKTGEN 사용을 위한 DPDK 포트 설정:

```bash
# 포트 설정
stop port all
set port all dev-configure 1 4
set port all nrxdesc 1024
set port all ntxdesc 1024
set port all promiscuous enable
start port all
```

## 설정 예제

### 완전한 PKTGEN 설정
[`example-config/sdplane-pktgen.conf`](../../example-config/sdplane-pktgen.conf)를 참조하세요:

```bash
# 로그 설정
log file /var/log/sdplane.log
log stdout

# 장치 바인딩
set device 01:00.0 driver vfio-pci bind
set device 01:00.1 driver vfio-pci bind
set device 03:00.0 driver uio_pci_generic bind

# PKTGEN 초기화
set argv-list 2 ./usr/local/bin/pktgen -l 0-7 -n 4 --proc-type auto --log-level 7 --file-prefix pg -- -v -T -P -l pktgen.log -m [4:5].0 -m [6:7].1 -f themes/black-yellow.theme
pktgen init argv-list 2

# 메모리 풀 설정
set mempool

# 백그라운드 워커
set worker lthread stat-collector
set worker lthread rib-manager
set worker lthread netlink-thread

# 포트 설정
stop port all
set port all dev-configure 1 4
set port all nrxdesc 1024
set port all ntxdesc 1024

# 워커 할당
set worker lcore 2 vlan-switch
set worker lcore 3 tap-handler
set worker lcore 4 pktgen
set worker lcore 5 pktgen
set worker lcore 6 pktgen
set worker lcore 7 pktgen

# 프로미스큐어스 모드 활성화 및 시작
set port all promiscuous enable
start port all

# 모든 워커 시작
start worker lcore all
```

## PKTGEN 명령행 인수

### 핵심 매개변수
- `-l 0-7`: CPU 코어 목록 (코어 0-7)
- `-n 4`: 메모리 채널 수
- `--proc-type auto`: 프로세스 타입 (프라이머리/세컨더리)
- `--log-level 7`: 디버그 로그 레벨
- `--file-prefix pg`: 공유 메모리 접두사

### PKTGEN 특정 옵션
- `-v`: 자세한 출력
- `-T`: 컬러 터미널 출력 활성화
- `-P`: 프로미스큐어스 모드 활성화
- `-l pktgen.log`: 로그 파일 위치
- `-m [4:5].0`: lcore 4,5를 포트 0에 매핑
- `-m [6:7].1`: lcore 6,7을 포트 1에 매핑
- `-f themes/black-yellow.theme`: 색상 테마

## lcore-to-port 매핑

### 매핑 구문
매핑 형식 `[lcore_list].port`로 코어를 포트에 할당:
- `[4:5].0`: lcore 4와 5가 포트 0 처리
- `[6:7].1`: lcore 6과 7이 포트 1 처리
- `[4].0`: 단일 lcore 4가 포트 0 처리
- `[4-7].0`: lcore 4부터 7까지가 포트 0 처리

### 성능 최적화
- **전용 코어**: TX와 RX에 별도 코어 할당
- **NUMA 인식**: 네트워크 인터페이스에 로컬한 코어 사용
- **경합 방지**: 시스템 워커와 중복 방지
- **부하 분산**: 여러 코어 간 트래픽 분산

## 운영 및 모니터링

### 상태 명령어
```bash
# PKTGEN 상태 및 설정 표시
show pktgen

# 워커 할당 및 성능 표시
show worker statistics

# 포트 레벨 통계
show port statistics all
```

### 런타임 관리
PKTGEN은 인터랙티브 CLI를 통해 광범위한 런타임 설정을 제공:
- **트래픽 매개변수**: 패킷 크기, 속도, 패턴
- **시작/중지 제어**: 포트별 트래픽 제어
- **통계**: 실시간 성능 모니터링
- **범위 테스트**: 매개변수 스위프 자동화

### 성능 모니터링
```bash
# 실시간 통계 모니터링
# (PKTGEN 인터랙티브 인터페이스를 통해 사용 가능)

# 주요 메트릭:
# - 초당 TX/RX 패킷 수
# - 대역폭 활용률
# - 패킷 손실률
# - 지연 시간 측정
```

## CLI 명령어

### PKTGEN 제어 명령어

#### 초기화
```bash
# argv-list 설정으로 PKTGEN 초기화
pktgen init argv-list <0-7>
```

#### 트래픽 생성 시작/중지
```bash
# 특정 포트에서 트래픽 생성 시작
pktgen do start port <0-7>
pktgen do start port all

# 트래픽 생성 중지
pktgen do stop port <0-7>
pktgen do stop port all
```

#### 트래픽 설정 명령어

##### 패킷 수 설정
```bash
# 전송할 패킷 수 설정
pktgen do set port <0-7> count <0-4000000000>
pktgen do set port all count <0-4000000000>
```

##### 패킷 크기 설정
```bash
# 패킷 크기를 바이트 단위로 설정
pktgen do set port <0-7> size <0-9999>
pktgen do set port all size <0-9999>
```

##### 전송 속도 설정
```bash
# 전송 속도를 백분율로 설정
pktgen do set port <0-7> rate <0-100>
pktgen do set port all rate <0-100>
```

##### TCP/UDP 포트 번호 설정
```bash
# TCP 송신자·대상 포트 설정
pktgen do set port <0-7> tcp src <0-65535> dst <0-65535>
pktgen do set port all tcp src <0-65535> dst <0-65535>

# UDP 송신자·대상 포트 설정
pktgen do set port <0-7> udp src <0-65535> dst <0-65535>
pktgen do set port all udp src <0-65535> dst <0-65535>
```

##### TTL 값 설정
```bash
# IP Time-to-Live 값 설정
pktgen do set port <0-7> ttl <0-255>
pktgen do set port all ttl <0-255>
```

##### MAC 주소 설정
```bash
# 송신자 MAC 주소 설정
pktgen do set port <0-7> mac source <MAC>
pktgen do set port all mac source <MAC>

# 대상 MAC 주소 설정
pktgen do set port <0-7> mac destination <MAC>
pktgen do set port all mac destination <MAC>
```

##### IPv4 주소 설정
```bash
# 송신자 IPv4 주소 설정
pktgen do set port <0-7> ipv4 source <IPv4>
pktgen do set port all ipv4 source <IPv4>

# 대상 IPv4 주소 설정
pktgen do set port <0-7> ipv4 destination <IPv4>
pktgen do set port all ipv4 destination <IPv4>
```

#### 상태·모니터링 명령어
```bash
# PKTGEN 상태 및 설정 표시
show pktgen

# 포트 통계 표시
show port statistics all
show port statistics <0-7>
```

## 사용 사례

### 네트워크 성능 테스트
- **처리량 테스트**: 최대 대역폭 측정
- **지연 시간 테스트**: 엔드투엔드 지연 시간 분석
- **부하 테스트**: 지속적인 트래픽 생성
- **스트레스 테스트**: 최대 패킷 속도 검증

### 장비 검증
- **스위치 테스트**: 전달 성능 검증
- **라우터 테스트**: L3 전달 성능
- **인터페이스 테스트**: 포트 및 케이블 검증
- **프로토콜 테스트**: 특정 프로토콜 동작

### 네트워크 개발
- **프로토콜 개발**: 새로운 네트워크 프로토콜 테스트
- **애플리케이션 테스트**: 현실적인 트래픽 패턴 생성
- **성능 튜닝**: 네트워크 설정 최적화
- **벤치마크**: 표준화된 성능 비교

## 성능 튜닝

### 코어 할당
```bash
# 최고 성능을 위한 코어 사용 최적화
# 가능한 경우 TX와 RX 코어 분리
# 네트워크 인터페이스에 로컬한 NUMA 코어 사용
set worker lcore 4 pktgen  # 포트 0의 TX 코어
set worker lcore 5 pktgen  # 포트 0의 RX 코어
```

### 메모리 설정
```bash
# 트래픽 패턴에 따라 디스크립터 링 최적화
set port all nrxdesc 2048  # 고속용으로 증가
set port all ntxdesc 2048  # 버스트용으로 증가
```

### 시스템 튜닝
- **CPU 격리**: OS 스케줄러에서 PKTGEN 코어 격리
- **인터럽트 친화성**: 비PKTGEN 코어에 인터럽트 바인딩
- **메모리 할당**: 최적 성능을 위해 휴지 페이지 사용
- **NIC 튜닝**: 네트워크 인터페이스 설정 최적화

## 문제 해결

### 일반적인 문제
- **낮은 성능**: 코어 할당 및 NUMA 토폴로지 확인
- **패킷 손실**: 버퍼 크기 및 시스템 리소스 확인
- **초기화 실패**: 장치 바인딩 및 권한 확인
- **속도 제한**: 인터페이스 능력 및 설정 확인

### 디버그 전략
- **자세한 로그 활성화**: 자세한 출력을 위해 높은 로그 레벨 사용
- **통계 확인**: 이상을 위한 TX/RX 카운터 모니터링
- **매핑 확인**: 올바른 lcore-to-port 할당 보장
- **시스템 모니터링**: 테스트 중 CPU, 메모리, 인터럽트 사용 확인

#### 디버그 명령어
```bash
# PKTGEN 디버그 로그 활성화
debug sdplane pktgen

# 일반 sdplane 디버그
debug sdplane rib
debug sdplane fdb-change
```

### 성능 검증
```bash
# PKTGEN이 예상 속도를 달성하는지 확인
show pktgen
show port statistics all

# 오류나 드롭 확인
# 테스트 중 시스템 리소스 모니터링
```

## 고급 기능

### 범위 테스트
PKTGEN은 매개변수 범위에서 자동 테스트를 지원:
- **패킷 크기 스위프**: 64부터 1518바이트까지 테스트
- **속도 스위프**: 라인 레이트 1%부터 100%까지 테스트
- **자동 보고**: 포괄적인 테스트 보고서 생성

### 트래픽 패턴
- **상수 속도**: 안정적인 트래픽 생성
- **버스트 패턴**: 유휴 기간이 있는 트래픽 버스트
- **램프 패턴**: 점진적으로 증가/감소하는 속도
- **커스텀 패턴**: 사용자 정의 트래픽 프로파일

## 관련 문서

- [패킷 생성 명령어](packet-generation.md) - 명령어 참조
- [워커 관리](worker-lcore-thread-management.md) - 워커 설정
- [포트 관리](port-management.md) - DPDK 포트 설정
- [성능 튜닝 가이드](#) - 시스템 최적화 팁

## 외부 리소스

- [DPDK Pktgen 문서](http://pktgen-dpdk.readthedocs.io/) - 공식 PKTGEN 문서
- [DPDK 성능 가이드](https://doc.dpdk.org/guides/prog_guide/) - DPDK 최적화 가이드