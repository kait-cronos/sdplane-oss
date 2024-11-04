
SUBDIRS = l2fwd l3fwd sdplane

AM_CPPFLAGS = -DSYSCONFDIR='$(sysconfdir)'
dist_sysconf_DATA = etc/sdplane.conf.sample etc/60-netplan-sdplane.yaml etc/iptables-rules.v4 etc/iptables-rules.v6 etc/sshd_config
dist_systemdsystemunit_DATA = etc/sdplane.service

