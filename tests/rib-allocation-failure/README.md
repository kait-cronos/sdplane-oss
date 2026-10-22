# RIBの割当失敗試験

RIB生成処理に割当失敗を注入し、次の項目を確認する。

- `rib`、`rib_info`、先頭と最後のFIBツリーの割当失敗を扱えること。
- 初回の更新に失敗した場合は、公開用RIBのポインターがNULLのままであること。
- 更新に失敗した場合は、公開済みRIBのポインターと`rib_info`の内容を維持すること。
- 失敗後の更新で割当に成功すれば、新しいRIBを公開できること。

ASanで不正なメモリアクセスとメモリリークを検査し、失敗までに確保したメモリの解放も確認する。
内部メッセージの処理関数を直接呼び出すため、NIC、hugepages、root権限は不要である。

## 実行方法

sdplaneのビルド環境を用意し、リポジトリのルートディレクトリでビルドする。

```sh
sh autogen.sh
./configure --disable-pktgen
make
```

続いて、ASan/UBSanを有効にしてテストを実行する。

```sh
make -C tests/rib-allocation-failure clean
make -C tests/rib-allocation-failure check
```

RIBや内部メッセージの割当が型の要求するアラインメントを満たさない構成では、UBSanのアラインメント検査を無効にして実行する。

```sh
make -C tests/rib-allocation-failure clean
make -C tests/rib-allocation-failure check EXTRA_CFLAGS=-fno-sanitize=alignment
```

コンパイルオプションや検証対象のソースツリーを変更する場合は、実行前に`clean`でテストのビルド成果物を削除する。
ソースツリーを変更した場合は、sdplaneもビルドし直す。
`ROOT`には、configureとビルドを済ませた別のソースツリーを指定できる。
省略すると、このテストディレクトリから見たリポジトリのルートを使う。

## 試験の仕組み

GNUリンカーの`--wrap`で`malloc()`、`aligned_alloc()`、`fib_new()`の呼び出しを差し替え、指定した割当を一度だけ失敗させる。
RIBの確保に`malloc()`と`aligned_alloc()`のどちらを使う構成にも対応する。
失敗の注入が実行されたことを確かめた後、公開済みRIBの状態を検査する。

Makefileでは、RIBマネージャー、内部メッセージ、FIB、FDB、MDBのソースをASan/UBSan付きで再コンパイルし、残りのオブジェクトはsdplaneの通常ビルドからリンクする。
メモリリーク検査を有効にし、sanitizerがエラーを検出すると試験を停止する。
