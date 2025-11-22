#!/bin/bash

# === 設定項目 ===
# このセクションの値は、環境に合わせて変更してください。

R1_NS="r1"
R1_PHY_IF="enp2s0"
R1_IPV4_ADDR="198.18.1.10/24"
R1_IPV6_ADDR="fd00:1::10/64"
R1_IPV4_LO="10.1.0.10/32"
R1_IPV6_LO="2001:1::10/128"

R2_NS="r2"
R2_PHY_IF="enp3s0"
R2_IPV4_ADDR="198.18.2.10/24"
R2_IPV6_ADDR="fd00:2::10/64"
R2_IPV4_LO="10.2.0.10/32"
R2_IPV6_LO="2001:2::10/128"

# === 関数定義 ===

# ------------------------------------------------------------------------------
# 設定を作成する関数
# ------------------------------------------------------------------------------
apply_config() {
  echo "==> setup namespace start"

  echo "==> create namespace: $R1_NS, $R2_NS"
  ip netns add $R1_NS
  ip netns add $R2_NS

  echo "==> move to physical Interface"
  ip link set ${R1_PHY_IF} netns $R1_NS
  ip link set ${R2_PHY_IF} netns $R2_NS

  echo "==> enable Interface"
  ip netns exec $R1_NS ip link set ${R1_PHY_IF} up
  ip netns exec $R2_NS ip link set ${R2_PHY_IF} up
  ip netns exec $R1_NS ip link set lo up
  ip netns exec $R2_NS ip link set lo up

  echo "==> set Interface IP address"
  ip netns exec $R1_NS ip a add $R1_IPV4_ADDR dev ${R1_PHY_IF}
  ip netns exec $R2_NS ip a add $R2_IPV4_ADDR dev ${R2_PHY_IF}
  ip netns exec $R1_NS ip -6 a add $R1_IPV6_ADDR dev ${R1_PHY_IF}
  ip netns exec $R2_NS ip -6 a add $R2_IPV6_ADDR dev ${R2_PHY_IF}

  echo "==> set Loopback IP address"
  ip netns exec $R1_NS ip a add $R1_IPV4_LO dev lo
  ip netns exec $R2_NS ip a add $R2_IPV4_LO dev lo
  ip netns exec $R1_NS ip -6 a add $R1_IPV6_LO dev lo
  ip netns exec $R2_NS ip -6 a add $R2_IPV6_LO dev lo

  echo "==> enable IP forwardhing"
  ip netns exec $R1_NS sysctl -w net.ipv4.ip_forward=1
  ip netns exec $R2_NS sysctl -w net.ipv4.ip_forward=1
  ip netns exec $R1_NS sysctl -w net.ipv6.conf.all.forwarding=1
  ip netns exec $R2_NS sysctl -w net.ipv6.conf.all.forwarding=1

  echo "==> setup namespace complete"

  echo ""

  echo "==> setup frr start"

  echo "==> setup frr config"
  mkdir -p /etc/frr/$R1_NS
  cp ./$R1_NS/daemons /etc/frr/$R1_NS/daemons
  cp ./$R1_NS/frr.conf /etc/frr/$R1_NS/frr.conf
  cp ./$R1_NS/vtysh.conf /etc/frr/$R1_NS/vtysh.conf
  chown -R frr:frr /etc/frr/$R1_NS
  chmod 640 /etc/frr/$R1_NS/*

  mkdir -p /etc/frr/$R2_NS
  cp ./$R2_NS/daemons /etc/frr/$R2_NS/daemons
  cp ./$R2_NS/frr.conf /etc/frr/$R2_NS/frr.conf
  cp ./$R2_NS/vtysh.conf /etc/frr/$R2_NS/vtysh.conf
  chown -R frr:frr /etc/frr/$R2_NS
  chmod 640 /etc/frr/$R2_NS/*

  echo "==> start frr"
  /usr/lib/frr/frrinit.sh start "$R1_NS"
  /usr/lib/frr/frrinit.sh start "$R2_NS"
}

# ------------------------------------------------------------------------------
# 設定を削除する関数
# ------------------------------------------------------------------------------
delete_config() {
  echo "* cleanup start"

  echo "==> cleanup frr config"
  /usr/lib/frr/frrinit.sh stop "$R1_NS"
  /usr/lib/frr/frrinit.sh stop "$R2_NS"
  rm -rf /etc/frr/$R1_NS || true
  rm -rf /etc/frr/$R2_NS || true

  echo "==> cleanup namespace: $R1_NS, $R2_NS"
  ip netns exec $R1_NS ip link set $R1_PHY_IF netns 1
  ip netns exec $R2_NS ip link set $R2_PHY_IF netns 1

  ip link set $R1_PHY_IF up 2>/dev/null || true
  ip link set $R2_PHY_IF up 2>/dev/null || true

  ip netns del $R1_NS 2>/dev/null || true
  ip netns del $R2_NS 2>/dev/null || true

  echo "* cleanup completed"
}

# ------------------------------------------------------------------------------
# 使い方を表示する関数
# ------------------------------------------------------------------------------
usage() {
  echo "使い方: sudo $0 [up|down]"
  echo ""
  echo "  up    : ネットワーク設定を作成します。"
  echo "  down  : ネットワーク設定を削除します。"
  echo ""
  exit 1
}

# namespace用のvtysh: ip netns exec r1 vtysh -N r1

# === メイン処理 ===

# コマンドが失敗した場合、スクリプトを直ちに終了する
set -e

# root権限で実行されているかチェック
if [ "$(id -u)" -ne 0 ]; then
  echo "エラー: このスクリプトはroot権限で実行する必要があります。" >&2
  exit 1
fi

# 第一引数 ($1) に基づいて処理を分岐
case "$1" in
  up|apply)
    apply_config
    ;;
  down|delete)
    delete_config
    ;;
  *)
    # 引数がない、または不正な場合は使い方を表示
    usage
    ;;
esac

exit 0
