# 调试·日志

**语言 / Language:** [English](../debug-logging.md) | [日本語](../ja/debug-logging.md) | [Français](../fr/debug-logging.md) | **中文** | [Deutsch](../de/debug-logging.md) | [Italiano](../it/debug-logging.md) | [한국어](../ko/debug-logging.md) | [ไทย](../th/debug-logging.md) | [Español](../es/debug-logging.md)

控制sdplane调试和日志功能的命令。

## 命令一览

### debug_sdplane - sdplane调试设置
```
debug sdplane [类别] [级别]
```

设置sdplane的调试日志。该命令是动态生成的，因此可用的类别和级别在运行时确定。

**使用例：**
```bash
# 启用调试设置
debug sdplane

# 启用特定类别的调试
debug sdplane [category] [level]
```

**注意：** 具体的类别和级别可以通过 `show debugging sdplane` 命令确认。

### show_debug_sdplane - sdplane调试信息显示
```
show debugging sdplane
```

显示当前的sdplane调试设置。

**使用例：**
```bash
show debugging sdplane
```

该命令显示以下信息：
- 当前启用的调试类别
- 各类别的调试级别
- 可用的调试选项

## 调试系统概要

sdplane的调试系统具有以下特点：

### 基于类别的调试
- 不同功能模块按调试类别分开
- 可以仅启用必要功能的调试日志

### 基于级别的控制
- 调试消息根据重要性分为不同级别
- 通过设置适当的级别，可以仅显示必要的信息

### 动态设置
- 可以在系统运行中更改调试设置
- 无需重启即可调整调试级别

## 使用方法

### 1. 确认当前调试设置
```bash
show debugging sdplane
```

### 2. 确认调试类别
请通过 `show debugging sdplane` 命令确认可用的类别。

### 3. 更改调试设置
```bash
# 启用特定类别的调试
debug sdplane [category] [level]
```

### 4. 确认调试日志
调试日志输出到标准输出或日志文件。

## 故障排除

### 调试日志未输出时
1. 确认调试类别设置是否正确
2. 确认调试级别设置是否合适
3. 确认日志输出目标设置是否正确

### 对性能的影响
- 启用调试日志可能影响性能
- 建议在生产环境中仅启用必要的最小调试

## 定义位置

这些命令在以下文件中定义：
- `sdplane/debug_sdplane.c`

## 相关项目

- [系统信息·监视](system-monitoring.md)
- [VTY·Shell管理](vty-shell.md)