# Gestión VTY y Shell

**Idioma / Language:** [English](../vty-shell.md) | [日本語](../ja/vty-shell.md) | [Français](../fr/vty-shell.md) | [中文](../zh/vty-shell.md) | [Deutsch](../de/vty-shell.md) | [Italiano](../it/vty-shell.md) | [한국어](../ko/vty-shell.md) | [ไทย](../th/vty-shell.md) | **Español**

Comandos para gestionar VTY (Terminal Virtual) y shell.

## Lista de Comandos

### clear_cmd - Limpiar Pantalla
```
clear
```

Limpia la pantalla del VTY.

**Ejemplo de uso:**
```bash
clear
```

### vty_exit_cmd - Salida de VTY
```
(exit|quit|logout)
```

Termina la sesión VTY. Múltiples alias están disponibles.

**Ejemplos de uso:**
```bash
exit
# o
quit
# o
logout
```

### shutdown_cmd - Apagado del Sistema
```
shutdown
```

Apaga el sistema sdplane.

**Ejemplo de uso:**
```bash
shutdown
```

**Nota:** Este comando detiene todo el sistema. Antes de ejecutar, confirme el guardado de configuraciones y la terminación de sesiones activas.

### exit_cmd - Salida de Consola
```
(exit|quit)
```

Termina el shell de consola.

**Ejemplos de uso:**
```bash
exit
# o
quit
```

## Diferencias entre VTY y Consola

### Shell VTY
- Shell remoto accedido vía Telnet
- Puede usar múltiples sesiones simultáneamente
- Acceso posible vía red

### Shell de Consola
- Acceso desde consola local
- Acceso directo al sistema
- Principalmente para administración local

## Métodos de Conexión

### Conexión a VTY
```bash
telnet localhost 9882
```

### Conexión a Consola
```bash
# Ejecutar sdplane directamente
sudo ./sdplane/sdplane
```

## Gestión de Sesiones

### Verificar Sesiones
El estado de sesiones VTY se puede verificar con el siguiente comando:
```bash
show worker
```

### Terminar Sesiones
- Terminación normal con comandos `exit`, `quit`, `logout`
- En caso de terminación anormal, detener todo el sistema con comando `shutdown`

## Consideraciones de Seguridad

### Control de Acceso
- VTY permite acceso solo desde localhost (127.0.0.1) por defecto
- Se recomienda restringir acceso al puerto 9882 con configuración de firewall

### Monitorización de Sesiones
- Terminar apropiadamente las sesiones innecesarias
- Verificar regularmente las sesiones inactivas por largos períodos

## Solución de Problemas

### Cuando No Se Puede Conectar a VTY
1. Verificar si sdplane se inició normalmente
2. Verificar si el puerto 9882 está disponible
3. Verificar configuración de firewall

### Cuando las Sesiones No Responden
1. Verificar estado con `show worker` desde otra sesión VTY
2. Reiniciar sistema con comando `shutdown` si es necesario

### Cuando los Comandos No Son Reconocidos
1. Verificar si está en el shell correcto (VTY o consola)
2. Mostrar ayuda con `?` para verificar comandos disponibles

## Ubicación de Definición

Estos comandos están definidos en los siguientes archivos:
- `sdplane/vty_shell.c` - Comandos relacionados con VTY
- `sdplane/console_shell.c` - Comandos relacionados con consola

## Elementos Relacionados

- [Depuración y Registros](debug-logging.md)
- [Información del Sistema y Monitorización](system-monitoring.md)