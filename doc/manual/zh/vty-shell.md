# VTY·Shell管理

**语言 / Language:** [English](../vty-shell.md) | [日本語](../ja/vty-shell.md) | **中文**

管理VTY（Virtual Terminal）和Shell的命令。

## 命令一览

### clear_cmd - 清屏
```
clear
```

清除VTY屏幕。

**使用例：**
```bash
clear
```

### vty_exit_cmd - VTY退出
```
(exit|quit|logout)
```

结束VTY会话。可以使用多个别名。

**使用例：**
```bash
exit
# 或者
quit
# 或者
logout
```

### shutdown_cmd - 系统关机
```
shutdown
```

关闭sdplane系统。

**使用例：**
```bash
shutdown
```

**注意：** 该命令将停止整个系统。执行前请确认保存配置和结束活动会话。

### exit_cmd - 控制台退出
```
(exit|quit)
```

退出控制台Shell。

**使用例：**
```bash
exit
# 或者
quit
```

## VTY和控制台的区别

### VTY Shell
- 通过Telnet访问的远程Shell
- 可以同时使用多个会话
- 可以通过网络访问

### 控制台Shell
- 从本地控制台访问
- 直接的系统访问
- 主要用于本地管理

## 连接方法

### 连接到VTY
```bash
telnet localhost 9882
```

### 连接到控制台
```bash
# 直接运行sdplane
sudo ./sdplane/sdplane
```

## 会话管理

### 确认会话
可以通过以下命令确认VTY会话状态：
```bash
show worker
```

### 结束会话
- 使用 `exit`、`quit`、`logout` 命令正常结束
- 异常结束时使用 `shutdown` 命令停止整个系统

## 安全注意事项

### 访问控制
- VTY默认仅允许从localhost（127.0.0.1）访问
- 建议通过防火墙设置限制端口9882的访问

### 会话监控
- 适当结束不需要的会话
- 定期确认长时间空闲会话

## 故障排除

### 无法连接VTY时
1. 确认sdplane正常启动
2. 确认端口9882可用
3. 确认防火墙设置

### 会话无响应时
1. 从其他VTY会话用 `show worker` 确认状态
2. 必要时使用 `shutdown` 命令重启系统

### 命令无法识别时
1. 确认是否在正确的Shell（VTY或控制台）中
2. 使用 `?` 显示帮助，确认可用命令

## 定义位置

这些命令在以下文件中定义：
- `sdplane/vty_shell.c` - VTY相关命令
- `sdplane/console_shell.c` - 控制台相关命令

## 相关项目

- [调试·日志](debug-logging.md)
- [系统信息·监视](system-monitoring.md)