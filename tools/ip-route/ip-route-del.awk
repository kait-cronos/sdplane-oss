
BEGIN {
  ifname="port-dpdk1"
  via="172.16.0.1"
  print "#!/bin/sh";
  printf "ip addr del 172.16.0.2/16 dev %s\n", ifname;
}

{
  printf "ip route del 169.254.%d.%d/32 via %s dev %s\n",
         int($1/256), $1%256, via, ifname;
}

