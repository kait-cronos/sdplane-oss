#!/usr/bin/env bash
set -euo pipefail

REMOTE_USER="${REMOTE_USER:-$(id -un)}"

DEV34="${REMOTE_USER}@192.168.18.34"
DEV35="${REMOTE_USER}@192.168.18.35"
TRAFFIC_GEN="${REMOTE_USER}@192.168.18.21"

GROUP="239.1.1.1"
PORT="5000"
PACKETS=100

RX_IF="enp1s0f0"
DEV34_TEST_IP="10.33.31.34"
TRAFFIC_GEN_IF="enp2s0.2031"
TRAFFIC_GEN_IP="10.33.31.10"
DPDK_PCIS=("0000:01:00.0" "0000:01:00.1" "0000:03:00.0")

CAPTURE_SECONDS=30
JOIN_SECONDS=20

TEST_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_DIR="$(cd "$TEST_DIR/../.." && pwd)"
SDPLANE_BIN="$REPO_DIR/build/sdplane/sdplane"
SDPLANE_CONF="$TEST_DIR/sdplane.conf"
SDPLANE_LOG="/tmp/sdplane-m0-physical.log"
RX1_CAPTURE="$(mktemp /tmp/mcast-m0-rx1.XXXXXX.log)"
RX2_CAPTURE="$(mktemp /tmp/mcast-m0-rx2.XXXXXX.log)"

sdplane_pid=""
capture34_pid=""
capture35_pid=""
join_pid=""

cleanup() {
  unset dev34_sudo_password dev35_sudo_password 2>/dev/null || true

  for pid in "$capture34_pid" "$capture35_pid" "$join_pid"; do
    if [[ -n "$pid" ]]; then
      kill "$pid" 2>/dev/null || true
    fi
  done

  if [[ -n "$sdplane_pid" ]]; then
    sudo kill "$sdplane_pid" 2>/dev/null || true
    for _ in $(seq 1 50); do
      if ! ps -p "$sdplane_pid" >/dev/null 2>&1; then
        break
      fi
      sleep 0.1
    done
    if ps -p "$sdplane_pid" >/dev/null 2>&1; then
      sudo kill -KILL "$sdplane_pid" 2>/dev/null || true
    fi
    sleep 2
  fi

  rm -f "$RX1_CAPTURE" "$RX2_CAPTURE" "$SDPLANE_LOG"
}
trap cleanup EXIT INT TERM

verify_sdplane_ready() {
  if grep -Eq 'port\[0\]:.*01:00\.0.*[[:space:]]up[[:space:]]' "$SDPLANE_LOG" &&
     grep -Eq 'port\[1\]:.*01:00\.1.*[[:space:]]up[[:space:]]' "$SDPLANE_LOG" &&
     grep -Eq 'port\[2\]:.*03:00\.0.*[[:space:]]up[[:space:]]' "$SDPLANE_LOG" &&
     grep -q 'startup-config> set vswitch 1 port 0 untag' "$SDPLANE_LOG" &&
     grep -q 'startup-config> set vswitch 1 port 1 untag' "$SDPLANE_LOG" &&
     grep -q 'startup-config> set vswitch 1 port 2 untag' "$SDPLANE_LOG"; then
    return 0
  fi

  echo "sdplane ports or vswitch are not ready" >&2
  tail -n 100 "$SDPLANE_LOG" >&2 || true
  return 1
}

wait_for_capture() {
  local host="$1"
  local ssh_pid="$2"

  for _ in $(seq 1 60); do
    if ssh "$host" \
      "pgrep -a -x tcpdump | grep -F -- 'dst host ${GROUP} and dst port ${PORT}' >/dev/null"; then
      return 0
    fi
    if ! kill -0 "$ssh_pid" 2>/dev/null; then
      wait "$ssh_pid" || true
      echo "tcpdump failed to start on ${host}" >&2
      return 1
    fi
    sleep 1
  done

  echo "timeout waiting for tcpdump on ${host}" >&2
  return 1
}

if [[ ! -x "$SDPLANE_BIN" ]]; then
  echo "sdplane binary not found: $SDPLANE_BIN" >&2
  echo "Build it first with: cd $REPO_DIR/build && make -j4" >&2
  exit 1
fi

if pgrep -x sdplane >/dev/null; then
  echo "sdplane is already running; stop it before this test" >&2
  exit 1
fi

if ! command -v dpdk-devbind.py >/dev/null; then
  echo "dpdk-devbind.py not found" >&2
  exit 1
fi

echo "[1/6] Resetting DPDK NICs and starting sdplane"
sudo -v
dpdk_devbind="$(command -v dpdk-devbind.py)"
sudo "$dpdk_devbind" --unbind "${DPDK_PCIS[@]}"
sudo "$dpdk_devbind" --bind=vfio-pci "${DPDK_PCIS[@]}"
sleep 3

dpdk_status="$(dpdk-devbind.py --status)"
for pci in "${DPDK_PCIS[@]}"; do
  pci_pattern="${pci//./\.}"
  if ! grep -Eq "^${pci_pattern} .*drv=vfio-pci" <<<"$dpdk_status"; then
    echo "${pci} is not bound to vfio-pci" >&2
    echo "Bind all three test NICs before running this test." >&2
    exit 1
  fi
done

if ! ssh "$DEV34" \
  "test \"\$(cat /sys/class/net/${RX_IF}/carrier)\" = 1 && ip -4 -o addr show dev ${RX_IF} | grep -q '${DEV34_TEST_IP}/'"; then
  echo "receiver 1 is not ready: ${DEV34} ${RX_IF} ${DEV34_TEST_IP}" >&2
  exit 1
fi

if ! ssh "$DEV35" \
  "test \"\$(cat /sys/class/net/${RX_IF}/carrier)\" = 1"; then
  echo "receiver 2 link is not ready: ${DEV35} ${RX_IF}" >&2
  exit 1
fi

if ! ssh "$TRAFFIC_GEN" \
  "test \"\$(cat /sys/class/net/${TRAFFIC_GEN_IF}/carrier)\" = 1 && ip -d link show ${TRAFFIC_GEN_IF} | grep -q 'id 2031' && ip -4 -o addr show dev ${TRAFFIC_GEN_IF} | grep -q '${TRAFFIC_GEN_IP}/'"; then
  echo "traffic generator is not ready: ${TRAFFIC_GEN} ${TRAFFIC_GEN_IF} ${TRAFFIC_GEN_IP}" >&2
  exit 1
fi

sudo "$SDPLANE_BIN" -f "$SDPLANE_CONF" >"$SDPLANE_LOG" 2>&1 &

for _ in $(seq 1 20); do
  sdplane_pid="$(pgrep -n -x sdplane || true)"
  if [[ -n "$sdplane_pid" ]]; then
    break
  fi
  sleep 0.5
done

if [[ -z "$sdplane_pid" ]]; then
  echo "sdplane failed to start; see $SDPLANE_LOG" >&2
  tail -n 40 "$SDPLANE_LOG" >&2 || true
  exit 1
fi

for _ in $(seq 1 60); do
  if grep -q 'startup_config: terminating\.' "$SDPLANE_LOG"; then
    break
  fi
  if ! ps -p "$sdplane_pid" -o comm= 2>/dev/null | grep -qx 'sdplane'; then
    echo "sdplane stopped during startup" >&2
    tail -n 60 "$SDPLANE_LOG" >&2 || true
    exit 1
  fi
  sleep 0.5
done

if ! grep -q 'startup_config: terminating\.' "$SDPLANE_LOG"; then
  echo "timeout waiting for sdplane startup configuration" >&2
  tail -n 60 "$SDPLANE_LOG" >&2 || true
  exit 1
fi

verify_sdplane_ready

sleep 5

echo "[2/6] Starting packet counters"
read -r -s -p "sudo password for receiver 1 (.34): " dev34_sudo_password
echo
read -r -s -p "sudo password for receiver 2 (.35): " dev35_sudo_password
echo

printf '%s\n' "$dev34_sudo_password" | ssh "$DEV34" \
  "sudo -S -p '' sysctl -w net.ipv4.conf.${RX_IF}.force_igmp_version=2 >/dev/null"

printf '%s\n' "$dev34_sudo_password" | ssh "$DEV34" \
  "sudo -S -p '' timeout -s INT ${CAPTURE_SECONDS}s tcpdump -l -nn -ni ${RX_IF} -c ${PACKETS} 'udp and dst host ${GROUP} and dst port ${PORT}'" \
  >"$RX1_CAPTURE" &
capture34_pid=$!

printf '%s\n' "$dev35_sudo_password" | ssh "$DEV35" \
  "sudo -S -p '' timeout -s INT ${CAPTURE_SECONDS}s tcpdump -l -nn -ni ${RX_IF} -c ${PACKETS} 'udp and dst host ${GROUP} and dst port ${PORT}'" \
  >"$RX2_CAPTURE" &
capture35_pid=$!

unset dev34_sudo_password dev35_sudo_password

wait_for_capture "$DEV34" "$capture34_pid"
wait_for_capture "$DEV35" "$capture35_pid"

sleep 2

echo "[3/6] Joining multicast group on receiver 1"
ssh "$DEV34" \
  "timeout ${JOIN_SECONDS}s python3 -u - '${GROUP}' '${PORT}' '${DEV34_TEST_IP}'" <<'PY' &
import socket
import sys

group = sys.argv[1]
port = int(sys.argv[2])
interface_ip = sys.argv[3]

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
sock.bind(("", port))
membership = socket.inet_aton(group) + socket.inet_aton(interface_ip)
sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, membership)

print(f"joined {group} on {interface_ip}", flush=True)
while True:
    sock.recvfrom(65535)
PY
join_pid=$!

sleep 1
if ! kill -0 "$join_pid" 2>/dev/null; then
  wait "$join_pid" || true
  echo "receiver 1 failed to join ${GROUP}" >&2
  exit 1
fi
sleep 1

echo "[4/6] Sending ${PACKETS} packets"
ssh "$TRAFFIC_GEN" \
  python3 - "$GROUP" "$PORT" "$TRAFFIC_GEN_IP" "$PACKETS" "$TRAFFIC_GEN_IF" <<'PY'
import socket
import sys
import time

group = sys.argv[1]
port = int(sys.argv[2])
source_ip = sys.argv[3]
packets = int(sys.argv[4])
interface = sys.argv[5]

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((source_ip, 0))
sock.setsockopt(
    socket.SOL_SOCKET,
    socket.SO_BINDTODEVICE,
    (interface + "\0").encode(),
)
sock.setsockopt(
    socket.IPPROTO_IP,
    socket.IP_MULTICAST_IF,
    socket.inet_aton(source_ip),
)
sock.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, 1)

for number in range(packets):
    sock.sendto(f"m0-physical-{number}".encode(), (group, port))
    time.sleep(0.01)

print(f"sent {packets} packets")
PY

echo "[5/6] Waiting for packet counters"
wait "$capture34_pid" || true
capture34_pid=""
wait "$capture35_pid" || true
capture35_pid=""
wait "$join_pid" || true
join_pid=""

echo "[6/6] Checking results"
rx1_count="$(awk -v target="> ${GROUP}.${PORT}: UDP," \
  'index($0, target) { count++ } END { print count + 0 }' "$RX1_CAPTURE")"
rx2_count="$(awk -v target="> ${GROUP}.${PORT}: UDP," \
  'index($0, target) { count++ } END { print count + 0 }' "$RX2_CAPTURE")"

result="FAIL"
if [[ "$rx1_count" -eq "$PACKETS" && \
      "$rx2_count" -eq "$PACKETS" ]]; then
  result="PASS"
fi

{
  echo "# Test result"
  echo
  echo "- Sent: $PACKETS"
  echo "- rx1 joined: yes"
  echo "- rx1 received: $rx1_count"
  echo "- rx2 joined: no"
  echo "- rx2 received: $rx2_count"
}

if [[ "$result" != "PASS" ]]; then
  exit 1
fi
