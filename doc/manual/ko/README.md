# sdplane-oss 사용자 가이드

**Language:** [English](../en/README.md) | [日本語](../ja/README.md) | [Français](../fr/README.md) | [中文](../zh/README.md) | [Deutsch](../de/README.md) | [Italiano](../it/README.md) | **한국어**

sdplane-oss는 고성능 DPDK 기반 소프트웨어 라우터입니다. 이 사용자 가이드에서는 sdplane의 모든 명령어와 기능에 대해 설명합니다.

## 목차

1. [포트 관리·통계](port-management.md) - DPDK 포트 관리 및 통계 정보
2. [Worker & lcore 관리 & 스레드 정보](worker-lcore-thread-management.md) - Worker 스레드, lcore 및 스레드 정보 관리
3. [디버그·로그](debug-logging.md) - 디버그 및 로그 기능
4. [VTY·셸 관리](vty-shell.md) - VTY와 셸 관리
5. [시스템 정보·모니터링](system-monitoring.md) - 시스템 정보 및 모니터링 기능
6. [RIB·라우팅](routing.md) - RIB와 라우팅 기능
7. [큐 설정](queue-configuration.md) - 큐 설정 및 관리
8. [패킷 생성](packet-generation.md) - PKTGEN을 사용한 패킷 생성

9. [TAP 인터페이스](tap-interface.md) - TAP 인터페이스 관리
10. [lthread 관리](lthread-management.md) - lthread 관리
11. [장치 관리](device-management.md) - 장치 및 드라이버 관리

## 기본 사용법

### 연결 방법

sdplane에 연결하려면:

```bash
# sdplane 시작
sudo ./sdplane/sdplane

# 다른 터미널에서 CLI에 연결
telnet localhost 9882
```

### 도움말 표시

각 명령어에서 `?`를 사용하여 도움말을 표시할 수 있습니다:

```
sdplane# ?
sdplane# show ?
sdplane# set ?
```

### 기본 명령어

- `show version` - 버전 정보 표시
- `show port` - 포트 정보 표시
- `show worker` - 워커 정보 표시
- `exit` - CLI에서 종료

## 명령어 분류

sdplane에는 79개의 명령어가 정의되어 있으며, 다음과 같은 13개의 기능 카테고리로 분류됩니다:

1. **포트 관리·통계** (10개 명령어) - DPDK 포트 제어 및 통계
2. **워커·lcore 관리** (6개 명령어) - 워커 스레드와 lcore 관리
3. **디버그·로그** (2개 명령어) - 디버그 및 로그 기능
4. **VTY·셸 관리** (4개 명령어) - VTY와 셸 제어
5. **시스템 정보·모니터링** (10개 명령어) - 시스템 정보 및 모니터링
6. **RIB·라우팅** (1개 명령어) - 라우팅 정보 관리
7. **큐 설정** (3개 명령어) - 큐 설정
8. **패킷 생성** (3개 명령어) - PKTGEN을 통한 패킷 생성
9. **스레드 정보** (2개 명령어) - 스레드 모니터링
10. **TAP 인터페이스** (2개 명령어) - TAP 인터페이스 관리
11. **lthread 관리** (3개 명령어) - lthread 관리
12. **장치 관리** (2개 명령어) - 장치 및 드라이버 관리

자세한 사용법은 각 카테고리의 문서를 참조하세요.