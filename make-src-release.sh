
version=`git describe`
src_dir=`pwd`
src_dir=${src_dir##*/}
tgt_dir=${src_dir}-${version}-src

echo ${src_dir}

echo cp -R ../${src_dir} ../${tgt_dir}
cp -R ../${src_dir} ../${tgt_dir}
[ -d ./${tgt_dir} ] && rm -rf ./${tgt_dir}
echo mv ../${tgt_dir} ./
mv ../${tgt_dir} ./

echo tar zcf ${tgt_dir}.tar.gz -X upstream-exclude.txt ./${tgt_dir}
tar zcf ${tgt_dir}.tar.gz -X upstream-exclude.txt ./${tgt_dir}

mkdir -p .tmp
[ -d .tmp/${tgt_dir} ] && rm -rf .tmp/${tgt_dir}
echo mv ${tgt_dir} .tmp
mv ${tgt_dir} .tmp

echo mv ${tgt_dir}.tar.gz ../
mv ${tgt_dir}.tar.gz ../

