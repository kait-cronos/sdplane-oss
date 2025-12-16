#!/usr/bin/env bash
if [ -z "$1" ]; then
    echo "usage: $0 {output-nic-name}"
    exit 1
fi

ip netns add seg6gen
ip link set $1 netns seg6gen

ip netns exec seg6gen bash -xlc '
  ip link set up dev lo
  ip addr add 10.0.0.1/32 dev lo
  ip link set up dev $1
  ip -6 addr add fc00:a::1/64 dev "$1"
  ip sr tunsrc set fc00:a::1
  ip -6 route add default via fc00:a::2
  ip route add 10.0.1.0/24 encap seg6 mode encap segs fc00:1::1,fc00:2::1 dev "$1"
  ping 10.0.1.1 &
' -- "$1"
