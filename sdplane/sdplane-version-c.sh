#!/bin/bash

sdplane_version=`top_srcdir=$top_srcdir bash $top_srcdir/sdplane/sdplane-version.sh`

cat << EOHD
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /*HAVE_CONFIG_H*/
const char sdplane_version[] = "$sdplane_version";
EOHD

