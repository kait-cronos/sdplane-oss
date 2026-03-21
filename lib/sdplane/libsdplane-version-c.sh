#!/bin/bash

libsdplane_version=`top_srcdir=$top_srcdir bash $top_srcdir/lib/sdplane/libsdplane-version.sh`

cat << EOHD
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /*HAVE_CONFIG_H*/
const char libsdplane_version[] = "$libsdplane_version";
EOHD

