# VTY·셸 관리

**언어 / Language:** [English](../vty-shell.md) | [日本語](../ja/vty-shell.md) | [Français](../fr/vty-shell.md) | [中文](../zh/vty-shell.md) | [Deutsch](../de/vty-shell.md) | [Italiano](../it/vty-shell.md) | **한국어**

VTY(Virtual Terminal)와 셸 관리를 수행하는 명령어입니다.

## 명령어 목록

### clear_cmd - 화면 지우기
```
clear
```

VTY 화면을 지웁니다.

**사용 예:**
```bash
clear
```

### vty_exit_cmd - VTY 종료
```
(exit|quit|logout)
```

VTY 세션을 종료합니다. 여러 별칭을 사용할 수 있습니다.

**사용 예:**
```bash
exit
# 또는
quit
# 또는
logout
```

### shutdown_cmd - 시스템 종료
```
shutdown
```

sdplane 시스템을 종료합니다.

**사용 예:**
```bash
shutdown
```

**주의:** 이 명령어는 시스템 전체를 중지합니다. 실행 전에 설정 저장과 활성 세션 종료를 확인하세요.

### exit_cmd - 콘솔 종료
```
(exit|quit)
```

콘솔 셸을 종료합니다.

**사용 예:**
```bash
exit
# 또는
quit
```

## VTY와 콘솔의 차이

### VTY 셸
- Telnet을 통해 접근하는 원격 셸
- 여러 세션을 동시에 사용 가능
- 네트워크를 통한 접근 가능

### 콘솔 셸
- 로컬 콘솔에서의 접근
- 직접적인 시스템 접근
- 주로 로컬 관리용

## 연결 방법

### VTY 연결
```bash
telnet localhost 9882
```

### 콘솔 연결
```bash
# sdplane을 직접 실행
sudo ./sdplane/sdplane
```

## 세션 관리

### 세션 확인
VTY 세션 상태는 다음 명령어로 확인할 수 있습니다:
```bash
show worker
```

### 세션 종료
- `exit`, `quit`, `logout` 명령어로 정상 종료
- 비정상 종료 시 `shutdown` 명령어로 시스템 전체 중지

## 보안 고려사항

### 접근 제어
- VTY는 기본적으로 localhost(127.0.0.1)에서의 접근만 허용
- 방화벽 설정으로 포트 9882의 접근을 제한하는 것을 권장

### 세션 모니터링
- 불필요한 세션은 적절히 종료
- 장시간 유휴 세션은 정기적으로 확인

## 문제 해결

### VTY에 연결할 수 없는 경우
1. sdplane이 정상적으로 시작되었는지 확인
2. 포트 9882가 사용 가능한지 확인
3. 방화벽 설정 확인

### 세션이 응답하지 않는 경우
1. 다른 VTY 세션에서 `show worker`로 상태 확인
2. 필요시 `shutdown` 명령어로 시스템 재시작

### 명령어가 인식되지 않는 경우
1. 올바른 셸(VTY 또는 콘솔)에 있는지 확인
2. `?`로 도움말을 표시하여 사용 가능한 명령어 확인

## 정의 위치

이러한 명령어는 다음 파일에 정의되어 있습니다:
- `sdplane/vty_shell.c` - VTY 관련 명령어
- `sdplane/console_shell.c` - 콘솔 관련 명령어

## 관련 항목

- [디버그·로그](debug-logging.md)
- [시스템 정보·모니터링](system-monitoring.md)