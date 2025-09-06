# L2 리피터 애플리케이션

**언어 / Language:** [English](../l2-repeater-application.md) | [日本語](../ja/l2-repeater-application.md) | [Français](../fr/l2-repeater-application.md) | [中文](../zh/l2-repeater-application.md) | [Deutsch](../de/l2-repeater-application.md) | [Italiano](../it/l2-repeater-application.md) | **한국어**

L2 리피터 애플리케이션은 기본적인 포트 간 리핏 기능을 갖춘 DPDK 포트 간의 간단한 레이어 2 패킷 전달을 제공합니다.

## 개요

L2 리피터는 다음 기능을 가진 직관적인 레이어 2 전달 애플리케이션입니다:
- 쌍으로 구성된 DPDK 포트 간 패킷 전달 (간단한 포트 간 리핏)
- MAC 주소 학습을 수행하지 않는 기본적인 패킷 리핏 기능
- 선택적 MAC 주소 업데이트 기능 (송신자 MAC 주소 변경)
- DPDK 제로 복사 패킷 처리를 통한 고성능 동작

## 주요 기능

### 레이어 2 전달
- **포트 간 리핏**: 사전 구성된 포트 쌍 간의 간단한 패킷 전달
- **MAC 학습 없음**: 전달 테이블을 구축하지 않는 직접적인 패킷 리핏
- **투명 전달**: 대상 MAC에 관계없이 모든 패킷 전달
- **포트 페어링**: 고정적인 포트 대 포트 전달 설정

### 성능 특성
- **제로 복사 처리**: DPDK의 효율적인 패킷 처리 사용
- **버스트 처리**: 최적의 처리량을 위한 패킷 버스트 처리
- **저지연**: 고속 전달을 위한 최소한의 처리 오버헤드
- **멀티코어 지원**: 스케일링을 위한 전용 lcore에서 실행

## 설정

### 기본 설정
L2 리피터는 메인 sdplane 설정 시스템을 통해 설정됩니다:

```bash
# 워커 타입을 L2 리피터로 설정
set worker lcore 1 l2-repeater

# 포트 및 큐 설정
set thread 1 port 0 queue 0  
set thread 1 port 1 queue 0

# 학습을 위한 프로미스큐어스 모드 활성화
set port all promiscuous enable
```

### 설정 파일 예제
전체 설정 예제는 [`example-config/sdplane_l2_repeater.conf`](../../example-config/sdplane_l2_repeater.conf)를 참조하세요:

```bash
# 장치 바인딩
set device 02:00.0 driver vfio-pci bind
set device 03:00.0 driver vfio-pci bind

# DPDK 초기화
set rte_eal argv -c 0x7
rte_eal_init

# 백그라운드 워커 (큐 설정 전에 시작 필요)
set worker lthread stat-collector
set worker lthread rib-manager
set worker lthread netlink-thread

set mempool

# 큐 설정 (rib-manager 시작 후 실행 필요)
set thread 1 port 0 queue 0
set thread 1 port 1 queue 0

# 포트 설정
stop port all
set port all dev-configure 1 4
set port all nrxdesc 1024
set port all ntxdesc 1024

# 워커 설정
set worker lcore 1 l2-repeater
set worker lcore 2 tap-handler
set port all promiscuous enable
start port all

# 포트 시작 대기
sleep 5

# 워커 시작
start worker lcore all
```

## 동작

### 브로드캐스트 전달
L2 리피터는 다른 모든 활성 포트로 패킷을 전달합니다:
- **브로드캐스트 동작**: 포트에서 수신한 모든 패킷을 다른 모든 활성 포트로 전달
- **스플릿 호라이즌**: 수신 포트로는 패킷을 다시 보내지 않음
- **필터링 없음**: 모든 패킷 타입 전달 (유니캐스트, 브로드캐스트, 멀티캐스트)

### 전달 동작
- **전체 트래픽 전달**: 대상 MAC에 관계없이 모든 패킷 리핏
- **전체 포트 브로드캐스트**: 입력 포트 이외의 모든 활성 포트로 패킷 전달
- **투명**: 패킷 내용 변경 없음 (MAC 업데이트가 활성화되지 않은 한)
- **멀티포트 복제**: 각 대상 포트용 패킷 복사본 생성

### MAC 주소 업데이트
활성화되면 L2 리피터가 패킷의 MAC 주소를 변경할 수 있습니다:
- **송신자 MAC 업데이트**: 송신자 MAC을 출력 포트의 MAC으로 변경
- **투명 브리지**: 원래 MAC 주소 유지 (기본값)

## 성능 조정

### 버퍼 설정
```bash
# 워크로드에 맞게 디스크립터 수 최적화
set port all nrxdesc 2048  # 높은 패킷 속도를 위해 증가
set port all ntxdesc 2048  # 버퍼링을 위해 증가
```

### 워커 할당
```bash
# L2 전달을 위한 특정 lcore 전용화
set worker lcore 1 l2-repeater  # 전용 코어에 할당
set worker lcore 2 tap-handler  # TAP 처리 분리
```

### 메모리 풀 크기 조정
메모리 풀은 예상 트래픽에 맞게 적절히 크기를 설정해야 합니다:
- 패킷 속도와 버퍼 요구사항 고려
- 버스트 크기와 임시 패킷 저장 고려

## 모니터링 및 디버깅

### 포트 통계
```bash
# 전달 통계 표시
show port statistics all

# 특정 포트 모니터링
show port statistics 0
show port statistics 1
```

### 디버그 명령어
```bash
# L2 리피터 디버그 활성화
debug sdplane l2-repeater

# VLAN 스위치 디버그 (대체 워커 타입)
debug sdplane vlan-switch

# 일반 sdplane 디버그
debug sdplane fdb-change
debug sdplane rib
```

## 사용 사례

### 허브형 리피터
- 연결된 모든 포트로 트래픽 복제
- 투명 레이어 2 리핏
- 학습 기능 없는 기본 허브 기능

### 포트 미러링/리피팅
- 포트 간 트래픽 미러링
- 네트워크 모니터링 및 분석
- 간단한 패킷 복제

### 성능 테스트
- 전달 성능 측정
- L2 전달 벤치마크 기준선
- DPDK 포트 설정 검증

## 제한사항

- **VLAN 처리 없음**: VLAN 인식 없는 간단한 L2 리핏
- **MAC 학습 없음**: 주소 학습 없는 전체 포트 브로드캐스트 전달
- **STP 지원 없음**: 스패닝 트리 프로토콜 구현 없음
- **필터링 없음**: 대상에 관계없이 모든 패킷 전달

## 관련 애플리케이션

- **향상된 리피터**: VLAN 지원과 TAP 인터페이스를 갖춘 고급 버전
- **L3 전달**: 레이어 3 라우팅 기능
- **VLAN 스위치**: VLAN 지원 스위칭 기능

VLAN 지원을 포함한 더 고급 레이어 2 기능에 대해서는 [향상된 리피터](enhanced-repeater.md) 문서를 참조하세요.