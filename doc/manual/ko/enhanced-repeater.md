# 향상된 리피터 설정

향상된 리피터는 L3 라우팅용과 패킷 캡처용의 TAP 인터페이스를 갖춘 고급 VLAN 스위칭 기능을 제공합니다. 가상 스위치 생성, VLAN 태깅을 사용한 DPDK 포트 링크, 커널 통합용 라우터 인터페이스, 모니터링용 캡처 인터페이스를 지원합니다.

## 가상 스위치 명령어

### set vswitch
```
set vswitch <1-4094>
```
**설명**: 지정된 VLAN ID로 가상 스위치를 생성합니다

**매개변수**:
- `<1-4094>`: 가상 스위치의 VLAN ID

**예제**:
```
set vswitch 2031
set vswitch 2032
```

### delete vswitch
```
delete vswitch <0-3>
```
**설명**: ID로 지정한 가상 스위치를 삭제합니다

**매개변수**:
- `<0-3>`: 삭제할 가상 스위치 ID

**예제**:
```
delete vswitch 0
```

### show vswitch_rib
```
show vswitch_rib
```
**설명**: 설정과 상태를 포함한 가상 스위치 RIB 정보를 표시합니다

## 가상 스위치 링크 명령어

### set vswitch-link
```
set vswitch-link vswitch <0-3> port <0-7> tag <0-4094>
```
**설명**: DPDK 포트를 가상 스위치에 VLAN 태깅 설정으로 링크합니다

**매개변수**:
- `vswitch <0-3>`: 가상 스위치 ID (0-3)
- `port <0-7>`: DPDK 포트 ID (0-7)  
- `tag <0-4094>`: VLAN 태그 ID (0: 네이티브/언태그, 1-4094: 태그된 VLAN)

**예제**:
```
# 포트 0을 가상 스위치 0에 VLAN 태그 2031로 링크
set vswitch-link vswitch 0 port 0 tag 2031

# 포트 0을 가상 스위치 1에 네이티브/언태그로 링크
set vswitch-link vswitch 1 port 0 tag 0
```

### delete vswitch-link
```
delete vswitch-link <0-7>
```
**설명**: ID로 지정한 가상 스위치 링크를 삭제합니다

**매개변수**:
- `<0-7>`: 가상 스위치 링크 ID

### show vswitch-link
```
show vswitch-link
```
**설명**: 모든 가상 스위치 링크 설정을 표시합니다

## 라우터 인터페이스 명령어

### set vswitch router-if
```
set vswitch <1-4094> router-if <WORD>
```
**설명**: 지정된 가상 스위치에 L3 연결용 라우터 인터페이스를 생성합니다

**매개변수**:
- `<1-4094>`: 가상 스위치의 VLAN ID
- `<WORD>`: TAP 인터페이스명

**예제**:
```
set vswitch 2031 router-if rif2031
set vswitch 2032 router-if rif2032
```

### no set vswitch router-if
```
no set vswitch <1-4094> router-if
```
**설명**: 지정된 가상 스위치에서 라우터 인터페이스를 제거합니다

**매개변수**:
- `<1-4094>`: 가상 스위치의 VLAN ID

### show rib vswitch router-if
```
show rib vswitch router-if
```
**설명**: MAC 주소, IP 주소, 인터페이스 상태를 포함한 라우터 인터페이스 설정을 표시합니다

## 캡처 인터페이스 명령어

### set vswitch capture-if
```
set vswitch <1-4094> capture-if <WORD>
```
**설명**: 지정된 가상 스위치에 패킷 모니터링용 캡처 인터페이스를 생성합니다

**매개변수**:
- `<1-4094>`: 가상 스위치의 VLAN ID
- `<WORD>`: TAP 인터페이스명

**예제**:
```
set vswitch 2031 capture-if cif2031
set vswitch 2032 capture-if cif2032
```

### no set vswitch capture-if
```
no set vswitch <1-4094> capture-if
```
**설명**: 지정된 가상 스위치에서 캡처 인터페이스를 제거합니다

**매개변수**:
- `<1-4094>`: 가상 스위치의 VLAN ID

### show rib vswitch capture-if
```
show rib vswitch capture-if
```
**설명**: 캡처 인터페이스 설정을 표시합니다

## VLAN 처리 기능

향상된 리피터는 고급 VLAN 처리를 수행합니다:

- **VLAN 변환**: vswitch-link 설정에 기반하여 VLAN ID 변경
- **VLAN 제거**: 태그가 0(네이티브)으로 설정된 경우 VLAN 헤더 제거  
- **VLAN 삽입**: 언태그 패킷을 태그된 포트로 전달할 때 VLAN 헤더 추가
- **스플릿 호라이즌**: 수신 포트로 패킷을 되돌려 보내지 않아 루프 방지

## 설정 예제

```bash
# 가상 스위치 생성
set vswitch 2031
set vswitch 2032

# DPDK 포트를 VLAN 태그와 함께 가상 스위치에 링크
set vswitch-link vswitch 0 port 0 tag 2031
set vswitch-link vswitch 1 port 0 tag 2032

# L3 처리용 라우터 인터페이스 생성
set vswitch 2031 router-if rif2031
set vswitch 2032 router-if rif2032

# 모니터링용 캡처 인터페이스 생성
set vswitch 2031 capture-if cif2031
set vswitch 2032 capture-if cif2032

# 향상된 리피터를 사용하도록 워커 설정
set worker lcore 1 enhanced-repeater
```

## TAP 인터페이스와의 통합

라우터 인터페이스와 캡처 인터페이스는 Linux 커널 네트워크 스택과 통합되는 TAP 인터페이스를 생성합니다:

- **라우터 인터페이스**: L3 라우팅, IP 주소 지정, 커널 네트워크 처리 가능
- **캡처 인터페이스**: 패킷 모니터링, 분석, 디버깅 활성화
- **링 버퍼**: 데이터 플레인과 커널 간의 효율적인 패킷 전송에 DPDK 링 사용