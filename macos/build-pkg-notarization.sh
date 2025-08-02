#!/bin/bash

ver=`bash app/opensh-version.sh`
entitlements=openshCommand.entitlements
dir=Products/usr/local/bin
binuniversal=opensh.universal
binprogram=opensh
pkg=opensh-$ver.pkg

if [ ! -f $pkg ]; then
    echo "cannot find pkg file";
    exit -1;
fi

echo xcrun notarytool \
  submit $pkg \
  --keychain-profile "opensh Application" \
  --wait
xcrun notarytool \
  submit $pkg \
  --keychain-profile "opensh Application" \
  --wait

echo xcrun stapler staple $pkg
xcrun stapler staple $pkg


