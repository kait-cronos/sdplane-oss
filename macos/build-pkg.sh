#!/bin/bash

ver=`bash app/opensh-version.sh`
entitlements=macos/openshCommand.entitlements
dir=Products/usr/local/bin
binuniversal=opensh.universal
binprogram=opensh
pkg=opensh-$ver.pkg

if [ ! -x app/$binprogram ]; then
    echo "cannot find opensh.universal";
    exit -1;
fi

if [ ! -f $entitlements ]; then
    echo "cannot find entitlements file";
    exit -1;
fi

rm -rf $dir

mkdir -p $dir
cp -f app/$binuniversal $dir/$binprogram

echo codesign \
  --force --verify --verbose \
  --sign "Developer ID Application: Yasuhiro Ohara (7FP4CWKTCR)" \
  --options runtime --entitlements $entitlements \
  --timestamp \
  $dir/$binprogram
codesign \
  --force --verify --verbose \
  --sign "Developer ID Application: Yasuhiro Ohara (7FP4CWKTCR)" \
  --options runtime --entitlements $entitlements \
  --timestamp \
  $dir/$binprogram

echo pkgbuild \
  --root "Products" --identifier "net.yasuhironet.opensh" \
  --version "$ver" --install-location "/" \
  --sign "Developer ID Installer: Yasuhiro Ohara (7FP4CWKTCR)" \
  $pkg
pkgbuild \
  --root "Products" --identifier "net.yasuhironet.opensh" \
  --version "$ver" --install-location "/" \
  --sign "Developer ID Installer: Yasuhiro Ohara (7FP4CWKTCR)" \
  $pkg

