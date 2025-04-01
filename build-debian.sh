
# 0.3.36: annotated tag: (virtually) upstream version.
# 0.3.36-1: lightweight tag: used for debian packaging.
# git describe (without --tags) should calculate the version
# from the latest annotated tag, not from the lightweight tag.
# calculate version without --tags for git-describe,
# and prepare the lightweight tag fot the name (e.g., 0.3.36-2).
# then we can generate changelog entry for 0.3.36-1..0.3.36-2.

packagename=sdplane

function logentry() {
    local previous=$1
    local version=$2
    local debversion=$3
    echo "$packagename ($debversion) UNRELEASED; urgency=low"
    echo
    echo "  * release. update changelog."
    git --no-pager log --format="  * %s" $previous${previous:+..}$version
    echo
    git --no-pager log --format=" -- %an <%ae>  %aD" -n 1 $version
    echo
}

# dirty=`git describe --dirty | grep dirty`
# if [ ! -z "$dirty" ]; then
#     echo -n "please commit first: "
#     git describe --dirty=-modified.
#     exit -1;
# fi

version=`git describe | sed -e 's/-g.*//'`
ncommit=`echo $version | sed -e 's/^.*-//'`
newcommit=$ncommit
if [[ $newcommit =~ ^[0-9]+$ ]]; then
  ((newcommit++))
  echo version: $version
  echo ncommit: $ncommit
  echo $ncommit + 1 = $newcommit
else
  echo $newcommit is not a commit distance. major changed?
fi
newversion=`echo $version | sed -e "s/-$ncommit/-$newcommit/"`
previous=`git describe --abbrev=0 --tags`
origname=`git describe --abbrev=0 | sed -e 's/-[0-9-]*//'`

debversion=`echo $newversion | sed -e 's/^v//'`
origdebversion=`echo $origname | sed -e 's/^v//'`

cat << EOHD
version: $version
newversion: $newversion
debversion: $debversion
previous: $previous
origname: $origname
EOHD

if [ ${previous} = ${version} && $1 != "-f" ]; then
    echo "nothing to release (previous = version: ${version})."
    exit
fi

if [ ! -f debian/changelog.${previous} ]; then
    echo create missing debian/changelog.${previous}
    cp debian/changelog debian/changelog.${previous}
    # git add debian/changelog.${previous}
fi

echo "create debian/changelog.${previous}-${newversion}."
logentry $previous HEAD $debversion > debian/changelog.${previous}-${newversion}

if [ ${previous} != ${newversion} ]; then
  echo "create debian/changelog.${newversion}."
  cat debian/changelog.${previous}-${newversion} debian/changelog.${previous} > debian/changelog.${newversion}
fi
rm debian/changelog.${previous}-${newversion}

echo "create debian/changelog."
cp debian/changelog.${newversion} debian/changelog

# remove intermittent files.
echo removing:
ls -l debian/changelog.v*
rm -f debian/changelog.v*

git commit -a -m "update changelog for $newversion."

newversion=`git describe | sed -e 's/-g.*//'`

# prepare the lightweight tag.
echo "tagging ${newversion}."
git tag ${newversion}

# vi debian/changelog
tar zcvf ../${packagename}_${origdebversion}.orig.tar.gz -X upstream-exclude.txt .

debuild

