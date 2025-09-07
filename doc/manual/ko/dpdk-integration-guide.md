# DPDK 애플리케이션 통합 가이드

**Language:** [English](../en/dpdk-integration-guide.md) | [日本語](../ja/dpdk-integration-guide.md) | [Français](../fr/dpdk-integration-guide.md) | [中文](../zh/dpdk-integration-guide.md) | [Deutsch](../de/dpdk-integration-guide.md) | [Italiano](../it/dpdk-integration-guide.md) | **한국어**

이 가이드는 기존 DPDK 애플리케이션과 그들의 pthread 기반 워커 스레드를 DPDK-dock 방식을 사용하여 sdplane 프레임워크에 통합하는 방법을 설명합니다.

## 개요

sdplane은 **DPDK-dock 방식**을 사용하여 여러 DPDK 애플리케이션을 단일 프로세스 공간 내에서 실행하는 통합 프레임워크를 제공합니다. 각 DPDK 애플리케이션이 자체 EAL 환경을 초기화하는 대신, 애플리케이션을 sdplane의 협력적 스레딩 모델 내의 워커 모듈로 통합할 수 있습니다.

DPDK-dock 방식을 통해 여러 DPDK 애플리케이션이 다음을 통해 효율적으로 리소스를 공유하고 공존할 수 있습니다:
- DPDK EAL 초기화의 중앙집중화
- 공유 메모리 풀 관리 제공
- 포트 및 큐 설정의 통합
- 다양한 DPDK 워크로드 간 협력적 멀티태스킹 실현

## 주요 통합 지점

### 워커 스레드 변환
- 기존 DPDK pthread 워커를 sdplane lcore 워커로 변환
- `pthread_create()`를 sdplane의 `set worker lcore <id> <worker-type>`으로 대체
- sdplane의 lcore 기반 스레딩 모델과 통합

### 초기화 통합
- 애플리케이션별 `rte_eal_init()` 호출 제거
- sdplane의 중앙집중화된 EAL 초기화 사용
- sdplane의 명령어 프레임워크를 통해 애플리케이션별 CLI 명령어 등록

### 메모리 및 포트 관리
- sdplane의 공유 메모리 풀 관리 활용
- sdplane의 포트 설정 및 큐 할당 시스템 사용
- 포트 상태를 위해 sdplane의 RIB(Routing Information Base)와 통합

## 통합 절차

### 1. 워커 함수 식별
DPDK 애플리케이션 내의 주요 패킷 처리 루프를 식별합니다. 이는 일반적으로 다음 특징을 가진 함수들입니다:
- 패킷을 처리하는 무한 루프에서 동작
- `rte_eth_rx_burst()`와 `rte_eth_tx_burst()` 사용
- 패킷 전달 또는 처리 로직 처리

### 2. 워커 모듈 생성
sdplane의 워커 인터페이스에 따라 워커 함수 구현:

```c
static __thread uint64_t loop_counter = 0;

int
my_worker_function(__rte_unused void *dummy)
{
    unsigned lcore_id = rte_lcore_id();
    int thread_id;
    
    // 모니터링용 루프 카운터 등록
    thread_id = thread_lookup_by_lcore(my_worker_function, lcore_id);
    thread_register_loop_counter(thread_id, &loop_counter);
    
    while (!force_quit && !force_stop[lcore_id]) {
        // 패킷 처리 로직을 여기에 작성
        
        // 모니터링용 루프 카운터 증가
        loop_counter++;
    }
    
    return 0;
}
```

### 워커 루프 카운터 모니터링

`loop_counter` 변수를 통해 sdplane 셸에서 워커 성능을 모니터링할 수 있습니다:

- **스레드 로컬 변수**: 각 워커가 자체 루프 카운터를 유지
- **등록**: 워커명과 lcore ID를 사용하여 sdplane의 모니터링 시스템에 카운터 등록
- **증가**: 메인 루프의 각 반복마다 카운터 증가
- **모니터링**: sdplane CLI에서 카운터 값을 표시하여 워커 활동 확인

**CLI 모니터링 명령어:**
```bash
# 루프 카운터를 포함한 스레드 카운터 정보 표시
show thread counter

# 일반 스레드 정보 표시
show thread

# 워커 설정 및 상태 표시
show worker
```

이를 통해 관리자는 워커가 활발히 처리를 수행하고 있음을 확인하고, 루프 카운터의 증분을 관찰함으로써 잠재적 성능 문제나 워커 정지를 감지할 수 있습니다.

### 3. RCU를 사용한 RIB 정보 접근

DPDK 패킷 처리 워커 내에서 포트 정보와 설정에 접근하기 위해, sdplane은 스레드 안전 작업을 위한 RCU(Read-Copy-Update)를 통해 RIB(Routing Information Base) 접근을 제공합니다.

#### RIB 접근 패턴

```c
#if HAVE_LIBURCU_QSBR
#include <urcu/urcu-qsbr.h>
#endif /*HAVE_LIBURCU_QSBR*/

static __thread struct rib *rib = NULL;

int
my_worker_function(__rte_unused void *dummy)
{
    unsigned lcore_id = rte_lcore_id();
    int thread_id;
    
    // 모니터링용 루프 카운터 등록
    thread_id = thread_lookup_by_lcore(my_worker_function, lcore_id);
    thread_register_loop_counter(thread_id, &loop_counter);
    
#if HAVE_LIBURCU_QSBR
    urcu_qsbr_register_thread();
#endif /*HAVE_LIBURCU_QSBR*/

    while (!force_quit && !force_stop[lcore_id]) {
#if HAVE_LIBURCU_QSBR
        urcu_qsbr_read_lock();
        rib = (struct rib *) rcu_dereference(rcu_global_ptr_rib);
#endif /*HAVE_LIBURCU_QSBR*/

        // 패킷 처리 로직을 여기에 작성
        // rib->rib_info->port[portid]를 통해 포트 정보 접근
        
#if HAVE_LIBURCU_QSBR
        urcu_qsbr_read_unlock();
        urcu_qsbr_quiescent_state();
#endif /*HAVE_LIBURCU_QSBR*/

        loop_counter++;
    }

#if HAVE_LIBURCU_QSBR
    urcu_qsbr_unregister_thread();
#endif /*HAVE_LIBURCU_QSBR*/
    
    return 0;
}
```

#### 포트 정보 접근

RIB를 획득한 후, 포트별 정보에 접근:

```c
// 포트 링크 상태 확인
if (!rib->rib_info->port[portid].link.link_status) {
    // 포트가 다운됨, 처리 건너뛰기
    continue;
}

// 포트가 중지되었는지 확인
if (unlikely(rib->rib_info->port[portid].is_stopped)) {
    // 포트가 관리적으로 중지됨
    continue;
}

// 포트 설정 접근
struct port_config *port_config = &rib->rib_info->port[portid];

// lcore 큐 설정 획득
struct lcore_qconf *lcore_qconf = &rib->rib_info->lcore_qconf[lcore_id];
for (i = 0; i < lcore_qconf->nrxq; i++) {
    portid = lcore_qconf->rx_queue_list[i].port_id;
    queueid = lcore_qconf->rx_queue_list[i].queue_id;
    // 이 포트/큐에서 패킷 처리
}
```

#### RCU 안전성 가이드라인

- **스레드 등록**: 항상 `urcu_qsbr_register_thread()`로 스레드 등록
- **읽기 락**: RIB 데이터 접근 전 읽기 락 획득
- **역참조**: RCU 보호 포인터에 안전하게 접근하기 위해 `rcu_dereference()` 사용
- **정지 상태**: 안전한 지점을 나타내기 위해 `urcu_qsbr_quiescent_state()` 호출
- **스레드 정리**: `urcu_qsbr_unregister_thread()`로 스레드 등록 해제

#### RIB 데이터 구조

RIB를 통해 사용 가능한 주요 정보:
- **포트 정보**: 링크 상태, 설정, 통계
- **큐 설정**: lcore에서 포트/큐로의 할당
- **VLAN 설정**: 가상 스위치 및 VLAN 설정 (고급 기능용)
- **인터페이스 설정**: TAP 인터페이스 및 라우팅 정보

### 4. CLI 명령어 추가
sdplane의 CLI 시스템에 애플리케이션별 명령어 등록:

```c
CLI_COMMAND2(my_app_command,
             "my-app command <WORD>",
             "my application\n",
             "command help\n")
{
    // 명령어 구현
    return 0;
}

// 초기화 함수 내
INSTALL_COMMAND2(cmdset, my_app_command);
```

### 4. 빌드 시스템 설정
모듈을 포함하도록 빌드 설정 업데이트:

```makefile
# sdplane/Makefile.am에 추가
sdplane_SOURCES += my_worker.c my_worker.h
```

### 5. 통합 테스트
sdplane 설정을 사용하여 기능 검증:

```bash
# sdplane 설정 내
set worker lcore 1 my-worker-type
set thread 1 port 0 queue 0

# 워커 시작
start worker lcore all
```

## 통합된 애플리케이션

다음 DPDK 애플리케이션들이 DPDK-dock 방식을 사용하여 sdplane 통합에 성공했습니다:

### L2FWD 통합 (`module/l2fwd/`)
**상태**: ✅ 통합 성공
- 원본 DPDK l2fwd 애플리케이션을 sdplane용으로 적용
- 포트 간 기본 패킷 전달 시연
- **참고자료**: 원본 DPDK l2fwd 소스코드용 패치 파일 사용 가능
- pthread-to-lcore 워커 변환 시연
- 주요 파일: `module/l2fwd/l2fwd.c`, 통합 패치

### PKTGEN 통합 (`module/pktgen/`)
**상태**: ✅ 통합 성공
- 완전한 DPDK PKTGEN 애플리케이션 통합
- 복잡한 멀티스레드 애플리케이션 예제
- sdplane CLI와의 외부 라이브러리 통합
- 고급 통합 기법 시연
- 주요 파일: `module/pktgen/app/`, `sdplane/pktgen_cmd.c`

### L3FWD 통합
**상태**: ⚠️ 부분 통합 (완전 기능하지 않음)
- 통합을 시도했으나 추가 작업 필요
- 새로운 통합의 참고로는 권장되지 않음
- L2FWD와 PKTGEN을 주요 예제로 사용

## 권장 통합 참고자료

새로운 DPDK 애플리케이션을 통합하는 개발자들에게는 다음을 주요 참고자료로 사용할 것을 권장합니다:

### 1. L2FWD 통합 (권장)
- **위치**: `module/l2fwd/`
- **패치 파일**: 원본 DPDK l2fwd 소스용 사용 가능
- **적용 범위**: 기본 pthread-to-lcore 변환 이해
- **통합 패턴**: 간단한 패킷 전달 워커

### 2. PKTGEN 통합 (고급 참고)
- **위치**: `module/pktgen/`과 `sdplane/pktgen_cmd.c`
- **적용 범위**: CLI 명령어를 가진 복잡한 애플리케이션 통합
- **통합 패턴**: 외부 의존성을 가진 멀티컴포넌트 애플리케이션

### 통합 패치 파일

L2FWD 통합에 대해, 원본 DPDK l2fwd 애플리케이션을 sdplane 통합용으로 적용하는 데 필요한 정확한 수정사항을 보여주는 패치 파일이 사용 가능합니다. 이러한 패치들은 다음을 시연합니다:
- EAL 초기화 제거
- 스레딩 모델 적용
- CLI 명령어 통합
- 리소스 관리 변경

개발자들은 DPDK 애플리케이션 통합에 대한 체계적인 접근 방법을 이해하기 위해 이러한 패치 파일을 참조해야 합니다.

## 커스텀 워커 예제

### L2 리피터 (`sdplane/l2_repeater.c`)
커스텀 sdplane 워커 (기존 DPDK 앱 기반이 아님):
- 간단한 패킷 전달 워커
- 모든 활성 포트로의 브로드캐스트 전달
- sdplane의 포트 관리와 통합

### 향상된 리피터 (`sdplane/enhanced_repeater.c`)
고급 기능을 가진 커스텀 sdplane 워커:
- VLAN 지원 스위칭
- TAP 인터페이스 통합
- 포트 상태를 위한 RIB 통합

### VLAN 스위치 (`sdplane/vlan_switch.c`)
커스텀 레이어 2 스위칭 구현:
- MAC 학습 및 전달
- VLAN 처리
- 멀티포트 패킷 처리

## 모범 사례

### 성능 고려사항
- 최적 성능을 위해 버스트 처리 사용
- 가능한 한 패킷 복사 최소화
- DPDK의 제로 복사 메커니즘 활용
- 워커 할당에서 NUMA 토폴로지 고려

### 오류 처리
- DPDK 함수의 반환값 확인
- 적절한 종료 처리 구현
- 적절한 로그 레벨 사용
- 리소스 정리를 적절히 처리

### 스레딩 모델
- sdplane의 lcore 기반 스레딩 이해
- 효율적인 패킷 처리 루프 설계
- 적절한 동기화 메커니즘 사용
- 스레드 친화성과 CPU 격리 고려

## 디버깅 및 모니터링

### 디버그 로그
워커의 디버그 로그 활성화:

```bash
debug sdplane my-worker-type
```

### 통계 수집
sdplane의 통계 프레임워크와 통합:

```c
// 포트 통계 업데이트
port_statistics[portid].rx += nb_rx;
port_statistics[portid].tx += nb_tx;
```

### CLI 모니터링
모니터링용 상태 명령어 제공:

```bash
show my-worker status
show my-worker statistics
```

## 일반적인 통합 패턴

### 패킷 처리 파이프라인
```c
// sdplane 워커에서 일반적인 패킷 처리
while (!force_quit && !force_stop[lcore_id]) {
    // 1. 패킷 수신
    nb_rx = rte_eth_rx_burst(portid, queueid, pkts_burst, MAX_PKT_BURST);
    
    // 2. 패킷 처리
    for (i = 0; i < nb_rx; i++) {
        // 처리 로직
        process_packet(pkts_burst[i]);
    }
    
    // 3. 패킷 전송
    rte_eth_tx_burst(dst_port, queueid, pkts_burst, nb_rx);
}
```

### 설정 통합
```c
// sdplane의 설정 시스템에 등록
struct worker_config my_worker_config = {
    .name = "my-worker",
    .worker_func = my_worker_function,
    .init_func = my_worker_init,
    .cleanup_func = my_worker_cleanup
};
```

## 문제 해결

### 일반적인 문제
- **워커가 시작하지 않음**: lcore 할당과 설정 확인
- **패킷 드롭**: 큐 설정과 버퍼 크기 확인
- **성능 문제**: CPU 친화성과 NUMA 설정 확인
- **CLI 명령어가 작동하지 않음**: 적절한 명령어 등록 확인

### 디버그 기법
- sdplane의 디버그 로그 시스템 사용
- 워커 통계와 카운터 모니터링
- 포트 링크 상태와 설정 확인
- 메모리 풀 할당 검증

자세한 구현 예제는 코드베이스 내의 기존 워커 모듈과 해당 CLI 명령어 정의를 참조하세요.