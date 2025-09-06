# Enhanced Repeater-Konfiguration

Der Enhanced Repeater bietet erweiterte VLAN-Switching-Funktionen mit TAP-Interfaces für L3-Routing und Paket-Capture. Er unterstützt die Erstellung virtueller Switches, das Verknüpfen von DPDK-Ports mit VLAN-Tagging, Router-Interfaces für Kernel-Integration und Capture-Interfaces für Überwachung.

## Virtual Switch-Befehle

### set vswitch
```
set vswitch <1-4094>
```
**Beschreibung**: Erstellt einen virtuellen Switch mit der angegebenen VLAN-ID

**Parameter**:
- `<1-4094>`: VLAN-ID für den virtuellen Switch

**Beispiel**:
```
set vswitch 2031
set vswitch 2032
```

### delete vswitch
```
delete vswitch <0-3>
```
**Beschreibung**: Löscht den durch ID spezifizierten virtuellen Switch

**Parameter**:
- `<0-3>`: Zu löschende virtuelle Switch-ID

**Beispiel**:
```
delete vswitch 0
```

### show vswitch_rib
```
show vswitch_rib
```
**Beschreibung**: Zeigt virtuelle Switch-RIB-Informationen einschließlich Konfiguration und Status an

## Virtual Switch-Link-Befehle

### set vswitch-link
```
set vswitch-link vswitch <0-3> port <0-7> tag <0-4094>
```
**Beschreibung**: Verknüpft einen DPDK-Port mit einem virtuellen Switch mit VLAN-Tagging-Konfiguration

**Parameter**:
- `vswitch <0-3>`: Virtuelle Switch-ID (0-3)
- `port <0-7>`: DPDK-Port-ID (0-7)  
- `tag <0-4094>`: VLAN-Tag-ID (0: native/untagged, 1-4094: tagged VLAN)

**Beispiel**:
```
# Port 0 mit virtuellem Switch 0 mit VLAN-Tag 2031 verknüpfen
set vswitch-link vswitch 0 port 0 tag 2031

# Port 0 mit virtuellem Switch 1 als native/untagged verknüpfen
set vswitch-link vswitch 1 port 0 tag 0
```

### delete vswitch-link
```
delete vswitch-link <0-7>
```
**Beschreibung**: Löscht die durch ID spezifizierte virtuelle Switch-Verknüpfung

**Parameter**:
- `<0-7>`: Virtuelle Switch-Link-ID

### show vswitch-link
```
show vswitch-link
```
**Beschreibung**: Zeigt alle virtuellen Switch-Link-Konfigurationen an

## Router-Interface-Befehle

### set vswitch router-if
```
set vswitch <1-4094> router-if <WORD>
```
**Beschreibung**: Erstellt ein Router-Interface für L3-Konnektivität auf dem spezifizierten virtuellen Switch

**Parameter**:
- `<1-4094>`: VLAN-ID des virtuellen Switches
- `<WORD>`: TAP-Interface-Name

**Beispiel**:
```
set vswitch 2031 router-if rif2031
set vswitch 2032 router-if rif2032
```

### no set vswitch router-if
```
no set vswitch <1-4094> router-if
```
**Beschreibung**: Entfernt das Router-Interface vom spezifizierten virtuellen Switch

**Parameter**:
- `<1-4094>`: VLAN-ID des virtuellen Switches

### show rib vswitch router-if
```
show rib vswitch router-if
```
**Beschreibung**: Zeigt Router-Interface-Konfigurationen einschließlich MAC-Adressen, IP-Adressen und Interface-Status an

## Capture-Interface-Befehle

### set vswitch capture-if
```
set vswitch <1-4094> capture-if <WORD>
```
**Beschreibung**: Erstellt ein Capture-Interface für Paketüberwachung auf dem spezifizierten virtuellen Switch

**Parameter**:
- `<1-4094>`: VLAN-ID des virtuellen Switches
- `<WORD>`: TAP-Interface-Name

**Beispiel**:
```
set vswitch 2031 capture-if cif2031
set vswitch 2032 capture-if cif2032
```

### no set vswitch capture-if
```
no set vswitch <1-4094> capture-if
```
**Beschreibung**: Entfernt das Capture-Interface vom spezifizierten virtuellen Switch

**Parameter**:
- `<1-4094>`: VLAN-ID des virtuellen Switches

### show rib vswitch capture-if
```
show rib vswitch capture-if
```
**Beschreibung**: Zeigt Capture-Interface-Konfigurationen an

## VLAN-Verarbeitungsfunktionen

Der Enhanced Repeater führt erweiterte VLAN-Verarbeitung durch:

- **VLAN-Translation**: Ändert VLAN-IDs basierend auf vswitch-link-Konfiguration
- **VLAN-Removal**: Entfernt VLAN-Header wenn Tag auf 0 (native) gesetzt ist  
- **VLAN-Insertion**: Fügt VLAN-Header hinzu, wenn untagged-Pakete an tagged-Ports weitergeleitet werden
- **Split-Horizon**: Verhindert Schleifen durch Nicht-Rücksendung von Paketen an den empfangenden Port

## Konfigurationsbeispiel

```bash
# Virtuelle Switches erstellen
set vswitch 2031
set vswitch 2032

# DPDK-Ports mit VLAN-Tags zu virtuellen Switches verknüpfen
set vswitch-link vswitch 0 port 0 tag 2031
set vswitch-link vswitch 1 port 0 tag 2032

# Router-Interfaces für L3-Verarbeitung erstellen
set vswitch 2031 router-if rif2031
set vswitch 2032 router-if rif2032

# Capture-Interfaces für Überwachung erstellen
set vswitch 2031 capture-if cif2031
set vswitch 2032 capture-if cif2032

# Worker für Enhanced Repeater konfigurieren
set worker lcore 1 enhanced-repeater
```

## TAP-Interface-Integration

Router-Interfaces und Capture-Interfaces erstellen TAP-Interfaces, die mit dem Linux-Kernel-Netzwerkstack integriert sind:

- **Router-Interfaces**: Ermöglichen L3-Routing, IP-Adressierung und Kernel-Netzwerkverarbeitung
- **Capture-Interfaces**: Ermöglichen Paketüberwachung, -analyse und -debugging
- **Ring-Buffer**: Verwenden DPDK-Ringe für effiziente Paketübertragung zwischen Datenebene und Kernel