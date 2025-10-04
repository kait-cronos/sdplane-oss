#!/bin/sh

set -e

cmd=""

print_cmd_and_exec() {
  echo "# $cmd"
  eval $cmd
  echo ""
}

echo "*******"
cmd="lsb_release -a"
print_cmd_and_exec

echo "*******"
cmd="lscpu"
print_cmd_and_exec

echo "*******"
cmd="free -h"
print_cmd_and_exec

echo "*******"
cmd="dmidecode -t system"
print_cmd_and_exec

echo "*******"
cmd="lspci"
print_cmd_and_exec

echo "*******"
cmd="df -h"
print_cmd_and_exec

echo "*******"
cmd="dpkg -l"
print_cmd_and_exec

echo "*******"
cmd="pkg-config --modversion libdpdk"
print_cmd_and_exec

echo "*******"
cmd="cat /etc/sdplane/sdplane.conf"
print_cmd_and_exec

# echo "*******"
# cmd="ip link show"
# print_cmd_and_exec

echo "*******"
cmd="ip addr show"
print_cmd_and_exec

echo "*******"
cmd="ip route show"
print_cmd_and_exec

