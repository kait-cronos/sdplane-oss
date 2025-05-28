
    <a href="https://gist.github.com/cheerfulstoic/d107229326a01ff0f333a1d3476e068d"><img src="https://img.shields.io/badge/Maintenance%20Level-Actively%20Developed-brightgreen.svg" /></a>

# What is sdplane-dev (soft-dplane)
DPDKを使った高速のOSSソフトウェアルータです。

# 必要なもの 　　
    + yasuhironet/lthread (from DPDK)
    + kait-cronos/libsdplane-dev （自前で、make insallしたほうがいい)
    + Ubuntu(現時点では）
　　+ etckeeper, tig, build-essential, bridge-utils, cmake, iptables-persistent, fail2ban, dmidecode, screen, ripgrep
    + DPDK
    + hugepage
    + 4 NIC (virtio-netでもできる)
    + netplan
    + iptables


toptonは、10G NICのmini-pcです。
wiretapは、1G NICのmini-pcです。

基本的には、このドキュメントに従う。
https://github.com/kait-cronos/sdplane-dev/blob/main/doc/install-memo-topton.txt
: topton用install document

https://github.com/kait-cronos/sdplane-dev/blob/main/doc/install-memo-v15.txt: wiretap用install document

参考:https://enog.jp/wordpress/wp-content/uploads/2024/11/2024-11-22-sdn-onsen-yasu.pdf
※ zcmdshは、libsdplane-devのことです。名前が変わりました。

## how to make sdplane-dev
===
% aclocal
% autoheader
% automake -a -c
% autoconf
% mkdir build
% cd build
% ../configure
% make

/* run in foreground */
# ./sdplane/sdplane
===

# how to use
$ sdplane local 9882

