
SUBDIRS = sdplane

AM_CPPFLAGS = -DSYSCONFDIR='$(sysconfdir)'
dist_sysconf_DATA = etc/sdplane.conf.sample etc/60-netplan-sdplane.yaml etc/iptables-rules.v4 etc/iptables-rules.v6 etc/sshd_config etc/modules-load.d/uio_pci_generic.conf etc/modules-load.d/vfio-pci.conf
dist_systemdsystemunit_DATA = etc/sdplane.service
