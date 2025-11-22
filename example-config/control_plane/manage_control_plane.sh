#!/bin/bash

# === 設定項目 ===
# --- インターフェース 1 ---
TAP_IF1="rif1"
VLAN_ID1="1"
IPV4_ADDR1="198.18.1.1/24"
IPV6_ADDR1="fd00:1::1/64"

# --- インターフェース 2 ---
TAP_IF2="rif2"
VLAN_ID2="2"
IPV4_ADDR2="198.18.2.1/24"
IPV6_ADDR2="fd00:2::1/64"

# === 関数定義 ===

# ------------------------------------------------------------------------------
# 設定を作成する関数
# ------------------------------------------------------------------------------
apply_config() {
  echo "* setup tap dev start"

  echo "==> set IP address: $TAP_IF1 $TAP_IF2"
  ip a add $IPV4_ADDR1 dev $TAP_IF1
  ip -6 a add $IPV6_ADDR1 dev $TAP_IF1
  ip a add $IPV4_ADDR2 dev $TAP_IF2
  ip -6 a add $IPV6_ADDR2 dev $TAP_IF2

  echo "* setup tap dev completed"

  echo ""

  echo "* setup frr start"

  echo "==> setup frr config"
  mkdir -p /etc/frr/sdplane
  cp ./daemons /etc/frr/sdplane/daemons
  cp ./frr.conf /etc/frr/sdplane/frr.conf
  cp ./vtysh.conf /etc/frr/sdplane/vtysh.conf
  chown -R frr:frr /etc/frr/sdplane
  chmod 640 /etc/frr/sdplane/*

  echo "==> start frr"
  /usr/lib/frr/frrinit.sh start "sdplane"

  echo "* setup frr completed"
}

# ------------------------------------------------------------------------------
# 設定を削除する関数
# ------------------------------------------------------------------------------
delete_config() {
  echo "* cleanup start"

  echo "==> cleanup frr config"
  /usr/lib/frr/frrinit.sh stop "sdplane"
  rm -rf /etc/frr/sdplane || true

  echo "==> cleanup: $IPV4_ADDR1 $IPV6_ADDR1" 
  ip a del $IPV4_ADDR1 dev $TAP_IF1 || true
  ip -6 a del $IPV6_ADDR1 dev $TAP_IF1 || true

  echo "==> cleanup: $IPV4_ADDR2 $IPV6_ADDR2"
  ip a del $IPV4_ADDR2 dev $TAP_IF2 || true
  ip -6 a del $IPV6_ADDR2 dev $TAP_IF2 || true

  echo "* cleanup completed"
}

# ------------------------------------------------------------------------------
# 使い方を表示する関数
# ------------------------------------------------------------------------------
usage() {
    echo "usage: sudo $0 [up|down]"
    echo ""
    echo "  up    : setup tap dev and frr config"
    echo "  down  : cleanup"
    echo ""
    exit 1
}


# === メイン処理 ===

# コマンドが失敗した場合、スクリプトを直ちに終了する
set -e

# root権限で実行されているかチェック
if [ "$(id -u)" -ne 0 ]; then
  echo "error: must be sudo user" >&2
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
