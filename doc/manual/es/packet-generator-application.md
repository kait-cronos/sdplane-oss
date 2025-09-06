# Aplicación Generador de Paquetes (PKTGEN)

**Idioma / Language:** [English](../packet-generator-application.md) | [日本語](../ja/packet-generator-application.md) | [Français](../fr/packet-generator-application.md) | [中文](../zh/packet-generator-application.md) | [Deutsch](../de/packet-generator-application.md) | [Italiano](../it/packet-generator-application.md) | [한국어](../ko/packet-generator-application.md) | [ไทย](../th/packet-generator-application.md) | **Español**

La aplicación Generador de Paquetes (PKTGEN) proporciona capacidades de generación de paquetes de alto rendimiento y pruebas de tráfico utilizando el marco de procesamiento de paquetes optimizado de DPDK.

## Resumen

PKTGEN es una herramienta sofisticada de generación de tráfico integrada en sdplane-oss que proporciona:
- **Generación de Paquetes de Alta Velocidad**: Tasas de transmisión de paquetes multi-gigabit
- **Patrones de Tráfico Flexibles**: Tamaño, tasa y patrones de paquetes personalizables
- **Soporte Multi-puerto**: Generación independiente de tráfico en múltiples puertos
- **Características Avanzadas**: Pruebas de rango, limitación de tasa, modelado de tráfico
- **Pruebas de Rendimiento**: Medición de rendimiento y latencia de red

## Arquitectura

### Componentes Centrales
- **Motor TX**: Transmisión de paquetes de alto rendimiento usando DPDK
- **Motor RX**: Recepción de paquetes y recolección de estadísticas
- **Marco L2P**: Mapeo lcore-to-port para rendimiento óptimo
- **Gestión de Configuración**: Configuración dinámica de parámetros de tráfico
- **Motor de Estadísticas**: Métricas de rendimiento e informes completos

### Modelo de Worker
PKTGEN opera usando hilos de trabajo (lcore) dedicados:
- **Workers TX**: Núcleos dedicados a transmisión de paquetes
- **Workers RX**: Núcleos dedicados a recepción de paquetes
- **Workers Mixtos**: TX/RX combinados en un solo núcleo
- **Hilos de Control**: Gestión y recolección de estadísticas

## Características Clave

### Generación de Tráfico
- **Tasa de Paquetes**: Generación de tráfico a velocidad de línea hasta límites de interfaz
- **Tamaño de Paquetes**: Configurable desde 64 bytes hasta tramas jumbo
- **Patrones de Tráfico**: Uniforme, ráfaga, patrones personalizados
- **Multi-flujo**: Múltiples flujos de tráfico por puerto

### Características Avanzadas
- **Limitación de Tasa**: Control preciso de tasa de tráfico
- **Pruebas de Rango**: Barridos de tamaño de paquete y tasa
- **Patrones de Carga**: Tráfico constante, rampa, ráfaga
- **Soporte de Protocolo**: Ethernet, IPv4, IPv6, TCP, UDP

### Monitorización de Rendimiento
- **Estadísticas en Tiempo Real**: Tasas TX/RX, conteos de paquetes, conteos de errores
- **Medición de Latencia**: Pruebas de latencia extremo a extremo
- **Análisis de Rendimiento**: Utilización y eficiencia de ancho de banda
- **Detección de Errores**: Detección de pérdida y corrupción de paquetes

## Configuración

### Configuración Básica
PKTGEN requiere inicialización específica y configuración de workers:

```bash
# Vinculación de dispositivos para PKTGEN
set device 01:00.0 driver vfio-pci bind
set device 01:00.1 driver vfio-pci bind

# Inicialización PKTGEN con argv-list
set argv-list 2 ./usr/local/bin/pktgen -l 0-7 -n 4 --proc-type auto --log-level 7 --file-prefix pg -- -v -T -P -l pktgen.log -m [4:5].0 -m [6:7].1 -f themes/black-yellow.theme

# Inicialización PKTGEN
pktgen init argv-list 2
```

### Asignación de Workers
Asignación de núcleos dedicados para rendimiento óptimo:

```bash
# Asignar workers PKTGEN a lcore específicos
set worker lcore 4 pktgen  # Puerto 0 TX/RX
set worker lcore 5 pktgen  # Puerto 0 TX/RX
set worker lcore 6 pktgen  # Puerto 1 TX/RX
set worker lcore 7 pktgen  # Puerto 1 TX/RX
```

### Configuración de Puertos
Configuración de puertos DPDK para uso con PKTGEN:

```bash
# Configuración de puertos
stop port all
set port all dev-configure 1 4
set port all nrxdesc 1024
set port all ntxdesc 1024
set port all promiscuous enable
start port all
```

## Ejemplos de Configuración

### Configuración Completa de PKTGEN
Consulte [`example-config/sdplane-pktgen.conf`](../../example-config/sdplane-pktgen.conf):

```bash
# Configuración de logs
log file /var/log/sdplane.log
log stdout

# Vinculación de dispositivos
set device 01:00.0 driver vfio-pci bind
set device 01:00.1 driver vfio-pci bind
set device 03:00.0 driver uio_pci_generic bind

# Inicialización PKTGEN
set argv-list 2 ./usr/local/bin/pktgen -l 0-7 -n 4 --proc-type auto --log-level 7 --file-prefix pg -- -v -T -P -l pktgen.log -m [4:5].0 -m [6:7].1 -f themes/black-yellow.theme
pktgen init argv-list 2

# Configuración de pool de memoria
set mempool

# Workers de segundo plano
set worker lthread stat-collector
set worker lthread rib-manager
set worker lthread netlink-thread

# Configuración de puertos
stop port all
set port all dev-configure 1 4
set port all nrxdesc 1024
set port all ntxdesc 1024

# Asignación de workers
set worker lcore 2 vlan-switch
set worker lcore 3 tap-handler
set worker lcore 4 pktgen
set worker lcore 5 pktgen
set worker lcore 6 pktgen
set worker lcore 7 pktgen

# Habilitar modo promiscuo e iniciar
set port all promiscuous enable
start port all

# Iniciar todos los workers
start worker lcore all
```

## Argumentos de Línea de Comandos PKTGEN

### Parámetros Centrales
- `-l 0-7`: Lista de núcleos CPU (núcleos 0-7)
- `-n 4`: Número de canales de memoria
- `--proc-type auto`: Tipo de proceso (primario/secundario)
- `--log-level 7`: Nivel de log de depuración
- `--file-prefix pg`: Prefijo de memoria compartida

### Opciones Específicas de PKTGEN
- `-v`: Salida verbosa
- `-T`: Habilitar salida de terminal a color
- `-P`: Habilitar modo promiscuo
- `-l pktgen.log`: Ubicación del archivo de log
- `-m [4:5].0`: Mapear lcore 4,5 al puerto 0
- `-m [6:7].1`: Mapear lcore 6,7 al puerto 1
- `-f themes/black-yellow.theme`: Tema de color

## Mapeo lcore-to-port

### Sintaxis de Mapeo
Asignar núcleos a puertos usando el formato `[lcore_list].port`:
- `[4:5].0`: lcore 4 y 5 manejan puerto 0
- `[6:7].1`: lcore 6 y 7 manejan puerto 1
- `[4].0`: solo lcore 4 maneja puerto 0
- `[4-7].0`: lcore 4 a 7 manejan puerto 0

### Optimización de Rendimiento
- **Núcleos Dedicados**: Asignar núcleos separados para TX y RX
- **Conciencia NUMA**: Usar núcleos locales a interfaces de red
- **Evitar Contención**: Evitar superposición con workers del sistema
- **Balanceo de Carga**: Distribuir tráfico entre múltiples núcleos

## Operación y Monitorización

### Comandos de Estado
```bash
# Mostrar estado y configuración de PKTGEN
show pktgen

# Mostrar asignación y rendimiento de workers
show worker statistics

# Estadísticas a nivel de puerto
show port statistics all
```

### Gestión en Tiempo de Ejecución
PKTGEN proporciona configuración extensiva en tiempo de ejecución a través de CLI interactiva:
- **Parámetros de Tráfico**: Tamaño de paquete, tasa, patrones
- **Control Iniciar/Detener**: Control de tráfico por puerto
- **Estadísticas**: Monitorización de rendimiento en tiempo real
- **Pruebas de Rango**: Automatización de barridos de parámetros

### Monitorización de Rendimiento
```bash
# Monitorizar estadísticas en tiempo real
# (Disponible a través de interfaz interactiva PKTGEN)

# Métricas clave:
# - Paquetes TX/RX por segundo
# - Utilización de ancho de banda
# - Tasa de pérdida de paquetes
# - Mediciones de latencia
```

## Comandos CLI

### Comandos de Control PKTGEN

#### Inicialización
```bash
# Inicializar PKTGEN con configuración argv-list
pktgen init argv-list <0-7>
```

#### Iniciar/Detener Generación de Tráfico
```bash
# Iniciar generación de tráfico en puerto específico
pktgen do start port <0-7>
pktgen do start port all

# Detener generación de tráfico
pktgen do stop port <0-7>
pktgen do stop port all
```

#### Comandos de Configuración de Tráfico

##### Configuración de Número de Paquetes
```bash
# Configurar número de paquetes a transmitir
pktgen do set port <0-7> count <0-4000000000>
pktgen do set port all count <0-4000000000>
```

##### Configuración de Tamaño de Paquetes
```bash
# Configurar tamaño de paquetes en bytes
pktgen do set port <0-7> size <0-9999>
pktgen do set port all size <0-9999>
```

##### Configuración de Tasa de Transmisión
```bash
# Configurar tasa de transmisión en porcentaje
pktgen do set port <0-7> rate <0-100>
pktgen do set port all rate <0-100>
```

##### Configuración de Números de Puerto TCP/UDP
```bash
# Configurar puertos fuente y destino TCP
pktgen do set port <0-7> tcp src <0-65535> dst <0-65535>
pktgen do set port all tcp src <0-65535> dst <0-65535>

# Configurar puertos fuente y destino UDP
pktgen do set port <0-7> udp src <0-65535> dst <0-65535>
pktgen do set port all udp src <0-65535> dst <0-65535>
```

##### Configuración de Valor TTL
```bash
# Configurar valor IP Time-to-Live
pktgen do set port <0-7> ttl <0-255>
pktgen do set port all ttl <0-255>
```

##### Configuración de Direcciones MAC
```bash
# Configurar dirección MAC fuente
pktgen do set port <0-7> mac source <MAC>
pktgen do set port all mac source <MAC>

# Configurar dirección MAC destino
pktgen do set port <0-7> mac destination <MAC>
pktgen do set port all mac destination <MAC>
```

##### Configuración de Direcciones IPv4
```bash
# Configurar dirección IPv4 fuente
pktgen do set port <0-7> ipv4 source <IPv4>
pktgen do set port all ipv4 source <IPv4>

# Configurar dirección IPv4 destino
pktgen do set port <0-7> ipv4 destination <IPv4>
pktgen do set port all ipv4 destination <IPv4>
```

#### Comandos de Estado y Monitorización
```bash
# Mostrar estado y configuración de PKTGEN
show pktgen

# Mostrar estadísticas de puertos
show port statistics all
show port statistics <0-7>
```

## Casos de Uso

### Pruebas de Rendimiento de Red
- **Pruebas de Rendimiento**: Medición de ancho de banda máximo
- **Pruebas de Latencia**: Análisis de latencia extremo a extremo
- **Pruebas de Carga**: Generación sostenida de tráfico
- **Pruebas de Estrés**: Validación de tasa máxima de paquetes

### Validación de Equipos
- **Pruebas de Conmutadores**: Validación de rendimiento de reenvío
- **Pruebas de Enrutadores**: Rendimiento de reenvío L3
- **Pruebas de Interfaces**: Validación de puertos y cables
- **Pruebas de Protocolos**: Comportamientos específicos de protocolos

### Desarrollo de Red
- **Desarrollo de Protocolos**: Pruebas de nuevos protocolos de red
- **Pruebas de Aplicaciones**: Generación de patrones realistas de tráfico
- **Ajuste de Rendimiento**: Optimización de configuraciones de red
- **Benchmarking**: Comparaciones estandarizadas de rendimiento

## Ajuste de Rendimiento

### Asignación de Núcleos
```bash
# Optimizar uso de núcleos para máximo rendimiento
# Separar núcleos TX y RX cuando sea posible
# Usar núcleos NUMA locales a interfaces de red
set worker lcore 4 pktgen  # Núcleo TX para puerto 0
set worker lcore 5 pktgen  # Núcleo RX para puerto 0
```

### Configuración de Memoria
```bash
# Optimizar anillos de descriptores para patrones de tráfico
set port all nrxdesc 2048  # Aumentar para alta tasa
set port all ntxdesc 2048  # Aumentar para ráfagas
```

### Ajuste del Sistema
- **Aislamiento de CPU**: Aislar núcleos PKTGEN del planificador OS
- **Afinidad de Interrupciones**: Vincular interrupciones a núcleos no-PKTGEN
- **Asignación de Memoria**: Usar páginas enormes para rendimiento óptimo
- **Ajuste de NIC**: Optimizar configuraciones de interfaz de red

## Solución de Problemas

### Problemas Comunes
- **Bajo Rendimiento**: Verificar asignación de núcleos y topología NUMA
- **Pérdida de Paquetes**: Verificar tamaños de búfer y recursos del sistema
- **Fallas de Inicialización**: Verificar vinculación de dispositivos y permisos
- **Limitación de Tasa**: Verificar capacidades y configuración de interfaz

### Estrategias de Depuración
- **Habilitar Logs Verbosos**: Usar niveles de log más altos para salida detallada
- **Verificar Estadísticas**: Monitorizar contadores TX/RX para anomalías
- **Verificar Mapeos**: Asegurar asignaciones correctas lcore-to-port
- **Monitorizar Sistema**: Verificar uso de CPU, memoria e interrupciones durante pruebas

#### Comandos de Depuración
```bash
# Habilitar logs de depuración PKTGEN
debug sdplane pktgen

# Depuración general de sdplane
debug sdplane rib
debug sdplane fdb-change
```

### Verificación de Rendimiento
```bash
# Verificar que PKTGEN esté alcanzando tasas esperadas
show pktgen
show port statistics all

# Verificar errores o descartes
# Monitorizar recursos del sistema durante pruebas
```

## Características Avanzadas

### Pruebas de Rango
PKTGEN soporta pruebas automatizadas en rangos de parámetros:
- **Barrido de Tamaño de Paquetes**: Pruebas desde 64 a 1518 bytes
- **Barrido de Tasas**: Pruebas desde 1% a 100% de velocidad de línea
- **Informes Automáticos**: Generación de informes de prueba completos

### Patrones de Tráfico
- **Tasa Constante**: Generación estable de tráfico
- **Patrones de Ráfaga**: Ráfagas de tráfico con períodos inactivos
- **Patrones de Rampa**: Tasas gradualmente crecientes/decrecientes
- **Patrones Personalizados**: Perfiles de tráfico definidos por el usuario

## Documentación Relacionada

- [Comandos de Generación de Paquetes](packet-generation.md) - Referencia de comandos
- [Gestión de Workers](worker-lcore-thread-management.md) - Configuración de workers
- [Gestión de Puertos](port-management.md) - Configuración de puertos DPDK
- [Guía de Ajuste de Rendimiento](#) - Consejos de optimización del sistema

## Recursos Externos

- [Documentación DPDK Pktgen](http://pktgen-dpdk.readthedocs.io/) - Documentación oficial de PKTGEN
- [Guía de Rendimiento DPDK](https://doc.dpdk.org/guides/prog_guide/) - Guías de optimización DPDK