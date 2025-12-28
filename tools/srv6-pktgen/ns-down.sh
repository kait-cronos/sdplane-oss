#!/usr/bin/env bash
NS="seg6gen-1 seg6gen-2"

for ns in $NS; do
	ip netns pids "$ns" | xargs -r kill
	DEVS=$(ip -n "$ns" -j link show 2>/dev/null | jq -r '.[] | select(.ifname != "lo") | .ifname')
	for dev in $DEVS; do
		ip -n "$ns" link del "$dev" 2>/dev/null || true
	done
	ip netns del "$ns" || true
done
