#!/bin/bash

opensh_version=`top_srcdir=$top_srcdir bash $top_srcdir/app/opensh-version.sh`
#echo $opensh_version

cat << EOHD
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /*HAVE_CONFIG_H*/
const char opensh_version[] = "$opensh_version";
EOHD

