## Process this file with automake to produce Makefile.in

SUBDIRS = lib sdplane

ACLOCAL_AMFLAGS = -I m4

LIBTOOL_DEPS = @LIBTOOL_DEPS@
libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status --recheck

AM_CPPFLAGS = -DSYSCONFDIR='$(sysconfdir)'
dist_sysconf_DATA = etc/sdplane.conf.sample etc/60-netplan-sdplane.yaml etc/iptables-rules.v4 etc/iptables-rules.v6 etc/sshd_config etc/modules-load.d/uio_pci_generic.conf etc/modules-load.d/vfio-pci.conf
dist_systemdsystemunit_DATA = etc/sdplane.service
