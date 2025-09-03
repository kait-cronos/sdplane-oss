
BEGIN {
  ifname="port-dpdk1"
  print "#!/bin/sh";
}

{
  printf "ip neigh add 169.254.%d.%d dev %s lladdr 00:0d:b9:57:%02x:%02x\n",
         int($1/256), $1%256, ifname, int($1/256), $1%256;
}

