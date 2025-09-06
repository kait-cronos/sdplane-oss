# Information Système & Surveillance

**Language:** [English](../system-monitoring.md) | [日本語](../ja/system-monitoring.md) | **Français** | [中文](../zh/system-monitoring.md) | [Deutsch](../de/system-monitoring.md) | [Italiano](../it/system-monitoring.md) | [한국어](../ko/system-monitoring.md) | [ไทย](../th/system-monitoring.md) | [Español](../es/system-monitoring.md)

Commandes pour fournir les informations système et les fonctions de surveillance dans sdplane.

## Liste des Commandes

### show_version - Afficher la Version
```
show version
```

Affiche les informations de version de sdplane.

**Exemples :**
```bash
show version
```

### set_locale - Définir la Locale
```
set locale (C|C.utf8|en_US.utf8|POSIX)
```

Définit la locale système.

**Locales Disponibles :**
- `C` - Locale C standard
- `C.utf8` - Locale C avec support UTF-8
- `en_US.utf8` - Locale anglaise UTF-8
- `POSIX` - Locale POSIX

**Exemples :**
```bash
# Définir la locale C avec UTF-8
set locale C.utf8

# Définir la locale anglaise UTF-8
set locale en_US.utf8
```

### set_argv_list_1 - Définir argv-list
```
set argv-list <0-7> <WORD>
```

Définit la liste des arguments en ligne de commande.

**Paramètres :**
- `<0-7>` - Index (0-7)
- `<WORD>` - Chaîne à définir

**Exemples :**
```bash
# Définir l'argument à l'index 0
set argv-list 0 "--verbose"

# Définir l'argument à l'index 1
set argv-list 1 "--config"
```

### **show argv-list**

Affiche toute la liste des arguments en ligne de commande configurés.

**Exemples :**
```bash
# Afficher toute la argv-list
show argv-list
```

---

### **show argv-list \<0-7\>**

Affiche la argv-list d'un index spécifique.

**Exemples :**
```bash
# Afficher la argv-list d'un index spécifique
show argv-list 0

# Afficher la argv-list de l'index 3
show argv-list 3
```

### show_loop_count - Afficher le Compteur de Boucles
```
show loop-count (console|vty-shell|l2fwd) (pps|total)
```

Affiche les compteurs de boucles pour chaque composant.

**Composants :**
- `console` - Console
- `vty-shell` - Shell VTY
- `l2fwd` - Transfert L2

**Types de Statistiques :**
- `pps` - Boucles par seconde
- `total` - Nombre total de boucles

**Exemples :**
```bash
# Afficher le PPS de la console
show loop-count console pps

# Afficher le nombre total de boucles du transfert L2
show loop-count l2fwd total
```

### show_rcu - Afficher les Informations RCU
```
show rcu
```

Affiche les informations RCU (Read-Copy-Update).

**Exemples :**
```bash
show rcu
```

### show_fdb - Afficher les Informations FDB
```
show fdb
```

Affiche les informations FDB (Forwarding Database).

**Exemples :**
```bash
show fdb
```

### show_vswitch - Afficher les Informations vswitch
```
show vswitch
```

Affiche les informations du commutateur virtuel.

**Exemples :**
```bash
show vswitch
```

### sleep_cmd - Commande Sleep
```
sleep <0-300>
```

Suspend l'exécution pendant le nombre de secondes spécifié.

**Paramètres :**
- `<0-300>` - Durée de suspension (secondes)

**Exemples :**
```bash
# Suspendre pendant 5 secondes
sleep 5

# Suspendre pendant 30 secondes
sleep 30
```

### show_mempool - Afficher les Informations de Pool Mémoire
```
show mempool
```

Affiche les informations du pool mémoire DPDK.

**Exemples :**
```bash
show mempool
```

## Descriptions des Éléments de Surveillance

### Informations de Version
- Version de sdplane
- Informations de compilation
- Versions des bibliothèques dépendantes

### Compteur de Boucles
- Nombre de boucles de traitement pour chaque composant
- Utilisé pour la surveillance des performances
- Utilisé pour le calcul PPS (Paquets Par Seconde)

### Informations RCU
- État du mécanisme Read-Copy-Update
- État du traitement de synchronisation
- État de gestion mémoire

### Informations FDB
- État de la table d'adresses MAC
- Adresses MAC apprises
- Informations de vieillissement

### Informations vswitch
- Configuration du commutateur virtuel
- Informations des ports
- Configuration VLAN

### Informations Pool Mémoire
- Mémoire disponible
- Mémoire utilisée
- Statistiques des pools mémoire

## Meilleures Pratiques de Surveillance

### Surveillance Régulière
```bash
# Commandes de surveillance de base
show version
show mempool
show vswitch
show rcu
```

### Surveillance des Performances
```bash
# Surveillance des performances avec compteurs de boucles
show loop-count console pps
show loop-count l2fwd pps
```

### Dépannage
```bash
# Vérification de l'état du système
show fdb
show vswitch
show mempool
```

## Détail des Sorties de Commandes

### Sortie de show version
La commande `show version` affiche des informations critiques :
```
sdplane version X.X.X
Built on: [Date et heure de compilation]
Compiler: gcc version X.X.X
DPDK version: XX.XX.X
Architecture: x86_64
Features: [Liste des fonctionnalités activées]
```

**Informations clés :**
- **Version** : Version actuelle de sdplane
- **Build timestamp** : Quand le binaire a été compilé
- **Compiler info** : Version du compilateur et options utilisées
- **DPDK version** : Version de la bibliothèque DPDK liée
- **Features** : Fonctionnalités compilées (PKTGEN, debug, etc.)

### Sortie de show mempool
```
pool name: [nom_du_pool]
  flags: [drapeaux_du_pool]
  pool: [adresse_mémoire]
  phys_addr: [adresse_physique]
  nb_mem_chunks: [nombre_de_chunks]
  size: [taille_totale]
  populated_size: [taille_populée]
  header_size: [taille_en-tête]
  elt_size: [taille_élément]
  nb_elts: [nombre_total_éléments]
  cache_size: [taille_cache]
  cache_flushthresh: [seuil_vidage_cache]
```

**Métriques importantes :**
- **nb_elts** : Nombre total d'éléments dans le pool
- **cache_size** : Taille du cache par cœur
- **populated_size** : Mémoire effectivement allouée
- **elt_size** : Taille de chaque élément

### Sortie de show vswitch
```
vswitch information:
  name: [nom_vswitch]
  ports: [nombre_ports]
  dpdk_ports: [liste_ports_dpdk]
  tap_ports: [liste_ports_tap]
  vlan_config: [configuration_vlan]
  fdb_entries: [nombre_entrées_fdb]
```

### Sortie de show rcu
```
RCU information:
  readers: [nombre_lecteurs_actifs]
  writers: [nombre_écrivains_actifs]
  grace_period: [période_grâce_actuelle]
  callbacks: [callbacks_en_attente]
```

### Sortie de show fdb
```
FDB entries:
MAC Address       | Port | VLAN | Age | Status
XX:XX:XX:XX:XX:XX | 0    | 100  | 45  | active
XX:XX:XX:XX:XX:XX | 1    | 200  | 12  | active
```

## Surveillance Automatisée et Scripts

### Scripts de Surveillance Automatique
```bash
#!/bin/bash
# /usr/local/bin/sdplane-health-check.sh

LOGFILE="/var/log/sdplane-health.log"
ALERT_EMAIL="admin@example.com"

health_check() {
    local timestamp=$(date "+%Y-%m-%d %H:%M:%S")
    local status="OK"
    local issues=()
    
    echo "[$timestamp] Début vérification santé système" >> $LOGFILE
    
    # Vérifier connectivité CLI
    if ! timeout 5 telnet localhost 9882 </dev/null 2>/dev/null; then
        issues+=("CLI inaccessible")
        status="CRITICAL"
    fi
    
    # Vérifier mémoire
    local mempool_status=$(echo "show mempool" | timeout 10 telnet localhost 9882 2>/dev/null)
    if [ -z "$mempool_status" ]; then
        issues+=("Impossible d'obtenir info mempool")
        status="WARNING"
    fi
    
    # Vérifier performances
    local console_pps=$(echo "show loop-count console pps" | timeout 10 telnet localhost 9882 2>/dev/null | awk '{print $NF}')
    if [ -z "$console_pps" ] || [ "$console_pps" -eq 0 ]; then
        issues+=("Console PPS = 0")
        status="WARNING"
    fi
    
    # Générer rapport
    if [ ${#issues[@]} -gt 0 ]; then
        local issue_list=$(printf ", %s" "${issues[@]}")
        issue_list=${issue_list:2}  # Supprimer la virgule initiale
        echo "[$timestamp] $status - Issues: $issue_list" >> $LOGFILE
        
        if [ "$status" = "CRITICAL" ]; then
            echo "Alerte critique sdplane: $issue_list" | mail -s "CRITICAL: sdplane Health Check Failed" $ALERT_EMAIL
        fi
    else
        echo "[$timestamp] $status - Système opérationnel" >> $LOGFILE
    fi
}

# Exécution
health_check
```

### Surveillance Continue avec Systemd
```ini
# /etc/systemd/system/sdplane-monitor.service
[Unit]
Description=sdplane Health Monitor
After=network.target

[Service]
Type=simple
ExecStart=/usr/local/bin/sdplane-continuous-monitor.sh
Restart=always
RestartSec=30
User=root

[Install]
WantedBy=multi-user.target
```

```bash
#!/bin/bash
# /usr/local/bin/sdplane-continuous-monitor.sh

METRICS_FILE="/var/lib/sdplane/metrics.json"
ALERT_THRESHOLD_PPS=100

continuous_monitor() {
    while true; do
        local timestamp=$(date -u +"%Y-%m-%dT%H:%M:%SZ")
        
        # Collecter métriques via telnet
        exec 3<>/dev/tcp/localhost/9882
        
        # Version
        echo "show version" >&3
        local version=$(timeout 5 cat <&3 | head -1)
        
        # PPS Console
        echo "show loop-count console pps" >&3
        local console_pps=$(timeout 5 cat <&3 | awk '{print $NF}')
        
        # PPS L2FWD
        echo "show loop-count l2fwd pps" >&3  
        local l2fwd_pps=$(timeout 5 cat <&3 | awk '{print $NF}')
        
        echo "quit" >&3
        exec 3<&-
        exec 3>&-
        
        # Créer entrée JSON
        cat << EOF >> $METRICS_FILE
{
  "timestamp": "$timestamp",
  "version": "$version",
  "console_pps": ${console_pps:-0},
  "l2fwd_pps": ${l2fwd_pps:-0},
  "status": "collected"
}
EOF
        
        # Vérifier seuils d'alerte
        if [ "$console_pps" -lt "$ALERT_THRESHOLD_PPS" ]; then
            logger -t sdplane-monitor "WARNING: Console PPS ($console_pps) below threshold ($ALERT_THRESHOLD_PPS)"
        fi
        
        sleep 60
    done
}

continuous_monitor
```

## Surveillance Avancée et Analyse des Performances

### Surveillance de la Mémoire et Hugepages

#### Gestion des Hugepages
Les hugepages sont essentielles pour les performances DPDK :

```bash
# Vérifier la configuration des hugepages système
cat /proc/meminfo | grep -i huge

# Afficher les pools mémoire DPDK détaillés
show mempool

# Surveiller l'utilisation mémoire en temps réel
watch -n 1 "show mempool"
```

**Métriques Clés des Hugepages :**
- **Hugepages Totales** : Nombre total de hugepages configurées
- **Hugepages Libres** : Hugepages disponibles pour allocation
- **Taille Hugepage** : Taille de chaque hugepage (généralement 2MB ou 1GB)
- **Utilisation Pool** : Pourcentage d'utilisation des pools mémoire DPDK

#### Analyse des Pools Mémoire
```bash
# Analyser l'état détaillé des pools mémoire
show mempool

# Informations typiques affichées :
# - Nom du pool mémoire
# - Nombre d'éléments totaux/disponibles/utilisés
# - Taille de chaque élément
# - Statistiques d'allocation/libération
# - Adresses mémoire et zones NUMA
```

### Surveillance de la Version et des Informations de Construction

#### Informations Détaillées de Version
```bash
# Afficher informations complètes de version
show version

# Informations typiques incluent :
# - Version sdplane
# - Date et heure de compilation
# - Version du compilateur utilisé
# - Versions des bibliothèques DPDK
# - Options de configuration activées
# - Informations sur l'architecture cible
```

#### Vérification de la Configuration de Construction
La commande `show version` fournit des détails critiques sur :
- **Optimisations Compilateur** : Niveau d'optimisation (-O0, -O2, -O3)
- **Fonctionnalités Activées** : Support PKTGEN, options debug
- **Bibliothèques Liées** : Versions DPDK, lthread, libpcap
- **Architecture Cible** : Support SSE/AVX, architecture CPU

### Surveillance des Performances en Temps Réel

#### Métriques de Performance Système
```bash
# Surveillance continue des compteurs de boucles
watch -n 1 "show loop-count console pps; show loop-count l2fwd pps"

# Analyse des tendances de performance
while true; do
    echo "=== $(date) ===" >> performance.log
    show loop-count console pps >> performance.log
    show loop-count l2fwd pps >> performance.log
    sleep 10
done
```

#### Calcul des Métriques de Débit
```bash
# Calcul approximatif du débit de paquets
# PPS console = boucles console par seconde
# PPS l2fwd = boucles de transfert L2 par seconde

# Exemple de script de calcul
#!/bin/bash
console_pps=$(show loop-count console pps | awk '{print $NF}')
l2fwd_pps=$(show loop-count l2fwd pps | awk '{print $NF}')

echo "Console: $console_pps boucles/sec"
echo "L2 Forward: $l2fwd_pps boucles/sec"

# Estimation du débit (approximative)
if [ "$l2fwd_pps" -gt 0 ]; then
    echo "Activité de transfert détectée: $l2fwd_pps paquets potentiels/sec"
fi
```

### Techniques d'Analyse et de Dépannage

#### Diagnostic Complet du Système
```bash
# Script de diagnostic système complet
#!/bin/bash
echo "=== DIAGNOSTIC SYSTÈME SDPLANE ==="
echo "Date: $(date)"
echo

echo "=== VERSION ET CONFIGURATION ==="
show version
echo

echo "=== INFORMATIONS VSWITCH ==="
show vswitch
echo

echo "=== BASE DE DONNÉES DE TRANSFERT ==="
show fdb
echo

echo "=== ÉTAT RCU ==="
show rcu
echo

echo "=== POOLS MÉMOIRE ==="
show mempool
echo

echo "=== PERFORMANCES TEMPS RÉEL ==="
echo "Console PPS:"
show loop-count console pps
echo "L2FWD Total:"
show loop-count l2fwd total
```

#### Identification des Problèmes de Performance
```bash
# Surveillance des problèmes de performance
#!/bin/bash

check_performance() {
    local console_pps=$(show loop-count console pps 2>/dev/null | awk '{print $NF}')
    local l2fwd_total=$(show loop-count l2fwd total 2>/dev/null | awk '{print $NF}')
    
    echo "Vérification performance - $(date)"
    
    if [ -z "$console_pps" ] || [ "$console_pps" -eq 0 ]; then
        echo "ALERTE: Console PPS = 0 ou indisponible"
    else
        echo "Console PPS: $console_pps"
    fi
    
    if [ -z "$l2fwd_total" ]; then
        echo "ALERTE: Compteur L2FWD indisponible"
    else
        echo "L2FWD Total: $l2fwd_total"
    fi
}

# Surveillance continue
while true; do
    check_performance
    sleep 30
done
```

### Intégration avec les Systèmes de Surveillance

#### Exportation des Métriques pour Prometheus
```bash
# Script d'exportation des métriques au format Prometheus
#!/bin/bash

export_metrics() {
    local timestamp=$(date +%s)
    
    # Métriques de version
    echo "# HELP sdplane_version_info Version information"
    echo "# TYPE sdplane_version_info gauge"
    echo "sdplane_version_info{version=\"$(show version | head -1)\"} 1 $timestamp"
    
    # Métriques de performance
    local console_pps=$(show loop-count console pps 2>/dev/null | awk '{print $NF}')
    local l2fwd_pps=$(show loop-count l2fwd pps 2>/dev/null | awk '{print $NF}')
    
    if [ -n "$console_pps" ] && [ "$console_pps" != "0" ]; then
        echo "# HELP sdplane_console_pps Console loops per second"
        echo "# TYPE sdplane_console_pps gauge"
        echo "sdplane_console_pps $console_pps $timestamp"
    fi
    
    if [ -n "$l2fwd_pps" ] && [ "$l2fwd_pps" != "0" ]; then
        echo "# HELP sdplane_l2fwd_pps L2 forwarding loops per second"
        echo "# TYPE sdplane_l2fwd_pps gauge"
        echo "sdplane_l2fwd_pps $l2fwd_pps $timestamp"
    fi
}

# Servir les métriques via HTTP (nécessite netcat)
while true; do
    echo -e "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n$(export_metrics)" | nc -l -p 9090 -q 1
done
```

#### Configuration SNMP (Simple Network Management Protocol)
```bash
# Configuration pour l'intégration SNMP
# Fichier: /etc/snmp/snmpd.conf (exemple)

# Script personnalisé pour métriques sdplane
extend .1.3.6.1.4.1.1001.1 sdplane-version /usr/local/bin/sdplane-version.sh
extend .1.3.6.1.4.1.1001.2 sdplane-console-pps /usr/local/bin/sdplane-console-pps.sh
extend .1.3.6.1.4.1.1001.3 sdplane-mempool /usr/local/bin/sdplane-mempool.sh

# Scripts correspondants :
# /usr/local/bin/sdplane-version.sh
#!/bin/bash
telnet localhost 9882 <<EOF | grep -v "^telnet" | tail -n +3 | head -n -1
show version
quit
EOF

# /usr/local/bin/sdplane-console-pps.sh  
#!/bin/bash
telnet localhost 9882 <<EOF | grep -v "^telnet" | tail -n +3 | head -n -1 | awk '{print $NF}'
show loop-count console pps
quit
EOF
```

### Surveillance de l'État du Système et Alerting

#### Système d'Alertes Avancé
```bash
# Système d'alertes avec seuils configurables
#!/bin/bash

# Configuration des seuils
MEMPOOL_USAGE_THRESHOLD=80  # Pourcentage
CONSOLE_PPS_MIN=1          # PPS minimum attendu
RCU_ERROR_THRESHOLD=0      # Nombre d'erreurs RCU tolérées

# Fonctions d'alerte
send_alert() {
    local level=$1
    local message=$2
    local timestamp=$(date)
    
    echo "[$timestamp] $level: $message" | tee -a /var/log/sdplane-alerts.log
    
    # Intégration avec systèmes d'alerte externes
    case $level in
        "CRITICAL")
            # Envoyer notification critique (email, Slack, etc.)
            # curl -X POST webhook_url -d "{'text': '$message'}"
            ;;
        "WARNING")
            # Envoyer avertissement
            ;;
    esac
}

# Vérification de l'état des pools mémoire
check_mempool() {
    local mempool_info=$(show mempool 2>/dev/null)
    
    if [ -z "$mempool_info" ]; then
        send_alert "CRITICAL" "Impossible d'obtenir les informations des pools mémoire"
        return 1
    fi
    
    # Analyser l'utilisation (logique spécifique selon le format de sortie)
    # Cette partie nécessiterait d'être adaptée selon le format exact de show mempool
}

# Vérification de l'état RCU
check_rcu() {
    local rcu_info=$(show rcu 2>/dev/null)
    
    if [ -z "$rcu_info" ]; then
        send_alert "CRITICAL" "Impossible d'obtenir les informations RCU"
        return 1
    fi
    
    # Vérifier les erreurs RCU (logique à adapter selon le format)
    if echo "$rcu_info" | grep -q "error\|fail"; then
        send_alert "WARNING" "Erreurs détectées dans le système RCU"
    fi
}

# Vérification des performances
check_performance() {
    local console_pps=$(show loop-count console pps 2>/dev/null | awk '{print $NF}')
    
    if [ -z "$console_pps" ] || [ "$console_pps" -eq 0 ]; then
        send_alert "CRITICAL" "Console PPS est zéro ou indisponible"
    elif [ "$console_pps" -lt "$CONSOLE_PPS_MIN" ]; then
        send_alert "WARNING" "Console PPS ($console_pps) inférieur au seuil minimum ($CONSOLE_PPS_MIN)"
    fi
}

# Boucle principale de surveillance
main_monitoring_loop() {
    while true; do
        check_mempool
        check_rcu
        check_performance
        
        sleep 60  # Vérifier toutes les minutes
    done
}

# Démarrer la surveillance
main_monitoring_loop
```

#### Configuration de Logs Structurés
```bash
# Configuration de logs au format JSON pour analyse
#!/bin/bash

log_metrics_json() {
    local timestamp=$(date -u +"%Y-%m-%dT%H:%M:%SZ")
    local console_pps=$(show loop-count console pps 2>/dev/null | awk '{print $NF}')
    local l2fwd_total=$(show loop-count l2fwd total 2>/dev/null | awk '{print $NF}')
    local version=$(show version 2>/dev/null | head -1)
    
    cat << EOF | tee -a /var/log/sdplane-metrics.json
{
  "timestamp": "$timestamp",
  "metrics": {
    "console_pps": ${console_pps:-null},
    "l2fwd_total": ${l2fwd_total:-null},
    "version": "$version"
  },
  "status": "active"
}
EOF
}

# Collecte périodique
while true; do
    log_metrics_json
    sleep 30
done
```

## Procédures de Dépannage Avancé

### Problèmes Courants et Solutions

#### 1. Problèmes de Mémoire et Hugepages
```bash
# Diagnostic des problèmes de mémoire
diagnose_memory_issues() {
    echo "=== DIAGNOSTIC MÉMOIRE ==="
    
    # Vérifier la configuration système des hugepages
    echo "Configuration Hugepages Système:"
    cat /proc/meminfo | grep -i huge
    
    # Vérifier les montages hugepages
    echo -e "\nMontages Hugepages:"
    mount | grep hugepage
    
    # Vérifier l'état des pools DPDK
    echo -e "\nPools Mémoire DPDK:"
    show mempool
    
    # Vérifier l'utilisation mémoire globale
    echo -e "\nUtilisation Mémoire Système:"
    free -h
    
    # Recommandations automatiques
    echo -e "\n=== RECOMMANDATIONS ==="
    local free_huge=$(cat /proc/meminfo | grep HugePages_Free | awk '{print $2}')
    local total_huge=$(cat /proc/meminfo | grep HugePages_Total | awk '{print $2}')
    
    if [ "$free_huge" -lt $((total_huge / 10)) ]; then
        echo "⚠️ ATTENTION: Moins de 10% de hugepages libres disponibles"
        echo "   Solution: Redémarrer sdplane ou augmenter le nombre de hugepages"
    fi
}
```

#### 2. Problèmes de Performance
```bash
# Analyse des problèmes de performance
analyze_performance() {
    echo "=== ANALYSE DE PERFORMANCE ==="
    
    # Collecter métriques sur une période
    local samples=5
    local interval=2
    
    echo "Collecte de $samples échantillons (intervalle: ${interval}s)"
    
    for i in $(seq 1 $samples); do
        echo "Échantillon $i:"
        echo "  Console PPS: $(show loop-count console pps | awk '{print $NF}')"
        echo "  L2FWD Total: $(show loop-count l2fwd total | awk '{print $NF}')"
        echo "  Timestamp: $(date)"
        sleep $interval
    done
    
    # Analyse du système
    echo -e "\n=== ANALYSE SYSTÈME ==="
    echo "État vswitch:"
    show vswitch
    
    echo -e "\nÉtat FDB:"
    show fdb
    
    echo -e "\nÉtat RCU:"
    show rcu
}
```

#### 3. Problèmes de Configuration
```bash
# Vérification de la configuration
verify_configuration() {
    echo "=== VÉRIFICATION CONFIGURATION ==="
    
    # Vérifier les arguments configurés
    echo "Arguments configurés:"
    for i in {0..7}; do
        local arg=$(show argv-list $i 2>/dev/null)
        if [ -n "$arg" ] && [ "$arg" != "unset" ]; then
            echo "  argv[$i]: $arg"
        fi
    done
    
    # Vérifier la locale
    echo -e "\nLocale système:"
    locale
    
    # Vérifier la configuration réseau
    echo -e "\nInterfaces réseau:"
    ip link show
    
    echo -e "\nRoutes:"
    ip route show
}
```

### Diagnostic et Résolution des Problèmes Critiques

#### Problèmes de Démarrage
```bash
# Diagnostic de démarrage sdplane
diagnose_startup_issues() {
    echo "=== DIAGNOSTIC DÉMARRAGE SDPLANE ==="
    
    # Vérifier les prérequis système
    echo "1. Vérification prérequis système:"
    
    # Hugepages
    local hugepages_free=$(cat /proc/meminfo | grep HugePages_Free | awk '{print $2}')
    local hugepages_total=$(cat /proc/meminfo | grep HugePages_Total | awk '{print $2}')
    
    if [ "$hugepages_total" -eq 0 ]; then
        echo "❌ ERREUR: Aucune hugepage configurée"
        echo "   Solution: echo 1024 > /sys/kernel/mm/hugepages/hugepages-2048kB/nr_hugepages"
    elif [ "$hugepages_free" -eq 0 ]; then
        echo "⚠️ ATTENTION: Aucune hugepage libre"
        echo "   Solution: Redémarrer les applications utilisant les hugepages"
    else
        echo "✅ Hugepages: $hugepages_free/$hugepages_total libres"
    fi
    
    # Interfaces réseau DPDK
    echo -e "\n2. Vérification interfaces DPDK:"
    if command -v dpdk-devbind.py >/dev/null 2>&1; then
        dpdk-devbind.py --status
    else
        echo "❌ dpdk-devbind.py non trouvé"
    fi
    
    # Permissions
    echo -e "\n3. Vérification permissions:"
    local user=$(whoami)
    if [ "$user" != "root" ]; then
        echo "⚠️ ATTENTION: Non exécuté en tant que root"
        echo "   Solution: sudo ./sdplane/sdplane"
    else
        echo "✅ Permissions root OK"
    fi
    
    # Ports en cours d'utilisation
    echo -e "\n4. Vérification ports:"
    if netstat -ln | grep -q ":9882"; then
        echo "❌ ERREUR: Port 9882 déjà utilisé"
        echo "   Solution: pkill sdplane ou changer de port"
    else
        echo "✅ Port CLI 9882 disponible"
    fi
}
```

#### Problèmes de Performances Réseau
```bash
# Analyse des performances réseau détaillée
analyze_network_performance() {
    echo "=== ANALYSE PERFORMANCES RÉSEAU ==="
    
    # Statistiques détaillées sur période
    local duration=10
    echo "Collecte sur $duration secondes..."
    
    local start_console=$(show loop-count console total 2>/dev/null | awk '{print $NF}')
    local start_l2fwd=$(show loop-count l2fwd total 2>/dev/null | awk '{print $NF}')
    local start_time=$(date +%s)
    
    sleep $duration
    
    local end_console=$(show loop-count console total 2>/dev/null | awk '{print $NF}')
    local end_l2fwd=$(show loop-count l2fwd total 2>/dev/null | awk '{print $NF}')
    local end_time=$(date +%s)
    
    local actual_duration=$((end_time - start_time))
    local console_rate=$(((end_console - start_console) / actual_duration))
    local l2fwd_rate=$(((end_l2fwd - start_l2fwd) / actual_duration))
    
    echo "Résultats sur $actual_duration secondes:"
    echo "  Console: $console_rate boucles/sec"
    echo "  L2FWD: $l2fwd_rate boucles/sec"
    
    # Analyse et recommandations
    if [ "$console_rate" -lt 100 ]; then
        echo "⚠️ Console rate faible - vérifier la charge système"
    fi
    
    if [ "$l2fwd_rate" -eq 0 ]; then
        echo "❌ Aucune activité L2 forwarding détectée"
        echo "   Vérifier:"
        echo "   - Configuration des ports"
        echo "   - Trafic entrant"
        echo "   - État FDB"
    fi
    
    # Vérifications supplémentaires
    echo -e "\n=== VÉRIFICATIONS COMPLÉMENTAIRES ==="
    
    # État interfaces
    echo "États des interfaces:"
    ip link show | grep -E "(UP|DOWN)"
    
    # Statistiques interfaces
    echo -e "\nStatistiques interfaces réseau:"
    cat /proc/net/dev | head -2
    cat /proc/net/dev | grep -v -E "(lo:|Inter-|face)"
}
```

#### Récupération Automatique d'Incidents
```bash
# Système de récupération automatique
auto_recovery_system() {
    local max_recovery_attempts=3
    local recovery_count=0
    local recovery_log="/var/log/sdplane-recovery.log"
    
    while [ $recovery_count -lt $max_recovery_attempts ]; do
        # Tester la santé du système
        if health_check_minimal; then
            echo "$(date): Système opérationnel" >> $recovery_log
            break
        fi
        
        recovery_count=$((recovery_count + 1))
        echo "$(date): Tentative récupération $recovery_count/$max_recovery_attempts" >> $recovery_log
        
        # Tentatives de récupération progressives
        case $recovery_count in
            1)
                # Récupération douce - redémarrer CLI seulement
                echo "$(date): Tentative récupération CLI" >> $recovery_log
                pkill -HUP sdplane 2>/dev/null
                ;;
            2)
                # Récupération moyenne - vider caches
                echo "$(date): Vidage caches système" >> $recovery_log
                sync
                echo 1 > /proc/sys/vm/drop_caches
                ;;
            3)
                # Récupération complète - redémarrage application
                echo "$(date): Redémarrage complet sdplane" >> $recovery_log
                systemctl restart sdplane 2>/dev/null || {
                    pkill sdplane
                    sleep 5
                    /usr/local/bin/sdplane &
                }
                ;;
        esac
        
        # Attendre avant nouvelle vérification
        sleep 30
    done
    
    if [ $recovery_count -ge $max_recovery_attempts ]; then
        echo "$(date): ÉCHEC - Récupération impossible après $max_recovery_attempts tentatives" >> $recovery_log
        # Alerte critique
        echo "CRITICAL: sdplane recovery failed after $max_recovery_attempts attempts" | \
            mail -s "sdplane Recovery Failed" admin@example.com
    fi
}

health_check_minimal() {
    # Test rapide de santé
    timeout 5 telnet localhost 9882 </dev/null 2>/dev/null && \
    timeout 10 echo "show version" | telnet localhost 9882 2>/dev/null | grep -q "sdplane"
}
```

## Surveillance Spécialisée par Environnement

### Environnement de Production
```bash
# Configuration surveillance production
production_monitoring() {
    # Métriques critiques haute fréquence
    while true; do
        local timestamp=$(date -u +"%Y-%m-%dT%H:%M:%SZ")
        local console_pps=$(echo "show loop-count console pps" | telnet localhost 9882 2>/dev/null | awk '{print $NF}')
        local l2fwd_pps=$(echo "show loop-count l2fwd pps" | telnet localhost 9882 2>/dev/null | awk '{print $NF}')
        
        # Log haute fréquence pour analyse
        echo "$timestamp,$console_pps,$l2fwd_pps" >> /var/log/sdplane-hf-metrics.csv
        
        # Alertes temps réel
        if [ "$console_pps" -lt 50 ]; then
            logger -p daemon.crit "sdplane: Console PPS critique ($console_pps)"
        fi
        
        sleep 10  # Surveillance toutes les 10 secondes en production
    done
}
```

### Environnement de Développement
```bash
# Surveillance développement avec debug
development_monitoring() {
    echo "=== MODE SURVEILLANCE DÉVELOPPEMENT ==="
    
    # Informations debug étendues
    echo "Arguments configurés:"
    for i in {0..7}; do
        local arg=$(show argv-list $i 2>/dev/null)
        if [ -n "$arg" ]; then
            echo "  argv[$i]: $arg"
        fi
    done
    
    # Surveillance mémoire détaillée
    echo -e "\n=== SURVEILLANCE MÉMOIRE DÉTAILLÉE ==="
    show mempool
    
    # Surveillance continue avec détails
    while true; do
        clear
        echo "=== SURVEILLANCE TEMPS RÉEL - $(date) ==="
        echo
        
        echo "Performance:"
        echo "  Console PPS: $(show loop-count console pps | awk '{print $NF}')"
        echo "  L2FWD Total: $(show loop-count l2fwd total | awk '{print $NF}')"
        
        echo -e "\nÉtat système:"
        echo "  RCU: $(show rcu | head -1)"
        echo "  FDB entries: $(show fdb | wc -l) entrées"
        
        sleep 5
    done
}
```

## Guide de Performance et Optimisation

### Métriques de Performance Clés
```bash
# Surveillance des métriques de performance critiques
monitor_performance_kpis() {
    echo "=== INDICATEURS CLÉS DE PERFORMANCE ==="
    
    # Débit de traitement des paquets
    local console_pps=$(show loop-count console pps | awk '{print $NF}')
    local l2fwd_pps=$(show loop-count l2fwd pps | awk '{print $NF}')
    
    echo "Débit de traitement:"
    echo "  Console: $console_pps boucles/sec"
    echo "  L2FWD: $l2fwd_pps boucles/sec"
    
    # Utilisation mémoire
    echo -e "\nUtilisation mémoire:"
    local mempool_info=$(show mempool)
    echo "$mempool_info" | grep -E "(pool|size|elts)"
    
    # État de synchronisation RCU
    echo -e "\nSynchronisation RCU:"
    show rcu
    
    # État de la base de données de forwarding
    echo -e "\nBase de données FDB:"
    local fdb_count=$(show fdb | grep -c ":")
    echo "  Entrées actives: $fdb_count"
    
    # Calcul de ratios de performance
    if [ "$l2fwd_pps" -gt 0 ] && [ "$console_pps" -gt 0 ]; then
        local efficiency=$((l2fwd_pps * 100 / console_pps))
        echo -e "\nEfficacité L2FWD: ${efficiency}%"
    fi
}
```

### Optimisation des Performances
```bash
# Suggestions d'optimisation basées sur les métriques
performance_optimization_suggestions() {
    echo "=== SUGGESTIONS D'OPTIMISATION ==="
    
    local console_pps=$(show loop-count console pps | awk '{print $NF}')
    local l2fwd_pps=$(show loop-count l2fwd pps | awk '{print $NF}')
    
    # Analyse des performances console
    if [ "$console_pps" -lt 100 ]; then
        echo "⚠️ PERFORMANCE CONSOLE FAIBLE:"
        echo "  - Vérifier la charge CPU système"
        echo "  - Considérer l'isolation de CPU (isolcpus)"
        echo "  - Vérifier les interruptions (cat /proc/interrupts)"
    fi
    
    # Analyse des performances L2 forwarding
    if [ "$l2fwd_pps" -eq 0 ]; then
        echo "⚠️ AUCUNE ACTIVITÉ L2 FORWARDING:"
        echo "  - Vérifier la configuration des ports"
        echo "  - Vérifier l'injection de trafic"
        echo "  - Contrôler l'état des interfaces DPDK"
    fi
    
    # Suggestions hugepages
    local hugepages_free=$(cat /proc/meminfo | grep HugePages_Free | awk '{print $2}')
    local hugepages_total=$(cat /proc/meminfo | grep HugePages_Total | awk '{print $2}')
    
    if [ "$hugepages_total" -gt 0 ]; then
        local hugepage_usage=$(((hugepages_total - hugepages_free) * 100 / hugepages_total))
        if [ "$hugepage_usage" -gt 80 ]; then
            echo "⚠️ UTILISATION HUGEPAGES ÉLEVÉE (${hugepage_usage}%):"
            echo "  - Considérer l'augmentation des hugepages"
            echo "  - echo 2048 > /sys/kernel/mm/hugepages/hugepages-2048kB/nr_hugepages"
        fi
    fi
}
```

## Emplacement de Définition

Ces commandes sont définies dans le fichier suivant :
- `sdplane/sdplane.c`

## Sujets Associés

- [Gestion des Ports & Statistiques](port-management.md)
- [Gestion des Workers & lcore](worker-lcore-thread-management.md)
- [Informations des Threads](worker-lcore-thread-management.md)