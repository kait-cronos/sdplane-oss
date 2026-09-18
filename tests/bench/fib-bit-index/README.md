
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

# 結果の解釈

ns - ナノ秒    
insns - instructions(命令数)
cycles - cpuサイクル数
branches - 分岐命令数
br-miss - 分岐命令予測失敗数
IPC - instructions per cycle(1サイクル当たり命令数)
stores - メモリ書き込み命令数
loads - メモリ読み込み命令数
ldhead_cyc - 最も古い未コミットのロード命令が原因でパイプラインのリタイア/進行が阻害されている総サイクル数
stfwd_blk - Store-Forwardingが成立せず、先行ストアのデータ待ちによりブロックされたロード命令の発生回数


まずIPCは素朴に考えれば1サイクルで1命令になるが
近年のCPUはスーパースカラとして複数命令を同時実行しているため
IPCが4などの値になるのはその影響である。
しかし、スーパースカラには限界があり後続の処理が
直前の処理の処理結果のデータに依存しているとCPUは同時実行ができない。

ここにおけるStore Forwardingとはcpuがメモリへ書き込んですぐに
同じメモリからサイズで読みだすときにそのデータを直接送るようにするというものである。
https://readpipe.org/posts/db632a67-296e-4992-9d2e-e3f1eae45d83
```
mov DWORD [esi], edi ; ediからeaxへ直接送る
mov eax, DWORD [esi] 

mov WORD [esi], di   ; サイズが異なるためStore Forwardingが成立しない
mov eax, DWORD [esi] 
```
これによりメモリ読み書きでブロックされずに後続のload命令を実行開始できるようになる。

oldの実装は以下であったが
key_safeへのstoreが終わらない限り後続のkey_safeからloadする処理ができないため
ここは直列化されてしまっていた。
```c
static inline uint16_t 
BIT_INDEX (const uint8_t *key, int s, int n) 
{ 
  uint8_t key_safe[17]; /* sentinel */ 
  memcpy (key_safe, key, 16); // ここでkey -> key_safeのstoreが走る
  key_safe[16] = 0; // そしてサイズの異なるstore

  /* 
  *  extract from two bytes comprising byte to 
  *  which s belongs and adjacent byte 
  */ 
  return (((key_safe[s >> 3] << 8) | (key_safe[(s >> 3) + 1])) >> // key_safeからのload
          (16 - ((s & 0x7) + n))) & ((1 << n) - 1); 
} 
```

そのためkey_safeは`fib_router_lookup`等で一回だけ作成し探索中は`memcpy`をしないようにしたところ
性能が改善した。

当初は要因が`memcpy`数だと思っていたが対照実験として以下のような実装を用意して
ストア処理の回数は同一で読みだすバッファだけ違うというものを用意してみた。

```c
static struct fib_node *
farload_fib_route_lookup (struct fib_tree *t, const uint8_t *key)
{
  struct fib_node *n = t->root;
  struct fib_node *cand = NULL;
  int cur_plen = 0;
  struct farload_buf scratch, readbuf;

  new_bit_index_key_init (readbuf.b, key);
  while (n)
    {
      if (n->leaf)
        cand = n;
      new_bit_index_key_init (scratch.b, key);
      __asm__ volatile ("" : "+m" (scratch));
      n = n->child[new_BIT_INDEX (readbuf.b, cur_plen, K)];
      cur_plen += K;
    }
  return cand;
}
```

その結果store命令数はほぼ同じにもかかわらず性能は`new`近くに改善したためstore命令数自体は律速要因ではなく
書き込み直後に同じメモリから読みだすことが律速原因だったと考えられる。

```
# old
362c:	movdqu (%rsi),%xmm0            # const uint8_t* keyの値をレジスタに読み込み (loop外)
3644:	movb   $0x0,0x10(%rsp)         # key_safe[16] = 0; 
364b:	movaps %xmm0,(%rsp)            # memcpy (key_safe, key, 16); # loop内でstore
3666:	movzbl (%rsp,%rax,1),%eax      # (((key_safe[s >> 3] << 8) | (key_safe[(s >> 3) + 1]))) のアクセス相当
366a:	movzbl (%rsp,%rcx,1),%ecx      # 

# new
3db6:	movdqu (%rsi),%xmm0            # const uint8_t* keyの値をレジスタに読み込み
3dcd:	mov    %dx,0x10(%rsp)          # memset (key_safe + KEY_SIZE (128), 0, BIT_INDEX_KEY_SIZE - KEY_SIZE (128));
3dd6:	movaps %xmm0,(%rsp)            # memcpy (key_safe, key, KEY_SIZE (128)); // どちらもloop前に完了
3e15:	movzbl 0x1(%rsp,%rcx,1),%eax   # (((key_safe[s >> 3] << 8) | (key_safe[(s >> 3) + 1]))) のアクセス相当
3e1a:	movzbl (%rsp,%rcx,1),%ecx      # loop内では読み込みのみ

# farload
3564:	movdqu (%rsi),%xmm0            # const uint8_t* keyの値をレジスタに読み込み
357f:	movaps %xmm0,0x20(%rsp)        # ループ外書き込み
359d:	movaps %xmm0,(%rsp)            # ループ内書き込み (読み込まれない)
35b8:	movzbl 0x20(%rsp,%rcx,1),%eax  # ループ外書き込みしたのから読み取る
35bd:	movzbl 0x21(%rsp,%rcx,1),%ecx  # 

# nocopy
36c8:	movzbl (%r8,%r10,1),%eax       # %r8 == keyのアドレス
36cd:	movzbl 0x1(%r8,%r10,1),%r10d   # 
3716:	movzbl 0xf(%r8),%eax           

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
  5. perfコマンドがあればperfを使って追加の指標を出す
+ cpufreq-pin.sh - ベンチマーク時にcpu周波数を固定する用スクリプト(要sudo)
  + 通常のcpuだとサーマルスロットリングしうるため周波数を固定してnsも厳密に測る用

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
