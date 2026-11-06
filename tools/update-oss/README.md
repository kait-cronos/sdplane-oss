# sdplane-oss統合リポジトリ構築スクリプト
sdplane-devをベースにlibsdplane-dev/lib/を履歴付きで統合し、GitHubのkait-cronos/sdplane-ossを上書きするための作業ディレクトリを構築

usage:
```
./tools/update-oss/update-oss.sh <workdir>
```

prerequisites:
- git-filter-repo (apt install git-filter-repo)

## 手動ステップ (スクリプト完了後):
- ビルド動作確認
- sdplane-oss への確認用 push
- sdplane-oss main への force push

## update-oss2.sh (既存の sdplane-oss に差分をマージする)

usage:
```
SDPLANE_OSS_REPO=<oss repo> ./tools/update-oss/update-oss2.sh <workdir> [step1|step2|step3|step4]
```

- step1: sdplane-dev, sdplane-oss を clone し、日付ブランチを作成
- step2: libsdplane-dev の lib/ をマージ
- step3: sdplane-dev をマージ
- step4: ビルドファイルの確認、パッケージングファイルの適用、内部ファイルの削除

merge で conflict が出たら、解消して commit し、次の step を実行する。

## oss-build/

sdplane-oss 用のファイル。sdplane-oss では lib/ (libsdplane) を同梱する。

- ビルドファイル (configure.ac, Makefile.am, lib/Makefile.am, sdplane/Makefile.am):
  sdplane-dev の変更 (新しいソースなど) をマージで取り込む必要があるため、
  上書きしない。step4 で OSS 用の設定が残っているか確認し、oss-build/ との差分を表示する。
  リリース後は oss-build/ を sdplane-oss の内容に合わせて更新しておく。
- README.md とパッケージングファイル (build-debian.sh, tools/deb/build-in-container.sh,
  debian/control, debian/rules): step4 で oss-build/ の内容に置き換える。
  sdplane-oss は libsdplane を同梱した sdplane パッケージ 1 つだけをビルドする。
  OSS 用のファイルを増やすときは、oss-build/ に置いて update-oss2.sh の
  OSS_PACKAGING_FILES に追加する。
