# netlink 経路ロス計測

カーネルに経路を一気に投入したとき、sdplane の netlink 処理がどれだけ経路を取りこぼすかを計測するベンチマークです。経路数ごとのロス率を CSV とグラフに出力します。

rib-manager と netlink-thread の配置（lthread か lcore か）や、バッファサイズなどの変更で結果がどう変わるかを比較するために使います。

## 計測の流れ

```
netns a
┌──────────────────────────────────────────────────┐
│  install_routes.py                               │
│    │ 1. RTM_NEWROUTE で N 経路を全速で追加        │
│    ▼                                             │
│  Linux カーネル                                   │
│    │ 2. netlink 通知（ここであふれると socket_drops）│
│    ▼                                             │
│  sdplane: netlink-thread                         │
│    │ 3. msg_queue_rib（ここであふれると ring_lost） │
│    ▼                                             │
│  sdplane: rib-manager → RIB                      │
│    ▲                                             │
│    │ 4. telnet 9882 で show rib ipv4 route を比較 │
│  measure_route_loss.py                           │
└──────────────────────────────────────────────────┘
```

1回の試行は次の手順です。経路は実インターネットの BGP テーブル（MRT 形式）から取ります。

1. sdplane を起動し、`router` タップにアドレスを付ける
2. 接続経路が RIB に届くまで待つ
3. N 経路をカーネルへ投入する
4. `show` の結果が変わらなくなるまで待つ（2秒おきに読み、3回連続で同じなら完了）
5. 投入した経路と RIB を比較し、足りない分をロスとして記録する
6. sdplane を止めて、経路を削除する

## 必要なもの

- ビルド済みの sdplane（`sdplane/sdplane`。`build/` でビルドしたときなど、別の場所なら `run_sweep.sh` は `--sdplane`、`run_sdplane.sh` は環境変数 `SDPLANE` で指定）
- `sudo` の権限（netns の作成と経路の投入に必要です）
- [uv](https://docs.astral.sh/uv/)（Python の依存関係は実行時に自動で入ります）
- `iproute2`、`curl`、`telnet`、DPDK の `dpdk-hugepages.py`

## 使い方

コマンドはすべて `tests/route-benchmark` で実行します。

### 1. hugepages を確保する

OS の起動後に1回だけ必要です。sdplane の実行中には実行しないでください。

```sh
./setup_hugepages.sh
```

### 2. MRT データを用意する

RIPE RIS の最新のダンプを `data/bview.gz` にダウンロードします。約400MBあります。`data/` は Git の管理外なので、データは commit されません。

```sh
./download_mrt.sh
```

別のダンプを使うときは URL を引数に渡すか、`data/bview.gz` に直接置きます。

下の参考結果では `bview.20261001.0000.gz` を使いました。比較するときは同じファイルを使ってください。ファイルが違うと、投入する経路も変わります。

```sh
./download_mrt.sh https://data.ris.ripe.net/rrc00/2026.10/bview.20261001.0000.gz
```

### 3. 計測する

まず短い計測で、動作を確認します。

```sh
./run_sweep.sh --counts 100 1000 --trials 1
```

本番の計測は、100〜100万経路を各3回実行します。1構成あたり約50分かかります。

```sh
./run_sweep.sh --conf configs/rib-lcore-netlink-lcore.conf
```

`--conf` を省略すると `configs/rib-lcore-netlink-lcore.conf` を使います。後に書いた引数が優先されるので、`--counts` などは上書きできます。全オプションは `uv run measure_route_loss.py --help` で確認できます。

netns `a` と `b` は自動で作られます。

### 4. 結果を比較する

```sh
uv run compare_results.py --out results/compare \
    lcore=results/rib-lcore-netlink-lcore-XXXX \
    lthread=results/rib-lthread-netlink-lthread-YYYY
```

`LABEL=` は省略でき、省略するとディレクトリ名が凡例になります。CSV からグラフだけを描き直すこともできます。

```sh
uv run measure_route_loss.py --plot results/XXXX/results.csv
```

## 出力

`results/<設定名>-<日時>/` に出力します。`results/` は Git の管理外です。

| ファイル | 内容 |
|---|---|
| `results.csv` | 試行ごとの結果 |
| `loss.png` | 経路数ごとのロス率 |
| `absorbed.png` | 投入時間と RIB に届いた経路数（ロスが20%以上の試行のみ） |
| `sdplane.conf` | 計測に使った設定 |
| `logs/` | 試行ごとの sdplane のログ |

`results.csv` の列は次のとおりです。

| 列 | 意味 |
|---|---|
| `routes`, `trial` | 投入した経路数、試行番号 |
| `installed` | カーネルへの投入に成功した経路数 |
| `seen` | RIB で確認できた経路数 |
| `lost`, `loss_pct` | 失われた経路数とその割合 |
| `socket_drops` | netlink ソケットのドロップ数（`/proc/net/netlink`） |
| `ring_lost` | `msg_queue_rib` があふれた回数（ログの `is lost.` の数） |
| `install_sec` | 投入にかかった時間 |
| `settle_sec` | 投入後、結果が落ち着くまでの時間 |
| `status` | `ok`、`not-settled`、`error: ...` のいずれか |

`ok` 以外の試行は、平均とグラフから除外されます。

`lost` が `socket_drops` と `ring_lost` のどちらと一致するかを見ると、落ちた場所が分かります。

## 設定ファイル

ファイル名は rib-manager と netlink-thread の配置を表します。

| ファイル | rib-manager | netlink-thread |
|---|---|---|
| `rib-lthread-netlink-lthread.conf` | lthread | lthread |
| `rib-lthread-netlink-lcore.conf` | lthread | lcore 4 |
| `rib-lcore-netlink-lthread.conf` | lcore 3 | lthread |
| `rib-lcore-netlink-lcore.conf` | lcore 3 | lcore 4 |

どの設定にも `set worker lthread rcu-reporter` を入れています。rib-manager の RCU の同期待ちが、lthread 側の報告を待って長引かないようにするためです。

## 参考結果

ロス率（%）です。commit `1e64c30`、各3回の平均、投入速度は約3,500〜4,000経路/秒です。`rcu-reporter` と `set rib msg-queue size`（既定値 1M 件）が入る前に測ったもので、今の設定と main では結果が変わります。

| 投入経路数 | rib lthread<br>netlink lthread | rib lthread<br>netlink lcore | rib lcore<br>netlink lthread | rib lcore<br>netlink lcore |
|---:|---:|---:|---:|---:|
| 1,000 | 0.00 | 0.00 | 0.00 | 0.00 |
| 2,000 | 3.43 | 0.00 | 0.00 | 38.12 |
| 3,000 | 24.81 | 0.00 | 0.00 | 59.13 |
| 5,000 | 36.21 | 12.73 | 10.18 | 75.04 |
| 10,000 | 52.18 | 40.44 | 40.72 | 87.77 |
| 100,000 | 84.33 | 82.94 | 83.07 | 98.28 |
| 1,000,000 | 95.75 | 95.54 | 95.58 | 99.58 |

ロスの場所は、rib lthread / netlink lthread だけソケットで、ほかの3構成は `msg_queue_rib` でした。

## ファイル構成

| ファイル | 役割 |
|---|---|
| `run_sweep.sh` | 本番の計測を実行する入口 |
| `measure_route_loss.py` | 計測本体。sdplane の起動から集計、グラフ作成まで |
| `install_routes.py` | MRT を読み、rtnetlink で経路を追加、削除する |
| `compare_results.py` | 複数の結果を1つのグラフにまとめる |
| `setup_netns.py` | netns `a`, `b` と2組の veth を作成、削除する |
| `setup_hugepages.sh` | hugepages を確保する |
| `download_mrt.sh` | MRT データをダウンロードする |
| `run_sdplane.sh` | 設定の確認用に sdplane を手で起動する |
| `configs/` | 計測用の sdplane 設定 |
| `test_install_routes.py` | `install_routes.py` の単体テスト（`python3 -m unittest test_install_routes`） |

## 手で動かす

計測を通さずに動きを見たいときの手順です。

```sh
# 端末1: sdplane を起動する
./run_sdplane.sh configs/rib-lcore-netlink-lcore.conf

# 端末2: タップにアドレスを付けて、経路を入れる（Enter で削除）
sudo ip netns exec a ip addr replace 192.168.2.1/24 dev router
sudo ip netns exec a ip link set dev router up
sudo ip netns exec a "$(command -v uv)" run install_routes.py --gateway 192.168.2.2 \
    --dev router --protocol 4 --limit 1000

# 端末3: CLI で確認する
sudo ip netns exec a telnet localhost 9882
```

## 注意点

- **ログの容量:** リングがあふれると、落ちた経路ごとに警告が出ます。本番の計測1回で `logs/` が1GBを超えることがあります。
- **ログ出力の影響:** 警告の出力が netlink-thread の速度に影響している可能性があります。ソケットで落ちる構成では経路ごとの警告が出ないので、ログ量はロスの大きさを表しません。
- **計測中の `show`:** `show` は lthread のコアを止めるので、投入中には実行しません。
- **投入速度:** 実行環境や構成で変わります。ロス率は速度に左右されるので、わずかな差は誤差と見てください。
- **`rib-lthread-netlink-lthread.conf` の lcore:** EAL が `-l 0-1` なので、`router` を割り当てた lcore 2 は起動しません。参考結果の測定時と同じ設定です。経路の処理には関係しないはずです。
- **rib-lcore の起動:** 起動時の経路の一括取得が `router-if` の登録より先に届き、接続経路が捨てられることがあります。`measure_route_loss.py` はアドレスを付け直して再通知させます。
- **`set pager`:** 設定ファイルに `set pager` を書かないでください。ファイルを256バイトずつ読んだ最後の塊が1行だけになると、libsdplane の `pager_start()` が EOF で無限ループし、起動が固まります。
- **経路が残った場合:** `SIGKILL` などで中断すると、カーネルに経路が残ることがあります。

  ```sh
  sudo ip netns exec a ip -4 route flush table 254 proto 4 metric 42760
  ```

- **`--source fib` のロス率:** より長い経路でアドレス範囲がすべて覆われた経路は、`show fib` に行が出ません。経路が失われていなくても、ロスとして数えられます（10万経路で約6%）。`ring_lost` と `socket_drops` も合わせて見てください。
- **対象範囲:** IPv4 の経路追加のみです。`--source fib` で FIB と比較できますが、経路削除の計測はありません。

## 片付け

```sh
sudo ./setup_netns.py down
```
