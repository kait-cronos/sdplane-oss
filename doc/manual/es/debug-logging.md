# Depuración y Registros

**Idioma / Language:** [English](../debug-logging.md) | [日本語](../ja/debug-logging.md) | [Français](../fr/debug-logging.md) | [中文](../zh/debug-logging.md) | [Deutsch](../de/debug-logging.md) | [Italiano](../it/debug-logging.md) | [한국어](../ko/debug-logging.md) | [ไทย](../th/debug-logging.md) | **Español**

Comandos para controlar las funciones de depuración y registro de sdplane.

## Lista de Comandos

### debug_sdplane - Configuración de Depuración sdplane
```
debug sdplane [categoría] [nivel]
```

Configura los registros de depuración de sdplane. Este comando se genera dinámicamente, por lo que las categorías y niveles disponibles se determinan en tiempo de ejecución.

**Ejemplos de uso:**
```bash
# Habilitar configuración de depuración
debug sdplane

# Habilitar depuración para una categoría específica
debug sdplane [category] [level]
```

**Nota:** Las categorías y niveles específicos se pueden verificar con el comando `show debugging sdplane`.

### show_debug_sdplane - Mostrar Información de Depuración sdplane
```
show debugging sdplane
```

Muestra la configuración de depuración actual de sdplane.

**Ejemplo de uso:**
```bash
show debugging sdplane
```

Este comando muestra la siguiente información:
- Categorías de depuración actualmente habilitadas
- Nivel de depuración para cada categoría
- Opciones de depuración disponibles

## Resumen del Sistema de Depuración

El sistema de depuración de sdplane tiene las siguientes características:

### Depuración Basada en Categorías
- Las categorías de depuración están separadas por diferentes módulos funcionales
- Se pueden habilitar registros de depuración solo para las funciones necesarias

### Control Basado en Niveles
- Los mensajes de depuración están clasificados por niveles según su importancia
- Configurando el nivel apropiado, se puede mostrar solo la información necesaria

### Configuración Dinámica
- Se pueden cambiar las configuraciones de depuración durante la operación del sistema
- Se pueden ajustar los niveles de depuración sin reiniciar

## Método de Uso

### 1. Verificar Configuración de Depuración Actual
```bash
show debugging sdplane
```

### 2. Verificar Categorías de Depuración
Verifique las categorías disponibles con el comando `show debugging sdplane`.

### 3. Cambiar Configuración de Depuración
```bash
# Habilitar depuración para una categoría específica
debug sdplane [category] [level]
```

### 4. Verificar Registros de Depuración
Los registros de depuración se envían a la salida estándar o archivos de registro.

## Solución de Problemas

### Cuando los Registros de Depuración No Se Muestran
1. Verificar si la categoría de depuración está configurada correctamente
2. Verificar si el nivel de depuración está configurado apropiadamente
3. Verificar si el destino de salida del registro está configurado correctamente

### Impacto en el Rendimiento
- Habilitar registros de depuración puede afectar el rendimiento
- Se recomienda habilitar solo la depuración mínima necesaria en entornos de producción

## Ubicación de Definición

Estos comandos están definidos en los siguientes archivos:
- `sdplane/debug_sdplane.c`

## Elementos Relacionados

- [Información del Sistema y Monitorización](system-monitoring.md)
- [Gestión VTY y Shell](vty-shell.md)