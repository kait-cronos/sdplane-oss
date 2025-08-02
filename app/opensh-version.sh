#!/bin/bash

git describe --dirty=-modified

exit

VERFILE=$(find $top_srcdir -name opensh-version.txt)
VERSION=$(cat $VERFILE)
COMMIT_ID=$(git rev-parse HEAD | cut -c1-7 | tr -d "\n")

# When we call this script from inside the configure.ac,
# the confdefs.h is always shown in the git status -z.
# below should prevent it.
GITSTATUS=$(git status -s | grep -v confdefs.h)

if [ ! -z "$GITSTATUS" ]; then
  MODIFIED="-modified"
fi

echo ${VERSION}-${COMMIT_ID}${MODIFIED}
# belwo can be used for debug.
# echo ${VERSION}-${COMMIT_ID}-${GITSTATUS}

