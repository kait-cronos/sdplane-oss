
ver=$1
if [ -z "$ver" ]; then
    echo "specify version."
    exit -1;
fi

# vi debian/changelog
tar zcvf ../sdplane_${ver}.orig.tar.gz .
debuild

