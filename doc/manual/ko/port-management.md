# 포트 관리·통계

**언어 / Language:** [English](../en/port-management.md) | [日本語](../ja/port-management.md) | [Français](../fr/port-management.md) | [中文](../zh/port-management.md) | [Deutsch](../de/port-management.md) | [Italiano](../it/port-management.md) | **한국어**

DPDK 포트 관리 및 통계 정보를 다루는 명령어입니다.

## 명령어 목록

### **start port**

DPDK 포트를 시작합니다 (기본 동작).

**사용 예:**
```bash
# 포트 시작 (기본)
start port
```

---

### **start port \<0-16\>**

특정 DPDK 포트를 시작합니다.

**사용 예:**
```bash
# 포트 0을 시작
start port 0

# 포트 1을 시작
start port 1
```

---

### **start port all**

모든 DPDK 포트를 시작합니다.

**사용 예:**
```bash
# 모든 포트를 시작
start port all
```

---

### **stop port**

DPDK 포트를 중지합니다 (기본 동작).

**사용 예:**
```bash
# 포트 중지 (기본)
stop port
```

---

### **stop port \<0-16\>**

특정 DPDK 포트를 중지합니다.

**사용 예:**
```bash
# 포트 0을 중지
stop port 0

# 포트 1을 중지
stop port 1
```

---

### **stop port all**

모든 DPDK 포트를 중지합니다.

**사용 예:**
```bash
# 모든 포트를 중지
stop port all
```

---

### **reset port**

DPDK 포트를 재설정합니다 (기본 동작).

**사용 예:**
```bash
# 포트 재설정 (기본)
reset port
```

---

### **reset port \<0-16\>**

특정 DPDK 포트를 재설정합니다.

**사용 예:**
```bash
# 포트 0을 재설정
reset port 0

# 포트 1을 재설정
reset port 1
```

---

### **reset port all**

모든 DPDK 포트를 재설정합니다.

**사용 예:**
```bash
# 모든 포트를 재설정
reset port all
```

---

### **show port**

모든 포트의 기본 정보를 표시합니다 (기본 동작).

**사용 예:**
```bash
# 모든 포트 정보 표시
show port
```

---

### **show port \<0-16\>**

특정 포트의 기본 정보를 표시합니다.

**사용 예:**
```bash
# 포트 0의 정보 표시
show port 0

# 포트 1의 정보 표시
show port 1
```

---

### **show port all**

모든 포트의 정보를 명시적으로 표시합니다.

**사용 예:**
```bash
# 모든 포트 정보를 명시적으로 표시
show port all
```

---

### **show port statistics**

모든 포트 통계 정보를 표시합니다.

**사용 예:**
```bash
# 모든 통계 정보 표시
show port statistics
```

---

### **show port statistics pps**

패킷/초 통계를 표시합니다.

**사용 예:**
```bash
# PPS 통계만 표시
show port statistics pps
```

---

### **show port statistics total**

총 패킷 수 통계를 표시합니다.

**사용 예:**
```bash
# 총 패킷 수 표시
show port statistics total
```

---

### **show port statistics bps**

비트/초 통계를 표시합니다.

**사용 예:**
```bash
# 비트/초 표시
show port statistics bps
```

---

### **show port statistics Bps**

바이트/초 통계를 표시합니다.

**사용 예:**
```bash
# 바이트/초 표시
show port statistics Bps
```

---

### **show port statistics total-bytes**

총 바이트 수 통계를 표시합니다.

**사용 예:**
```bash
# 총 바이트 수 표시
show port statistics total-bytes
```

---

### **show port \<0-16\> promiscuous**

지정된 포트의 프로미스큐어스 모드 상태를 표시합니다.

**사용 예:**
```bash
# 포트 0의 프로미스큐어스 모드 표시
show port 0 promiscuous

# 포트 1의 프로미스큐어스 모드 표시
show port 1 promiscuous
```

---

### **show port all promiscuous**

모든 포트의 프로미스큐어스 모드 상태를 표시합니다.

**사용 예:**
```bash
# 모든 포트의 프로미스큐어스 모드 표시
show port all promiscuous
```

---

### **show port \<0-16\> flowcontrol**

지정된 포트의 흐름 제어 설정을 표시합니다.

**사용 예:**
```bash
# 포트 0의 흐름 제어 설정 표시
show port 0 flowcontrol

# 포트 1의 흐름 제어 설정 표시
show port 1 flowcontrol
```

---

### **show port all flowcontrol**

모든 포트의 흐름 제어 설정을 표시합니다.

**사용 예:**
```bash
# 모든 포트의 흐름 제어 설정 표시
show port all flowcontrol
```

---

### **set port \<0-16\> promiscuous enable**

지정된 포트의 프로미스큐어스 모드를 활성화합니다.

**사용 예:**
```bash
# 포트 0의 프로미스큐어스 모드 활성화
set port 0 promiscuous enable

# 포트 1의 프로미스큐어스 모드 활성화
set port 1 promiscuous enable
```

---

### **set port \<0-16\> promiscuous disable**

지정된 포트의 프로미스큐어스 모드를 비활성화합니다.

**사용 예:**
```bash
# 포트 0의 프로미스큐어스 모드 비활성화
set port 0 promiscuous disable

# 포트 1의 프로미스큐어스 모드 비활성화
set port 1 promiscuous disable
```

---

### **set port all promiscuous enable**

모든 포트의 프로미스큐어스 모드를 활성화합니다.

**사용 예:**
```bash
# 모든 포트의 프로미스큐어스 모드 활성화
set port all promiscuous enable
```

---

### **set port all promiscuous disable**

모든 포트의 프로미스큐어스 모드를 비활성화합니다.

**사용 예:**
```bash
# 모든 포트의 프로미스큐어스 모드 비활성화
set port all promiscuous disable
```

---

### **set port \<0-16\> flowcontrol (rx|tx|autoneg|send-xon|fwd-mac-ctrl) (on|off)**

지정된 포트의 흐름 제어 설정을 변경합니다.

**옵션:**
- `rx` - 수신 흐름 제어
- `tx` - 송신 흐름 제어
- `autoneg` - 자동 협상
- `send-xon` - XON 송신
- `fwd-mac-ctrl` - MAC 제어 프레임 전달

**사용 예:**
```bash
# 포트 0의 수신 흐름 제어 활성화
set port 0 flowcontrol rx on

# 포트 1의 자동 협상 비활성화
set port 1 flowcontrol autoneg off
```

---

### **set port all flowcontrol (rx|tx|autoneg|send-xon|fwd-mac-ctrl) (on|off)**

모든 포트의 흐름 제어 설정을 변경합니다.

**옵션:**
- `rx` - 수신 흐름 제어
- `tx` - 송신 흐름 제어
- `autoneg` - 자동 협상
- `send-xon` - XON 송신
- `fwd-mac-ctrl` - MAC 제어 프레임 전달

**사용 예:**
```bash
# 모든 포트의 자동 협상 비활성화
set port all flowcontrol autoneg off

# 모든 포트의 송신 흐름 제어 활성화
set port all flowcontrol tx on
```

---

### **set port \<0-16\> dev-configure \<0-64\> \<0-64\>**

지정된 포트의 DPDK 장치 설정을 수행합니다.

**매개변수:**
- 첫 번째 인수: 수신 큐 수 (0-64)
- 두 번째 인수: 송신 큐 수 (0-64)

**사용 예:**
```bash
# 포트 0을 수신 큐 4, 송신 큐 4로 설정
set port 0 dev-configure 4 4

# 포트 1을 수신 큐 2, 송신 큐 2로 설정
set port 1 dev-configure 2 2
```

---

### **set port all dev-configure \<0-64\> \<0-64\>**

모든 포트의 DPDK 장치 설정을 수행합니다.

**매개변수:**
- 첫 번째 인수: 수신 큐 수 (0-64)
- 두 번째 인수: 송신 큐 수 (0-64)

**사용 예:**
```bash
# 모든 포트를 수신 큐 1, 송신 큐 1로 설정
set port all dev-configure 1 1
```

---

### **set port \<0-16\> nrxdesc \<0-16384\>**

지정한 포트의 수신 디스크립터 수를 설정합니다.

**사용 예:**
```bash
# 포트 0의 수신 디스크립터 수를 1024로 설정
set port 0 nrxdesc 1024

# 포트 1의 수신 디스크립터 수를 512로 설정
set port 1 nrxdesc 512
```

---

### **set port all nrxdesc \<0-16384\>**

모든 포트의 수신 디스크립터 수를 설정합니다.

**사용 예:**
```bash
# 모든 포트의 수신 디스크립터 수를 512로 설정
set port all nrxdesc 512
```

---

### **set port \<0-16\> ntxdesc \<0-16384\>**

지정한 포트의 송신 디스크립터 수를 설정합니다.

**사용 예:**
```bash
# 포트 0의 송신 디스크립터 수를 1024로 설정
set port 0 ntxdesc 1024

# 포트 1의 송신 디스크립터 수를 512로 설정
set port 1 ntxdesc 512
```

---

### **set port all ntxdesc \<0-16384\>**

모든 포트의 송신 디스크립터 수를 설정합니다.

**사용 예:**
```bash
# 모든 포트의 송신 디스크립터 수를 512로 설정
set port all ntxdesc 512
```

---

### **set port \<0-16\> link up**

지정된 포트의 링크를 활성화합니다.

**사용 예:**
```bash
# 포트 0의 링크를 활성화
set port 0 link up

# 포트 1의 링크를 활성화
set port 1 link up
```

---

### **set port \<0-16\> link down**

지정된 포트의 링크를 비활성화합니다.

**사용 예:**
```bash
# 포트 0의 링크를 비활성화
set port 0 link down

# 포트 1의 링크를 비활성화
set port 1 link down
```

---

### **set port all link up**

모든 포트의 링크를 활성화합니다.

**사용 예:**
```bash
# 모든 포트의 링크를 활성화
set port all link up
```

---

### **set port all link down**

모든 포트의 링크를 비활성화합니다.

**사용 예:**
```bash
# 모든 포트의 링크를 비활성화
set port all link down
```

## 정의 위치

이러한 명령어는 다음 파일에 정의되어 있습니다:
- `sdplane/dpdk_port_cmd.c`

## 관련 항목

- [워커·lcore 관리](worker-lcore-thread-management.md)
- [시스템 정보·모니터링](system-monitoring.md)
- [큐 설정](queue-configuration.md)