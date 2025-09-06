# 향상된 리피터 애플리케이션

**언어 / Language:** [English](../enhanced-repeater-application.md) | [日本語](../ja/enhanced-repeater-application.md) | [Français](../fr/enhanced-repeater-application.md) | [中文](../zh/enhanced-repeater-application.md) | [Deutsch](../de/enhanced-repeater-application.md) | [Italiano](../it/enhanced-repeater-application.md) | **한국어**

향상된 리피터 애플리케이션은 L3 라우팅과 패킷 캡처 기능을 위한 TAP 인터페이스 통합을 갖춘 고급 VLAN 지원 레이어 2 스위칭을 제공합니다.

## 개요

향상된 리피터는 기본적인 L2 전달을 다음 기능으로 확장하는 정교한 레이어 2 전달 애플리케이션입니다:
- **VLAN 지원 스위칭**: IEEE 802.1Q VLAN의 완전한 지원 (태깅/언태깅)
- **가상 스위치 추상화**: 독립적인 전달 테이블을 가진 여러 가상 스위치
- **TAP 인터페이스 통합**: 커널 통합을 위한 라우터 인터페이스와 캡처 인터페이스
- **고급 VLAN 처리**: VLAN 변환, 삽입, 제거 기능

## 아키텍처

### 가상 스위치 프레임워크
향상된 리피터는 가상 스위치 아키텍처를 구현합니다:
- **다중 VLAN**: 여러 VLAN 도메인(1-4094) 지원
- **포트 집약**: 가상 스위치당 여러 물리 포트
- **격리된 전달**: VLAN별 독립적인 전달 도메인
- **유연한 태깅**: 포트별 네이티브, 태그, 변환 모드

### TAP 인터페이스 통합
- **라우터 인터페이스**: L3 처리를 위한 커널 네트워크 스택 통합
- **캡처 인터페이스**: 패킷 모니터링 및 분석 기능
- **링 버퍼**: 데이터 플레인과 커널 간의 효율적인 패킷 전송
- **양방향**: 입출력 모두 패킷 처리

## 주요 기능

### VLAN 처리
- **VLAN 태깅**: 언태그 프레임에 802.1Q 헤더 추가
- **VLAN 언태깅**: 태그 프레임에서 802.1Q 헤더 제거
- **VLAN 변환**: 입출력 간 VLAN ID 변경
- **네이티브 VLAN**: 트렁크 포트에서 언태그 트래픽 처리

### 가상 스위칭
- **학습**: VLAN별 자동 MAC 주소 학습
- **플러딩**: 미지 유니캐스트와 브로드캐스트의 적절한 처리
- **스플릿 호라이즌**: 가상 스위치 내 루프 방지
- **다중 도메인**: VLAN별 독립적인 전달 테이블

### 패킷 처리
- **제로 복사**: 최소 오버헤드로 효율적인 DPDK 패킷 처리
- **버스트 처리**: 높은 패킷 속도에 최적화
- **헤더 조작**: 효율적인 VLAN 헤더 삽입/제거
- **복사 최적화**: TAP 인터페이스를 위한 선택적 패킷 복사

## 설정

### 가상 스위치 설정
다른 VLAN 도메인용 가상 스위치 생성:

```bash
# VLAN ID를 가진 가상 스위치 생성
set vswitch 2031
set vswitch 2032
```

### 포트를 가상 스위치에 링크
물리 DPDK 포트를 가상 스위치에 링크:

```bash
# 포트 0을 가상 스위치 0에 VLAN 태그 2031로 링크
set vswitch-link vswitch 0 port 0 tag 2031

# 포트 0을 가상 스위치 1에 네이티브/언태그로 링크
set vswitch-link vswitch 1 port 0 tag 0

# VLAN 변환 예제
set vswitch-link vswitch 0 port 1 tag 2040
```

### 라우터 인터페이스
L3 연결용 라우터 인터페이스 생성:

```bash
# L3 처리용 라우터 인터페이스 생성
set vswitch 2031 router-if rif2031
set vswitch 2032 router-if rif2032
```

### 캡처 인터페이스
모니터링용 캡처 인터페이스 설정:

```bash
# 패킷 모니터링용 캡처 인터페이스 생성
set vswitch 2031 capture-if cif2031
set vswitch 2032 capture-if cif2032
```

### 워커 설정
향상된 리피터를 lcore에 할당:

```bash
# 향상된 리피터 워커 설정
set worker lcore 1 enhanced-repeater

# 큐 할당 설정
set thread 1 port 0 queue 0
```

## 설정 예제

### 전체 설정
[`example-config/sdplane_enhanced_repeater.conf`](../../example-config/sdplane_enhanced_repeater.conf)를 참조하세요:

```bash
# 장치 설정
set device 03:00.0 driver vfio-pci bind

# DPDK 초기화
set rte_eal argv -c 0x7
rte_eal_init
set mempool

# 포트 설정
stop port all
set port all dev-configure 1 4
set port all nrxdesc 1024
set port all ntxdesc 1024

# 워커 설정
set worker lthread stat-collector
set worker lthread rib-manager
set worker lcore 1 enhanced-repeater
set worker lcore 2 l3-tap-handler

# 프로미스큐어스 모드 활성화
set port all promiscuous enable
start port all

# 큐 설정
set thread 1 port 0 queue 0

# 가상 스위치 설정
set vswitch 2031
set vswitch 2032

# 링크 설정
set vswitch-link vswitch 0 port 0 tag 2031
set vswitch-link vswitch 1 port 0 tag 2032

# 인터페이스 생성
set vswitch 2031 router-if rif2031
set vswitch 2032 router-if rif2032
set vswitch 2031 capture-if cif2031
set vswitch 2032 capture-if cif2032

# 워커 시작
start worker lcore all
```

## VLAN 처리 세부사항

### 태깅 모드

#### 태그 모드
- **입력**: 특정 VLAN 태그를 가진 패킷 수용
- **출력**: VLAN 태그 유지 또는 변환
- **용도**: 트렁크 포트, VLAN 변환

#### 네이티브 모드 (tag 0)
- **입력**: 언태그 패킷 수용
- **출력**: VLAN 헤더 제거
- **용도**: 액세스 포트, VLAN 미지원 장치

#### 변환 모드
- **기능**: 포트 간 VLAN ID 변경
- **설정**: 입출력 링크에서 서로 다른 태그
- **용도**: VLAN 매핑, 서비스 프로바이더 네트워크

### 패킷 플로우

1. **입력 처리**:
   - DPDK 포트에서 패킷 수신
   - 태그 또는 네이티브 설정에 따른 VLAN 결정
   - 대상 가상 스위치 조회

2. **가상 스위치 조회**:
   - MAC 주소 학습 및 조회
   - VLAN 도메인 내 출력 포트 결정
   - 미지 유니캐스트/브로드캐스트 플러딩 처리

3. **출력 처리**:
   - 포트 설정별 VLAN 헤더 조작
   - 여러 대상용 패킷 복사
   - TAP 인터페이스 통합

4. **TAP 인터페이스 처리**:
   - 라우터 인터페이스: 커널 L3 처리
   - 캡처 인터페이스: 모니터링 및 분석

## 모니터링 및 관리

### 상태 명령어
```bash
# 가상 스위치 설정 표시
show vswitch_rib

# 가상 스위치 링크 표시
show vswitch-link

# 라우터 인터페이스 표시
show rib vswitch router-if

# 캡처 인터페이스 확인
show rib vswitch capture-if
```

### 통계 및 성능
```bash
# 포트 레벨 통계
show port statistics all

# 워커 성능
show worker statistics

# 스레드 정보
show thread information
```

### 디버그 명령어
```bash
# 향상된 리피터 디버그
debug sdplane enhanced-repeater

# VLAN 스위치 디버그 (대체 워커 타입)
debug sdplane vlan-switch

# RIB 및 전달 디버그
debug sdplane rib
debug sdplane fdb-change
```

## 사용 사례

### VLAN 집약
- 단일 물리 링크에서 여러 VLAN 통합
- 프로바이더 엣지 기능
- 서비스 다중화

### L3 통합
- Router-on-a-stick 설정
- TAP 인터페이스를 통한 VLAN 간 라우팅
- 하이브리드 L2/L3 전달

### 네트워크 모니터링
- VLAN별 패킷 캡처
- 트래픽 분석 및 디버그
- 서비스 모니터링

### 서비스 프로바이더 네트워크
- 고객 격리를 위한 VLAN 변환
- 멀티테넌트 네트워킹
- 트래픽 엔지니어링

## 성능 고려사항

### 스케일링
- **워커 할당**: 최적 성능을 위한 lcore 전용화
- **큐 설정**: 코어 간 큐 할당 균형
- **메모리 풀**: 패킷 속도와 버퍼 요구에 적절한 크기 설정

### VLAN 처리 오버헤드
- **헤더 조작**: VLAN 조작의 최소 오버헤드
- **패킷 복사**: 필요한 경우에만 선택적 복사
- **버스트 최적화**: 효율성을 위한 버스트 단위 패킷 처리

### TAP 인터페이스 성능
- **링 버퍼 크기**: 적절한 링 크기 설정
- **선택적 전송**: TAP 인터페이스에 관련 패킷만 전송
- **커널 통합**: 커널 처리 오버헤드 고려

## 문제 해결

### 일반적인 문제
- **VLAN 불일치**: 태그 설정이 네트워크 설정과 일치하는지 확인
- **TAP 인터페이스 생성**: 적절한 권한과 커널 지원 확인
- **성능 문제**: 큐 할당 및 워커 분산 확인

### 디버그 전략
- **디버그 로그 활성화**: 상세한 패킷 플로우를 위한 디버그 명령어 사용
- **통계 모니터링**: 포트 및 워커 통계 모니터링
- **패킷 캡처**: 트래픽 분석에 캡처 인터페이스 사용

## 관련 문서

- [향상된 리피터 명령어](enhanced-repeater.md) - 완전한 명령어 참조
- [워커 관리](worker-lcore-thread-management.md) - 워커 설정 세부사항
- [포트 관리](port-management.md) - DPDK 포트 설정
- [TAP 인터페이스 관리](tap-interface.md) - TAP 인터페이스 세부사항