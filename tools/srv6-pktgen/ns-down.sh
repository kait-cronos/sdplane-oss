#!/usr/bin/env bash
NS="seg6gen"

ip netns pids "$NS" | xargs -r kill
for dev in $(ip -n "$NS" -o link show | awk -F': ' '{print $2}' | grep -v "lo"); do
    ip -n "$NS" link set "$dev" netns 1
done
ip netns del "$NS" || true
