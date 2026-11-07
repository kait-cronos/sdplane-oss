# 経路種別の保持・表示の試験

IPv4/IPv6の経路を登録・置換・削除し、内部メッセージ、RIB、FIBで保持する経路種別と`show fib`の表示を確認する。

- Netlinkの`rtm_type`が内部メッセージ、RIB、FIBへ引き継がれること。
- 同じプレフィックスの置換で種別が更新され、異なる長さのプレフィックスが重なる場合も、それぞれの種別が保持されること。
- `show fib`が既知の12種類と未知の値を表示し、各経路の種別がType列に揃うこと。
- SRv6 headendのCLIコマンドとlocal SID登録が作成する経路に`RTN_UNICAST`が設定されること。
- DPDKのmbufとringを使い、mainの転送とlocalのpuntで送出先とTTL・Hop Limitの値が期待どおりで、IPv4ヘッダーチェックサムが正しいこと。

NIC、hugepages、root権限は不要である。
パケット試験はDPDK EALを起動し、プロセスに割り当てられたCPUの1つと64 MiBのメモリを指定する。

## 実行方法

sdplaneのビルド環境を用意し、リポジトリのルートディレクトリでビルドする。

```sh
sh autogen.sh
./configure --disable-pktgen
make
```

続いて、ASan/UBSanを有効にして試験を実行する。

```sh
make -C tests/route-type-observability clean
make -C tests/route-type-observability check
```

RIBや内部メッセージの割当が型の要求するアラインメントを満たさない構成では、UBSanのアラインメント検査を無効にして実行する。

```sh
make -C tests/route-type-observability clean
make -C tests/route-type-observability check EXTRA_CFLAGS=-fno-sanitize=alignment
```

コンパイルオプションや検証対象のソースツリーを変更する場合は、実行前に`clean`で試験のビルド成果物を削除する。
ソースツリーを変更した場合は、sdplaneもビルドし直す。
`ROOT`には、configureとビルドを済ませた別のソースツリーを指定できる。
省略すると、この試験ディレクトリから見たリポジトリのルートを使う。

## 試験の仕組み

試験内でNetlinkメッセージを構築し、実装の受信処理とRIB更新処理に渡す。
表示試験の各経路には出力インターフェースを設定し、経路種別の名前と表示位置を確認する。
Linuxが生成する通知の網羅や、blackholeなどの種別に応じた転送動作は、この試験の対象に含めていない。

Makefileでは、RIBマネージャー、内部メッセージ、経路ツリー、FIB、RIB、ネクストホップ、FDB、MDB、Netlink、SRv6コマンドのソースをASan/UBSan付きで再コンパイルする。
パケット試験に取り込む転送処理もsanitizerの対象とし、残りのオブジェクトはsdplaneの通常ビルドからリンクする。
メモリリーク検査を有効にし、sanitizerがエラーを検出すると試験を停止する。
