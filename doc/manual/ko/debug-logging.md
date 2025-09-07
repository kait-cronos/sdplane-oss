# 디버그 및 로깅

**Language:** [English](../en/debug-logging.md) | [日本語](../ja/debug-logging.md) | [Français](../fr/debug-logging.md) | [中文](../zh/debug-logging.md) | [Deutsch](../de/debug-logging.md) | [Italiano](../it/debug-logging.md) | [한국어](../ko/debug-logging.md) | [ไทย](../th/debug-logging.md) | [Español](../es/debug-logging.md)

sdplane 디버그 및 로깅 기능을 제어하는 명령입니다.

## 명령 목록

### log_file - 로그 파일 출력 구성
```
log file <file-path>
```

파일로 로깅 출력을 구성합니다.

**매개변수:**
- <파일-경로> - 로그 출력 파일 경로

**예제:**
```bash
# 지정된 파일로 로그 출력
log file /var/log/sdplane.log

# 디버그 로그 파일
log file /tmp/sdplane-debug.log
```

### log_stdout - 표준 출력 로그 구성
```
log stdout
```

표준 출력으로 로깅을 구성합니다.

**예제:**
```bash
# 표준 출력에 로그 표시
log stdout
```

**참고:** `log file`과 `log stdout`을 동시에 구성할 수 있으며, 로그가 두 대상 모두에 출력됩니다.

### debug - 디버그 구성
```
debug <category> <target>
```

지정된 카테고리 내의 특정 대상에 대한 디버그 로깅을 활성화합니다.

**카테고리:**
- `sdplane` - 메인 sdplane 카테고리
- `zcmdsh` - 명령 셸 카테고리

**sdplane 대상 목록:**
- `lthread` - 경량 스레드
- `console` - 콘솔
- `tap-handler` - TAP 핸들러
- `l2fwd` - L2 전달
- `l3fwd` - L3 전달
- `vty-server` - VTY 서버
- `vty-shell` - VTY 셸
- `stat-collector` - 통계 수집기
- `packet` - 패킷 처리
- `fdb` - FDB (전달 데이터베이스)
- `fdb-change` - FDB 변경
- `rib` - RIB (라우팅 정보 베이스)
- `vswitch` - 가상 스위치
- `vlan-switch` - VLAN 스위치
- `pktgen` - 패킷 생성기
- `enhanced-repeater` - 향상된 리피터
- `netlink` - Netlink 인터페이스
- `neighbor` - 이웃 관리
- `all` - 모든 대상

**예제:**
```bash
# 특정 대상에 대한 디버그 활성화
debug sdplane rib
debug sdplane fdb-change
debug sdplane pktgen

# 모든 sdplane 디버그 활성화
debug sdplane all

# zcmdsh 카테고리 디버그
debug zcmdsh shell
debug zcmdsh command
```

### no debug - 디버그 비활성화
```
no debug <category> <target>
```

지정된 카테고리 내의 특정 대상에 대한 디버그 로깅을 비활성화합니다.

**예제:**
```bash
# 특정 대상에 대한 디버그 비활성화
no debug sdplane rib
no debug sdplane fdb-change

# 모든 sdplane 디버그 비활성화 (권장)
no debug sdplane all

# zcmdsh 카테고리 디버그 비활성화
no debug zcmdsh all
```

### show_debug_sdplane - Display sdplane Debug Information
```
show debugging sdplane
```

Display current sdplane debug configuration.

**Examples:**
```bash
show debugging sdplane
```

This command displays the following information:
- Currently enabled debug targets
- Debug status for each target
- Available debug options

## Debug System Overview

The sdplane debug system has the following features:

### Category-based Debugging
- Debug categories are separated by different functional modules
- You can enable debug logs only for the necessary functions

### Target-based Control
- Debug messages are classified by target type
- You can display only necessary information by setting appropriate targets

### Dynamic Configuration
- Debug configuration can be changed while the system is running
- Debug targets can be adjusted without restart

## Usage

### 1. Configure Log Output
```bash
# Configure log file output (recommended)
log file /var/log/sdplane.log

# Configure standard output
log stdout

# Enable both (convenient for debugging)
log file /var/log/sdplane.log
log stdout
```

### 2. Check Current Debug Configuration
```bash
show debugging sdplane
```

### 3. Check Debug Targets
Use the `show debugging sdplane` command to check available targets and their status.

### 4. Change Debug Configuration
```bash
# Enable debug for specific targets
debug sdplane rib
debug sdplane fdb-change

# Enable all targets at once
debug sdplane all
```

### 5. Check Debug Logs
Debug logs are output to the configured destinations (file or standard output).

## Troubleshooting

### When Debug Logs Are Not Output
1. Check if log output is configured (`log file` or `log stdout`)
2. Check if debug targets are correctly configured (`debug sdplane <target>`)
3. Check current debug status (`show debugging sdplane`)
4. Check log file disk space and permissions

### Log File Management
```bash
# Check log file size
ls -lh /var/log/sdplane.log

# Tail log file
tail -f /var/log/sdplane.log

# Check log file location (configuration file example)
grep "log file" /etc/sdplane/sdplane.conf
```

### Performance Impact
- Enabling debug logs may impact performance
- It is recommended to enable only minimal debugging in production environments
- Regularly rotate log files to prevent them from becoming too large

## Configuration Examples

### Basic Log Configuration
```bash
# Configuration file example (/etc/sdplane/sdplane.conf)
log file /var/log/sdplane.log
log stdout

# Enable debug at system startup
debug sdplane rib
debug sdplane fdb-change
```

### Debugging Configuration
```bash
# Detailed debug log configuration
log file /tmp/sdplane-debug.log
log stdout

# Enable all target debugging (development only)
debug sdplane all

# Enable specific targets only
debug sdplane rib
debug sdplane fdb-change
debug sdplane vswitch
```

### Production Environment Configuration
```bash
# Standard logging only in production
log file /var/log/sdplane.log

# Enable only critical targets as needed
# debug sdplane fdb-change
# debug sdplane rib
```

### Debug Cleanup Operations
```bash
# Disable all debugging
no debug sdplane all
no debug zcmdsh all
```

## Definition Location

These commands are defined in the following file:
- `sdplane/debug_sdplane.c`

## Related Topics

- [System Information & Monitoring](system-monitoring.md)
- [VTY & Shell Management](vty-shell.md)
