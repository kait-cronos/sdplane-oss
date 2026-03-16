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
