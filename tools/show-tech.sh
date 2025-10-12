#!/bin/sh

#set -e

cmd=""

print_cmd_and_exec() {
  echo "# $cmd"
  eval $cmd
  echo ""
}

dir=/var/tmp
host=`hostname`
machine=`sh config.guess`
date=`date "+%F-%T"`
filename=`echo show-tech-${host}-${date}.txt`

echo "saving in ..."
echo $dir/$filename

{

echo $filename
echo ""

echo "*******"
cmd="hostname --fqdn"
print_cmd_and_exec

echo "*******"
cmd="lsb_release -a"
print_cmd_and_exec

echo "*******"
cmd="uname -a"
print_cmd_and_exec

echo "*******"
cmd="lscpu"
print_cmd_and_exec

echo "*******"
cmd="free -h"
print_cmd_and_exec

echo "*******"
cmd="sudo dmidecode -t system"
print_cmd_and_exec

echo "*******"
cmd="lspci"
print_cmd_and_exec

echo "*******"
cmd="df -h"
print_cmd_and_exec

echo "*******"
cmd="dpkg -l --no-pager"
print_cmd_and_exec

echo "*******"
cmd="pkg-config --modversion libdpdk"
print_cmd_and_exec

echo "*******"
cmd="dpdk-devbind.py --status-dev net"
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

echo "*******"
cmd="ip -6 route show"
print_cmd_and_exec

echo "*******"
for file in `ls /etc/netplan/*.yaml`; do
  cmd="sudo cat $file"
  print_cmd_and_exec
done

echo "*******"
for file in `ls /etc/iptables/*`; do
  cmd="sudo cat $file"
  print_cmd_and_exec
done

cmd="sudo iptables -nL"
print_cmd_and_exec

cmd="sudo ip6tables -nL"
print_cmd_and_exec

echo "*******"
if [ -f /var/run/sdplane.pid ]; then
  cmd="cat /var/run/sdplane.pid"
  print_cmd_and_exec
  pid=`cat /var/run/sdplane.pid`
  cmd="ps -lp $pid"
  print_cmd_and_exec
else
  echo "no sdplane pid file."
fi

echo "*******"
dirs="/lib /usr/sbin /usr/lib /usr/local/lib /usr/local/sbin"
cmd="find $dirs -type f -name \"*sdplane*\" | xargs ls -lh"
print_cmd_and_exec

echo "*******"
execs=`find /usr -type f -name sdplane`
for exec in $execs; do
  #cmd="$exec -v"
  #print_cmd_and_exec
  cmd="ldd $exec"
  print_cmd_and_exec
done

echo "*******"
cmd="systemctl status sdplane"
print_cmd_and_exec

echo "*******"
echo "show tech completed."

# } 2>&1 | tee $dir/$filename
} > $dir/$filename 2>&1

echo "show tech completed."
