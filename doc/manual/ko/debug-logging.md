# 디버그·로그

**언어 / Language:** [English](../debug-logging.md) | [日本語](../ja/debug-logging.md) | [Français](../fr/debug-logging.md) | [中文](../zh/debug-logging.md) | [Deutsch](../de/debug-logging.md) | [Italiano](../it/debug-logging.md) | **한국어**

sdplane의 디버그 및 로그 기능을 제어하는 명령어입니다.

## 명령어 목록

### debug_sdplane - sdplane 디버그 설정
```
debug sdplane [카테고리] [레벨]
```

sdplane의 디버그 로그를 설정합니다. 이 명령어는 동적으로 생성되므로 사용 가능한 카테고리와 레벨은 런타임에서 결정됩니다.

**사용 예:**
```bash
# 디버그 설정 활성화
debug sdplane

# 특정 카테고리의 디버그 활성화
debug sdplane [category] [level]
```

**주의:** 구체적인 카테고리와 레벨은 `show debugging sdplane` 명령어로 확인할 수 있습니다.

### show_debug_sdplane - sdplane 디버그 정보 표시
```
show debugging sdplane
```

현재 sdplane 디버그 설정을 표시합니다.

**사용 예:**
```bash
show debugging sdplane
```

이 명령어는 다음 정보를 표시합니다:
- 현재 활성화된 디버그 카테고리
- 각 카테고리의 디버그 레벨
- 사용 가능한 디버그 옵션

## 디버그 시스템 개요

sdplane의 디버그 시스템은 다음과 같은 특징을 가지고 있습니다:

### 카테고리 기반 디버그
- 다른 기능 모듈별로 디버그 카테고리가 분리되어 있습니다
- 필요한 기능만 디버그 로그를 활성화할 수 있습니다

### 레벨 기반 제어
- 디버그 메시지는 중요도에 따라 레벨별로 구분됩니다
- 적절한 레벨을 설정하여 필요한 정보만 표시할 수 있습니다

### 동적 설정
- 시스템 운영 중에 디버그 설정을 변경할 수 있습니다
- 재시작 없이 디버그 레벨을 조정할 수 있습니다

## 사용 방법

### 1. 현재 디버그 설정 확인
```bash
show debugging sdplane
```

### 2. 디버그 카테고리 확인
`show debugging sdplane` 명령어로 사용 가능한 카테고리를 확인하세요.

### 3. 디버그 설정 변경
```bash
# 특정 카테고리의 디버그 활성화
debug sdplane [category] [level]
```

### 4. 디버그 로그 확인
디버그 로그는 표준 출력 또는 로그 파일에 출력됩니다.

## 문제 해결

### 디버그 로그가 출력되지 않는 경우
1. 디버그 카테고리가 올바르게 설정되었는지 확인
2. 디버그 레벨이 적절하게 설정되었는지 확인
3. 로그 출력 대상이 올바르게 설정되었는지 확인

### 성능에 미치는 영향
- 디버그 로그 활성화는 성능에 영향을 줄 수 있습니다
- 운영 환경에서는 필요 최소한의 디버그만 활성화하는 것을 권장합니다

## 정의 위치

이러한 명령어는 다음 파일에 정의되어 있습니다:
- `sdplane/debug_sdplane.c`

## 관련 항목

- [시스템 정보·모니터링](system-monitoring.md)
- [VTY·셸 관리](vty-shell.md)