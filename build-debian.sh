
packagename=sdplane

origname=`git describe --abbrev=0 | sed -e 's/-[0-9-]*//'`
origdebversion=`echo $origname | sed -e 's/^v//'`

cat << EOHD
origname: $origname
origdebversion: $origdebversion
EOHD

# vi debian/changelog
tar zcvf ../${packagename}_${origdebversion}.orig.tar.gz -X upstream-exclude.txt .

debuild

