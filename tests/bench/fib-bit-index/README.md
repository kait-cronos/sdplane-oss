
# fib.c BIT_INDEX関数ベンチマーク

[PR 400　perf: fib.cのBIT_INDEX関数の最適化](https://github.com/kait-cronos/sdplane-dev/pull/400)用に作成したマイクロベンチマーク用スクリプトである。ディレクトリ単体で完結しておりsdplane自体のビルドは必要ない。

### 実行方法
```
cd tests/bench/fib-bit-index
./run.sh
```

### 測定対象
```
old_ - PR400時点main実装。 (f12ce59d7f2e64cafce6a1a52f14a48fff8b5ed5)
new_ - PR400の実装(memcpy回数削減のみ)(348d5f6b071cba5bc3c7b813d8b79019ad418036)
farload_ - memcpyの頻度はold_と変わらずだがループごと書き出すバッファとkeyを読み込むバッファを別にする (ベンチマーク内)
umeda - 梅田さんのパッチの実装(loop化及びmemcpy回数削減) (パッチファイル参照)
nocopy - PR400の最初のコミットのバージョン(memcpyをなくして場合分けで対処しようとした場合) (348d5f6b071cba5bc3c7b813d8b79019ad418036)
nocopyloop - umedaさんのloop化をnocopyに適用した版 (ベンチマーク内)
```

以下の関数/マクロ定義を各実装を含むパッチ/コミットから抽出して同一条件でベンチマークするもの。
```
BIT_INDEX_KEY_SIZE
bit_index_key_init
BIT_INDEX
_lookup 
fib_route_lookup
```

また以下の関数については各実装間で変化がないためoldから抽出している
```
_count_nonzero
_create_fib_node
_add
fib_route_add
_free_fib_node
fib_free
fib_new
```

# 出力

.gitignoreされている

bitindex-bench.c - ベンチマーク用.cファイル
bitindex-bench - コンパイル後ベンチマーク実行ファイル
bitindex-bench.txt - ベンチマーク出力(bitindex-benchを出力してあとは)
disasm.txt - ディスアセンブリ
perf-events.txt - perfコマンドを使って個別のlookupごとにプロセスを別にして計測したもの

# 測定結果例

bitindex-bench.txt

```
# bitindex-bench
# date      : 2026-08-28T16:08:45+09:00
# cpu       : Intel(R) N100
# pinned to : cpu 2
# governor  : powersave
# freq now  : 1306212 kHz of max 3400000
# pkg temp  : 79000
# load      : 1.11 0.87 0.62
# gcc       : gcc (GCC) 16.1.1 20260625
# revisions measured:
  f12ce59 (f12ce59d7f) -> old_
  16a898b (16a898b853) -> nocopy_
  348d5f6 (348d5f6) -> new_
  134d2fb-t-umeda-inline-fib_route_lookup.patch applied to f12ce59 -> umeda_

BIT_INDEX equivalence: 26033280 comparisons, 0 mismatches -> IDENTICAL

1 routes + one /128
                                                   ns     insns    cycles  branches   br-miss
  /128  copy per level        (old_)            475.8    1502.0    1517.8      67.0      0.00
  /128  copy per lookup       (new_)             96.4    1328.0     311.6      70.0      0.00
  /128  copy per lookup, loop (umeda_)           94.7    1328.0     315.0      70.0      0.00
  /128  no copy, branch/level (nocopy_)         106.1    1607.0     355.1     196.0      0.00
  /128  no copy, loop         (nocopyloop_)     107.9    1619.0     356.7     199.0      0.00
  /128  store/level, far loads (farload_)       104.2    1520.0     339.1      71.0      0.00
  /64   copy per level        (old_)            135.5     445.0     445.2      21.0      0.00
  /64   copy per lookup       (new_)             30.1     408.0     100.9      24.0      0.00
  /64   copy per lookup, loop (umeda_)           30.7     408.0     101.7      24.0      0.00
  /64   no copy, branch/level (nocopy_)          31.7     482.0     106.2      59.0      0.00
  /64   no copy, loop         (nocopyloop_)      32.7     490.0     109.1      62.0      0.00

16 routes + one /128
                                                   ns     insns    cycles  branches   br-miss
  /128  copy per level        (old_)            469.2    1502.0    1517.9      67.0      0.00
  /128  copy per lookup       (new_)             95.4    1328.0     311.6      70.0      0.00
  /128  copy per lookup, loop (umeda_)          100.2    1328.0     315.0      70.0      0.00
  /128  no copy, branch/level (nocopy_)         108.4    1607.0     355.2     196.0      0.00
  /128  no copy, loop         (nocopyloop_)     107.8    1619.0     356.7     199.0      0.00
  /128  store/level, far loads (farload_)       105.6    1520.0     338.5      71.0      0.00
  /64   copy per level        (old_)            138.2     445.0     445.2      21.0      0.00
  /64   copy per lookup       (new_)             30.3     408.0     100.9      24.0      0.00
  /64   copy per lookup, loop (umeda_)           30.9     408.0     101.6      24.0      0.00
  /64   no copy, branch/level (nocopy_)          32.2     482.0     106.1      59.0      0.00
  /64   no copy, loop         (nocopyloop_)      33.6     490.0     109.1      62.0      0.00

256 routes + one /128
                                                   ns     insns    cycles  branches   br-miss
  /128  copy per level        (old_)            472.4    1502.0    1518.0      67.0      0.00
  /128  copy per lookup       (new_)             93.4    1328.0     311.7      70.0      0.00
  /128  copy per lookup, loop (umeda_)           94.5    1328.0     315.0      70.0      0.00
  /128  no copy, branch/level (nocopy_)         109.7    1607.0     355.1     196.0      0.00
  /128  no copy, loop         (nocopyloop_)     122.6    1619.0     357.5     199.0      0.00
  /128  store/level, far loads (farload_)       103.7    1520.0     338.5      71.0      0.00
  /64   copy per level        (old_)            135.6     445.0     445.2      21.0      0.00
  /64   copy per lookup       (new_)             30.6     408.0     100.9      24.0      0.00
  /64   copy per lookup, loop (umeda_)           30.6     408.0     101.7      24.0      0.00
  /64   no copy, branch/level (nocopy_)          31.6     482.0     106.1      59.0      0.00
  /64   no copy, loop         (nocopyloop_)      32.5     490.0     109.1      62.0      0.00

4096 routes + one /128
                                                   ns     insns    cycles  branches   br-miss
  /128  copy per level        (old_)            474.2    1502.0    1518.0      67.0      0.00
  /128  copy per lookup       (new_)             95.4    1328.0     311.6      70.0      0.00
  /128  copy per lookup, loop (umeda_)           95.3    1328.0     315.0      70.0      0.00
  /128  no copy, branch/level (nocopy_)         108.1    1607.0     355.1     196.0      0.00
  /128  no copy, loop         (nocopyloop_)     108.4    1619.0     356.7     199.0      0.00
  /128  store/level, far loads (farload_)       106.8    1520.0     338.5      71.0      0.00
  /64   copy per level        (old_)            143.3     445.0     445.2      21.0      0.00
  /64   copy per lookup       (new_)             30.7     408.0     100.9      24.0      0.00
  /64   copy per lookup, loop (umeda_)           30.6     408.0     101.6      24.0      0.00
  /64   no copy, branch/level (nocopy_)          31.8     482.0     106.2      59.0      0.00
  /64   no copy, loop         (nocopyloop_)      32.9     490.0     109.1      62.0      0.00


# load after: 1.10 0.87 0.62   pkg temp: 80000
```

perf-events.txt
```
# where the cycles go, one variant per process (perf stat, user only)
# reps=2000000 lookups of the /128 host route, 4096-route trie
#
# stores, loads   retired memory uops per lookup.  old_ and farload_ must
#                 match here: that is what rules out "farload_ is only
#                 faster because the compiler dropped the store"
# ldhead_cyc      CYCLES (not events) the oldest load was stalled, any reason
# stfwd_blk       loads blocked because an older store to the same address
#                 had no data ready yet -- the store-forward FAILURE case

variant          insns    cycles    IPC   stores    loads ldhead_cyc stfwd_blk
old             1597.4    1512.6   1.06    133.4    268.2     129.1     1.33
new             1341.4     316.3   4.24      5.4    268.2       3.8     0.26
umeda           1341.4     319.3   4.20      5.4    268.2       3.3     0.28
nocopy          1631.4     361.6   4.51      2.4    258.2       0.5     0.01
nocopyloop      1632.4     361.6   4.51      2.4    258.2       0.5     0.01
farload         1533.4     343.2   4.47    135.4    268.2       3.4     0.08
```

# スクリプトの内容

+ make-bench.sh - ベンチマーク用.cファイルを作成するスクリプト
  + run.shから呼び出される
  1. 各パッチのfib.cを取り出す
  2. awkで指定関数区間を抽出
  3. 各関数名をsedでprefix付きに置換
  4. ベンチマーク用ハーネスと結合してbitindex-bench.cを出力する。
+ run.sh - ベンチマーク用.cファイル作成をしコンパイルしてベンチマークを走らせる
  1. make-bench.shを呼び出してベンチマーク用bitindex-bench.cファイルを生成する
  2. gccでbitindex-bench.cをコンパイルする
  3. bitindex-benchを実行してbitindex-bench.txtに保存する
  4. disasm.txtにnmとobjdumpを使用して各実装に対応するアセンブリ部分を抽出する(best effort)
  5. perfコマンドがあればperfを使って
+ cpufreq-pin.sh - ベンチマーク時にcpu周波数を固定する用スクリプト(要sudo)
  + 通常のcpuだとサーマルスロットリングしうるため

# 必要依存関係

## 必須

+ bash
+ gcc
+ date
+ grep
+ cat
+ git
+ mktmp
+ rm
+ sed
+ shift
+ true
+ patch
+ awk
+ nm
+ objdump
+ tee
+ dirname

## オプション
+ perf    - プロセス内交絡を減らすため
+ taskset - ベンチマークを走らせるCPUを固定するために使う
