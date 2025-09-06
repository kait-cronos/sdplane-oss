# 调试和日志记录

**Language:** [English](../debug-logging.md) | [日本語](../ja/debug-logging.md) | [Français](../fr/debug-logging.md) | [中文](../zh/debug-logging.md) | [Deutsch](../de/debug-logging.md) | [Italiano](../it/debug-logging.md) | [한국어](../ko/debug-logging.md) | [ไทย](../th/debug-logging.md) | [Español](../es/debug-logging.md)

用于控制sdplane调试和日志记录功能的命令。

## 命令列表

### log_file - 日志文件输出配置
```
log file <file-path>
```

配置日志输出到文件。

**参数：**
- <文件路径> - 日志输出文件的路径

**示例：**
```bash
# 将日志输出到指定文件
log file /var/log/sdplane.log

# 调试日志文件
log file /tmp/sdplane-debug.log
```

### log_stdout - 标准输出日志配置
```
log stdout
```

配置日志输出到标准输出。

**示例：**
```bash
# 在标准输出上显示日志
log stdout
```

**注意：** `log file`和`log stdout`可以同时配置，日志将输出到两个目的地。

### debug - 调试配置
```
debug <category> <target>
```

为指定类别内的特定目标启用调试日志记录。

**类别：**
- `sdplane` - 主sdplane类别
- `zcmdsh` - 命令shell类别

**sdplane目标列表：**
- `lthread` - 轻量级线程
- `console` - 控制台
- `tap-handler` - TAP处理程序
- `l2fwd` - L2转发
- `l3fwd` - L3转发
- `vty-server` - VTY服务器
- `vty-shell` - VTY shell
- `stat-collector` - 统计收集器
- `packet` - 数据包处理
- `fdb` - FDB（转发数据库）
- `fdb-change` - FDB更改
- `rib` - RIB（路由信息库）
- `vswitch` - 虚拟交换机
- `vlan-switch` - VLAN交换机
- `pktgen` - 数据包生成器
- `enhanced-repeater` - 增强中继器
- `netlink` - Netlink接口
- `neighbor` - 邻居管理
- `all` - 所有目标

**示例：**
```bash
# 为特定目标启用调试
debug sdplane rib
debug sdplane fdb-change
debug sdplane pktgen

# 启用所有sdplane调试
debug sdplane all

# zcmdsh类别调试
debug zcmdsh shell
debug zcmdsh command
```

### no debug - 调试禁用
```
no debug <category> <target>
```

为指定类别内的特定目标禁用调试日志记录。

**示例：**
```bash
# 禁用特定目标的调试
no debug sdplane rib
no debug sdplane fdb-change

# 禁用所有sdplane调试（推荐）
no debug sdplane all

# 禁用zcmdsh类别调试
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
