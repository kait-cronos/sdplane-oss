#!/usr/bin/env bash

if [ -z "$1" ]; then
    echo "usage: $0 {output-nic-name}"
    exit 1
fi
PHY_DEV="$1"
ip link set dev $PHY_DEV up

ip netns add seg6gen-1
ip netns add seg6gen-2

ip link add link "$PHY_DEV" name "$PHY_DEV.300" type vlan id 300
ip link set "$PHY_DEV.300" netns seg6gen-1

ip link add link "$PHY_DEV" name "$PHY_DEV.400" type vlan id 400
ip link set "$PHY_DEV.400" netns seg6gen-2

ip netns exec seg6gen-1 bash -lc '
  ip link set up dev lo
  ip addr add 10.0.0.1/32 dev lo
  ip link set up dev "$1"
  ip -6 addr add fc00:a::1/64 dev "$1"
  ip sr tunsrc set fc00:a::1
  ip -6 route add default via fc00:a::2
  ip route add 10.0.1.0/24 encap seg6 mode encap segs fc00:1::1,fc00:2::1 dev "$1"
  ping 10.0.1.1 > /dev/null 2>&1 &
' -- "$PHY_DEV.300"

ip netns exec seg6gen-2 bash -lc '
  ip link set up dev lo
  ip link set up dev "$1"
  ip -6 addr add fc00:b::1/64 dev "$1"
  ip sr tunsrc set fc00:b::1
  ip -6 route add default via fc00:b::2
  ip route add 10.0.2.0/24 encap seg6 mode encap segs fc00:1::1,fc00:2::1 dev "$1"
' -- "$PHY_DEV.400"
