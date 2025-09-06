# Configuración de Repetidor Mejorado

**Idioma / Language:** [English](../enhanced-repeater.md) | [日本語](../ja/enhanced-repeater.md) | [Français](../fr/enhanced-repeater.md) | [中文](../zh/enhanced-repeater.md) | [Deutsch](../de/enhanced-repeater.md) | [Italiano](../it/enhanced-repeater.md) | [한국어](../ko/enhanced-repeater.md) | [ไทย](../th/enhanced-repeater.md) | **Español**

El Repetidor Mejorado proporciona funcionalidades avanzadas de conmutación VLAN con interfaces TAP para enrutamiento L3 y captura de paquetes. Soporta la creación de conmutadores virtuales, vinculación de puertos DPDK con etiquetado VLAN, interfaces de enrutador para integración con kernel, e interfaces de captura para monitorización.

## Comandos de Conmutador Virtual

### set vswitch
```
set vswitch <1-4094>
```
**Descripción**: Crea un conmutador virtual con el ID VLAN especificado

**Parámetros**:
- `<1-4094>`: ID VLAN del conmutador virtual

**Ejemplos**:
```
set vswitch 2031
set vswitch 2032
```

### delete vswitch
```
delete vswitch <0-3>
```
**Descripción**: Elimina el conmutador virtual especificado por ID

**Parámetros**:
- `<0-3>`: ID del conmutador virtual a eliminar

**Ejemplo**:
```
delete vswitch 0
```

### show vswitch_rib
```
show vswitch_rib
```
**Descripción**: Muestra información RIB del conmutador virtual incluyendo configuración y estado

## Comandos de Enlace de Conmutador Virtual

### set vswitch-link
```
set vswitch-link vswitch <0-3> port <0-7> tag <0-4094>
```
**Descripción**: Vincula un puerto DPDK a un conmutador virtual con configuración de etiquetado VLAN

**Parámetros**:
- `vswitch <0-3>`: ID del conmutador virtual (0-3)
- `port <0-7>`: ID del puerto DPDK (0-7)  
- `tag <0-4094>`: ID de etiqueta VLAN (0: nativo/sin etiquetar, 1-4094: VLAN etiquetado)

**Ejemplos**:
```
# Vincular puerto 0 al conmutador virtual 0 con etiqueta VLAN 2031
set vswitch-link vswitch 0 port 0 tag 2031

# Vincular puerto 0 al conmutador virtual 1 como nativo/sin etiquetar
set vswitch-link vswitch 1 port 0 tag 0
```

### delete vswitch-link
```
delete vswitch-link <0-7>
```
**Descripción**: Elimina el enlace de conmutador virtual especificado por ID

**Parámetros**:
- `<0-7>`: ID del enlace de conmutador virtual

### show vswitch-link
```
show vswitch-link
```
**Descripción**: Muestra todas las configuraciones de enlace de conmutador virtual

## Comandos de Interfaz de Enrutador

### set vswitch router-if
```
set vswitch <1-4094> router-if <WORD>
```
**Descripción**: Crea una interfaz de enrutador para el conmutador virtual especificado para conectividad L3

**Parámetros**:
- `<1-4094>`: ID VLAN del conmutador virtual
- `<WORD>`: Nombre de interfaz TAP

**Ejemplos**:
```
set vswitch 2031 router-if rif2031
set vswitch 2032 router-if rif2032
```

### no set vswitch router-if
```
no set vswitch <1-4094> router-if
```
**Descripción**: Elimina la interfaz de enrutador del conmutador virtual especificado

**Parámetros**:
- `<1-4094>`: ID VLAN del conmutador virtual

### show rib vswitch router-if
```
show rib vswitch router-if
```
**Descripción**: Muestra configuración de interfaz de enrutador incluyendo direcciones MAC, direcciones IP y estado de interfaz

## Comandos de Interfaz de Captura

### set vswitch capture-if
```
set vswitch <1-4094> capture-if <WORD>
```
**Descripción**: Crea una interfaz de captura para el conmutador virtual especificado para monitorización de paquetes

**Parámetros**:
- `<1-4094>`: ID VLAN del conmutador virtual
- `<WORD>`: Nombre de interfaz TAP

**Ejemplos**:
```
set vswitch 2031 capture-if cif2031
set vswitch 2032 capture-if cif2032
```

### no set vswitch capture-if
```
no set vswitch <1-4094> capture-if
```
**Descripción**: Elimina la interfaz de captura del conmutador virtual especificado

**Parámetros**:
- `<1-4094>`: ID VLAN del conmutador virtual

### show rib vswitch capture-if
```
show rib vswitch capture-if
```
**Descripción**: Muestra configuración de interfaz de captura

## Funcionalidades de Procesamiento VLAN

El Repetidor Mejorado realiza procesamiento VLAN avanzado:

- **Traducción VLAN**: Modificar IDs VLAN basándose en configuración vswitch-link
- **Eliminación VLAN**: Eliminar encabezados VLAN cuando la etiqueta está configurada a 0 (nativo)  
- **Inserción VLAN**: Agregar encabezados VLAN al reenviar paquetes sin etiquetar a puertos etiquetados
- **Horizonte Dividido**: Prevenir bucles no enviando paquetes de vuelta al puerto de recepción

## Ejemplo de Configuración

```bash
# Crear conmutadores virtuales
set vswitch 2031
set vswitch 2032

# Vincular puertos DPDK a conmutadores virtuales con etiquetas VLAN
set vswitch-link vswitch 0 port 0 tag 2031
set vswitch-link vswitch 1 port 0 tag 2032

# Crear interfaces de enrutador para procesamiento L3
set vswitch 2031 router-if rif2031
set vswitch 2032 router-if rif2032

# Crear interfaces de captura para monitorización
set vswitch 2031 capture-if cif2031
set vswitch 2032 capture-if cif2032

# Configurar worker para usar repetidor mejorado
set worker lcore 1 enhanced-repeater
```

## Integración con Interfaces TAP

Las interfaces de enrutador y captura crean interfaces TAP que se integran con la pila de red del kernel de Linux:

- **Interfaces de Enrutador**: Habilitan enrutamiento L3, direccionamiento IP y procesamiento de red del kernel
- **Interfaces de Captura**: Habilitan monitorización, análisis y depuración de paquetes
- **Búferes de Anillo**: Utilizan anillos DPDK para transferencia eficiente de paquetes entre plano de datos y kernel