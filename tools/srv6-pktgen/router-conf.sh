#!/usr/bin/env bash
set -e

ip -6 addr add dev rif300 fc00:a::2/64
ip -6 addr add dev rif400 fc00:b::2/64
ip -6 route add default via fc00:b::1
ip -6 route add fc00:1::1/128 encap seg6local action End dev lo