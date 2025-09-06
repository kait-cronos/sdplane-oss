# RIB·라우팅

**언어 / Language:** [English](../routing.md) | [日本語](../ja/routing.md) | [Français](../fr/routing.md) | [中文](../zh/routing.md) | [Deutsch](../de/routing.md) | [Italiano](../it/routing.md) | **한국어**

RIB(Routing Information Base)와 시스템 리소스 정보를 관리하는 명령어입니다.

## 명령어 목록

### show_rib - RIB 정보 표시
```
show rib
```

RIB(Routing Information Base) 정보를 표시합니다.

**사용 예:**
```bash
show rib
```

이 명령어는 다음 정보를 표시합니다:
- RIB 버전과 메모리 포인터
- 가상 스위치 구성과 VLAN 할당
- DPDK 포트 상태와 큐 구성
- lcore-to-port 큐 할당
- L2/L3 전달용 네이버 테이블

## RIB 개요

### RIB란
RIB(Routing Information Base)는 시스템 리소스와 네트워크 정보를 저장하는 중앙 데이터베이스입니다. sdplane에서는 다음 정보를 관리합니다:

- **가상 스위치 구성** - VLAN 스위칭과 포트 할당
- **DPDK 포트 정보** - 링크 상태, 큐 구성, 기능 정보
- **lcore 큐 구성** - CPU 코어별 패킷 처리 할당
- **네이버 테이블** - L2/L3 전달 데이터베이스 엔트리

### RIB 구조
RIB는 두 개의 주요 구조체로 구성됩니다:

```c
struct rib {
    struct rib_info *rib_info;  // 실제 데이터에 대한 포인터
};

struct rib_info {
    uint32_t ver;                                    // 버전 번호
    uint8_t vswitch_size;                           // 가상 스위치 수
    uint8_t port_size;                              // DPDK 포트 수
    uint8_t lcore_size;                             // lcore 수
    struct vswitch_conf vswitch[MAX_VSWITCH];       // 가상 스위치 구성
    struct vswitch_link vswitch_link[MAX_VSWITCH_LINK]; // VLAN 포트 링크
    struct port_conf port[MAX_ETH_PORTS];           // DPDK 포트 구성
    struct lcore_qconf lcore_qconf[RTE_MAX_LCORE];  // lcore 큐 할당
    struct neigh_table neigh_tables[NEIGH_NR_TABLES]; // 네이버/전달 테이블
};
```

## RIB 정보 해석

### 기본 표시 항목
- **Destination** - 대상 네트워크 주소
- **Netmask** - 넷마스크
- **Gateway** - 게이트웨이(넥스트 홉)
- **Interface** - 출력 인터페이스
- **Metric** - 루트 메트릭 값
- **Status** - 루트 상태

### 루트 상태
- **Active** - 활성 루트
- **Inactive** - 비활성 루트
- **Pending** - 설정 중인 루트
- **Invalid** - 무효한 루트

## 사용 예

### 기본 사용법
```bash
# RIB 정보 표시
show rib
```

### 출력 예제 해석
```
rib information version: 21 (0x55555dd42010)
vswitches: 
dpdk ports: 
  dpdk_port[0]: 
    link: speed=1000Mbps duplex=full autoneg=on status=up
    nb_rxd=1024 nb_txd=1024
    queues: nrxq=1 ntxq=4
  dpdk_port[1]: 
    link: speed=0Mbps duplex=half autoneg=on status=down
    nb_rxd=1024 nb_txd=1024
    queues: nrxq=1 ntxq=4
  dpdk_port[2]: 
    link: speed=0Mbps duplex=half autoneg=off status=down
    nb_rxd=1024 nb_txd=1024
    queues: nrxq=1 ntxq=4
lcores: 
  lcore[0]: nrxq=0
  lcore[1]: nrxq=1
    rxq[0]: dpdk_port[0], queue_id=0
  lcore[2]: nrxq=1
    rxq[0]: dpdk_port[1], queue_id=0
  lcore[3]: nrxq=1
    rxq[0]: dpdk_port[2], queue_id=0
  lcore[4]: nrxq=0
  lcore[5]: nrxq=0
  lcore[6]: nrxq=0
  lcore[7]: nrxq=0
```

이 예제에서:
- RIB 버전 21이 현재 시스템 상태를 나타냄
- DPDK 포트 0이 활성(up) 상태이며 1Gbps 링크 속도
- DPDK 포트 1, 2는 비활성(down) 상태이며 링크 없음
- lcore 1, 2, 3이 각각 포트 0, 1, 2에서의 패킷 처리를 담당
- 각 포트는 1개의 RX 큐와 4개의 TX 큐를 사용
- RX/TX 디스크립터 링은 1024 엔트리로 설정

## RIB 관리

### 자동 업데이트
RIB는 다음 시점에서 자동으로 업데이트됩니다:
- 인터페이스 상태 변경
- 네트워크 설정 변경
- 라우팅 프로토콜로부터 정보 수신

### 수동 업데이트
RIB 정보를 수동으로 확인하려면:
```bash
show rib
```

## 문제 해결

### 라우팅이 올바르게 작동하지 않는 경우
1. RIB 정보 확인
```bash
show rib
```

2. 인터페이스 상태 확인
```bash
show port
```

3. 워커 상태 확인
```bash
show worker
```

### RIB에 루트가 표시되지 않는 경우
- 네트워크 설정 확인
- 인터페이스 상태 확인
- RIB 관리자 동작 확인

## 고급 기능

### RIB 관리자
RIB 관리자는 독립적인 스레드로 동작하며 다음 기능을 제공합니다:
- 라우팅 정보 자동 업데이트
- 루트 유효성 검사
- 네트워크 상태 모니터링

### 관련 워커
- **rib-manager** - RIB 관리를 수행하는 워커
- **l3fwd** - Layer 3 포워딩에서 RIB 활용
- **l3fwd-lpm** - LPM 테이블과 RIB 연동

## 정의 위치

이 명령어는 다음 파일에 정의되어 있습니다:
- `sdplane/rib.c`

## 관련 항목

- [워커·lcore 관리](worker-management.md)
- [lthread 관리](lthread-management.md)
- [시스템 정보·모니터링](system-monitoring.md)