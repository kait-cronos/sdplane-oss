# DPDKアプリケーション統合ガイド

**言語 / Language:** [English](../dpdk-integration-guide.md) | **日本語** | [Français](../fr/dpdk-integration-guide.md) | [中文](../zh/dpdk-integration-guide.md) | [Deutsch](../de/dpdk-integration-guide.md) | [Italiano](../it/dpdk-integration-guide.md) | [한국어](../ko/dpdk-integration-guide.md) | [ไทย](../th/dpdk-integration-guide.md) | [Español](../es/dpdk-integration-guide.md)

このガイドでは、既存のDPDKアプリケーションとそのpthreadベースのワーカースレッドをsdplaneフレームワークにDPDK-dock方式を使用して統合する方法を説明します。

## 概要

sdplaneは、**DPDK-dock方式**を使用して複数のDPDKアプリケーションを単一のプロセス空間内で実行するための統合フレームワークを提供します。各DPDKアプリケーションが独自のEAL環境を初期化する代わりに、アプリケーションをsdplaneの協調的スレッディングモデル内のワーカーモジュールとして統合できます。

DPDK-dock方式では、以下により複数のDPDKアプリケーションが効率的にリソースを共有し共存できます：
- DPDK EAL初期化の一元化
- 共有メモリプール管理の提供
- ポートとキュー設定の統一
- 異なるDPDKワークロード間での協調的マルチタスク処理の実現

## 主要な統合ポイント

### ワーカースレッドの変換
- 従来のDPDK pthreadワーカーをsdplane lcoreワーカーに変換
- `pthread_create()`をsdplaneの`set worker lcore <id> <worker-type>`に置き換え
- sdplaneのlcoreベースのスレッディングモデルと統合

### 初期化の統合
- アプリケーション固有の`rte_eal_init()`呼び出しを削除
- sdplaneの一元化されたEAL初期化を使用
- sdplaneのコマンドフレームワーク経由でアプリケーション固有のCLIコマンドを登録

### メモリとポートの管理
- sdplaneの共有メモリプール管理を活用
- sdplaneのポート設定とキュー割り当てシステムを使用
- ポート状態のためにsdplaneのRIB（Routing Information Base）と統合

## 統合手順

### 1. ワーカー関数の特定
DPDKアプリケーション内の主要なパケット処理ループを特定します。これらは通常、以下の特徴を持つ関数です：
- パケットを処理する無限ループで動作
- `rte_eth_rx_burst()`と`rte_eth_tx_burst()`を使用
- パケット転送または処理ロジックを処理

### 2. ワーカーモジュールの作成
sdplaneのワーカーインターフェースに従ってワーカー関数を実装：

```c
static __thread uint64_t loop_counter = 0;

int
my_worker_function(__rte_unused void *dummy)
{
    unsigned lcore_id = rte_lcore_id();
    int thread_id;
    
    // 監視用のループカウンターを登録
    thread_id = thread_lookup_by_lcore(my_worker_function, lcore_id);
    thread_register_loop_counter(thread_id, &loop_counter);
    
    while (!force_quit && !force_stop[lcore_id]) {
        // パケット処理ロジックをここに記述
        
        // 監視用ループカウンターをインクリメント
        loop_counter++;
    }
    
    return 0;
}
```

### ワーカーループカウンター監視

`loop_counter`変数により、sdplaneシェルからワーカーのパフォーマンスを監視できます：

- **スレッドローカル変数**: 各ワーカーが独自のループカウンターを保持
- **登録**: ワーカー名とlcore IDを使用してsdplaneの監視システムにカウンターを登録
- **インクリメント**: メインループの各反復でカウンターが増加
- **監視**: sdplane CLIからカウンター値を表示してワーカーの活動を確認

**CLI監視コマンド:**
```bash
# ループカウンターを含むスレッドカウンター情報を表示
show thread counter

# 一般的なスレッド情報を表示
show thread

# ワーカー設定と状態を表示
show worker
```

これにより、管理者はワーカーが活発に処理を行っていることを確認し、ループカウンターの増分を観察することで潜在的なパフォーマンス問題やワーカーの停止を検出できます。

### 3. RCUを使用したRIB情報へのアクセス

DPDKパケット処理ワーカー内でポート情報と設定にアクセスするため、sdplaneはスレッドセーフな操作のためのRCU（Read-Copy-Update）を通じてRIB（Routing Information Base）アクセスを提供します。

#### RIBアクセスパターン

```c
#if HAVE_LIBURCU_QSBR
#include <urcu/urcu-qsbr.h>
#endif /*HAVE_LIBURCU_QSBR*/

static __thread struct rib *rib = NULL;

int
my_worker_function(__rte_unused void *dummy)
{
    unsigned lcore_id = rte_lcore_id();
    int thread_id;
    
    // 監視用のループカウンターを登録
    thread_id = thread_lookup_by_lcore(my_worker_function, lcore_id);
    thread_register_loop_counter(thread_id, &loop_counter);
    
#if HAVE_LIBURCU_QSBR
    urcu_qsbr_register_thread();
#endif /*HAVE_LIBURCU_QSBR*/

    while (!force_quit && !force_stop[lcore_id]) {
#if HAVE_LIBURCU_QSBR
        urcu_qsbr_read_lock();
        rib = (struct rib *) rcu_dereference(rcu_global_ptr_rib);
#endif /*HAVE_LIBURCU_QSBR*/

        // パケット処理ロジックをここに記述
        // rib->rib_info->port[portid]を通じてポート情報にアクセス
        
#if HAVE_LIBURCU_QSBR
        urcu_qsbr_read_unlock();
        urcu_qsbr_quiescent_state();
#endif /*HAVE_LIBURCU_QSBR*/

        loop_counter++;
    }

#if HAVE_LIBURCU_QSBR
    urcu_qsbr_unregister_thread();
#endif /*HAVE_LIBURCU_QSBR*/
    
    return 0;
}
```

#### ポート情報へのアクセス

RIBを取得後、ポート固有の情報にアクセス：

```c
// ポートリンク状態を確認
if (!rib->rib_info->port[portid].link.link_status) {
    // ポートがダウンしている、処理をスキップ
    continue;
}

// ポートが停止しているかチェック
if (unlikely(rib->rib_info->port[portid].is_stopped)) {
    // ポートが管理的に停止している
    continue;
}

// ポート設定にアクセス
struct port_config *port_config = &rib->rib_info->port[portid];

// lcoreキュー設定を取得
struct lcore_qconf *lcore_qconf = &rib->rib_info->lcore_qconf[lcore_id];
for (i = 0; i < lcore_qconf->nrxq; i++) {
    portid = lcore_qconf->rx_queue_list[i].port_id;
    queueid = lcore_qconf->rx_queue_list[i].queue_id;
    // このポート/キューからパケットを処理
}
```

#### RCU安全性ガイドライン

- **スレッド登録**: 常に`urcu_qsbr_register_thread()`でスレッドを登録
- **読み込みロック**: RIBデータにアクセスする前に読み込みロックを取得
- **逆参照**: RCU保護されたポインターに安全にアクセスするため`rcu_dereference()`を使用
- **静止状態**: 安全なポイントを示すために`urcu_qsbr_quiescent_state()`を呼び出し
- **スレッドクリーンアップ**: `urcu_qsbr_unregister_thread()`でスレッドを登録解除

#### RIBデータ構造

RIBを通じて利用可能な主要情報：
- **ポート情報**: リンク状態、設定、統計
- **キュー設定**: lcoreからポート/キューへの割り当て
- **VLAN設定**: 仮想スイッチとVLAN設定（高度な機能用）
- **インターフェース設定**: TAPインターフェースとルーティング情報

### 4. CLIコマンドの追加
sdplaneのCLIシステムにアプリケーション固有のコマンドを登録：

```c
CLI_COMMAND2(my_app_command,
             "my-app command <WORD>",
             "my application\n",
             "command help\n")
{
    // コマンド実装
    return 0;
}

// 初期化関数内
INSTALL_COMMAND2(cmdset, my_app_command);
```

### 4. ビルドシステムの設定
モジュールを含むようにビルド設定を更新：

```makefile
# sdplane/Makefile.amに追加
sdplane_SOURCES += my_worker.c my_worker.h
```

### 5. 統合テスト
sdplaneの設定を使用して機能を検証：

```bash
# sdplane設定内
set worker lcore 1 my-worker-type
set thread 1 port 0 queue 0

# ワーカー開始
start worker lcore all
```

## 統合済みアプリケーション

以下のDPDKアプリケーションが、DPDK-dock方式を使用してsdplaneへの統合に成功しています：

### L2FWD統合 (`module/l2fwd/`)
**ステータス**: ✅ 統合成功
- オリジナルのDPDK l2fwdアプリケーションをsdplane用に適応
- ポート間の基本的なパケット転送を実証
- **参考資料**: オリジナルDPDK l2fwdソースコード用のパッチファイルが利用可能
- pthread-to-lcoreワーカー変換を実証
- 主要ファイル: `module/l2fwd/l2fwd.c`、統合パッチ

### PKTGEN統合 (`module/pktgen/`)
**ステータス**: ✅ 統合成功
- 完全なDPDK PKTGENアプリケーション統合
- 複雑なマルチスレッドアプリケーションの例
- sdplane CLIとの外部ライブラリ統合
- 高度な統合技術を実証
- 主要ファイル: `module/pktgen/app/`、`sdplane/pktgen_cmd.c`

### L3FWD統合
**ステータス**: ⚠️ 部分的統合（完全機能していない）
- 統合を試みたが追加作業が必要
- 新しい統合の参考としては推奨されない
- L2FWDとPKTGENを主要な例として使用

## 推奨統合リファレンス

新しいDPDKアプリケーションを統合する開発者には、以下を主要な参考として使用することを推奨します：

### 1. L2FWD統合（推奨）
- **場所**: `module/l2fwd/`
- **パッチファイル**: オリジナルDPDK l2fwdソース用が利用可能
- **適用範囲**: 基本的なpthread-to-lcore変換の理解
- **統合パターン**: シンプルなパケット転送ワーカー

### 2. PKTGEN統合（高度な参考）
- **場所**: `module/pktgen/`と`sdplane/pktgen_cmd.c`
- **適用範囲**: CLIコマンド付きの複雑なアプリケーション統合
- **統合パターン**: 外部依存関係を持つマルチコンポーネントアプリケーション

### 統合パッチファイル

L2FWD統合について、オリジナルのDPDK l2fwdアプリケーションをsdplane統合用に適応するために必要な正確な修正を示すパッチファイルが利用可能です。これらのパッチは以下を実証します：
- EAL初期化の削除
- スレッディングモデルの適応
- CLIコマンド統合
- リソース管理の変更

開発者は、DPDKアプリケーション統合への体系的なアプローチを理解するために、これらのパッチファイルを参照すべきです。

## カスタムワーカーの例

### L2リピーター (`sdplane/l2_repeater.c`)
カスタムsdplaneワーカー（既存DPDKアプリベースではない）：
- シンプルなパケット転送ワーカー
- すべてのアクティブポートへのブロードキャスト転送
- sdplaneのポート管理との統合

### 拡張リピーター (`sdplane/enhanced_repeater.c`)
高度な機能を持つカスタムsdplaneワーカー：
- VLAN対応スイッチング
- TAPインターフェース統合
- ポート状態のためのRIB統合

### VLANスイッチ (`sdplane/vlan_switch.c`)
カスタムレイヤー2スイッチング実装：
- MAC学習と転送
- VLAN処理
- マルチポートパケット処理

## ベストプラクティス

### パフォーマンスの考慮事項
- 最適なパフォーマンスのためにバースト処理を使用
- 可能な限りパケットコピーを最小化
- DPDKのゼロコピーメカニズムを活用
- ワーカー割り当てでNUMAトポロジーを考慮

### エラー処理
- DPDK関数の戻り値をチェック
- 適切なシャットダウン処理を実装
- 適切なログレベルを使用
- リソースクリーンアップを適切に処理

### スレッディングモデル
- sdplaneのlcoreベースのスレッディングを理解
- 効率的なパケット処理ループを設計
- 適切な同期メカニズムを使用
- スレッドアフィニティとCPU分離を考慮

## デバッグと監視

### デバッグログ
ワーカーのデバッグログを有効化：

```bash
debug sdplane my-worker-type
```

### 統計収集
sdplaneの統計フレームワークと統合：

```c
// ポート統計の更新
port_statistics[portid].rx += nb_rx;
port_statistics[portid].tx += nb_tx;
```

### CLI監視
監視用のステータスコマンドを提供：

```bash
show my-worker status
show my-worker statistics
```

## 一般的な統合パターン

### パケット処理パイプライン
```c
// sdplaneワーカーでの典型的なパケット処理
while (!force_quit && !force_stop[lcore_id]) {
    // 1. パケット受信
    nb_rx = rte_eth_rx_burst(portid, queueid, pkts_burst, MAX_PKT_BURST);
    
    // 2. パケット処理
    for (i = 0; i < nb_rx; i++) {
        // 処理ロジック
        process_packet(pkts_burst[i]);
    }
    
    // 3. パケット送信
    rte_eth_tx_burst(dst_port, queueid, pkts_burst, nb_rx);
}
```

### 設定統合
```c
// sdplaneの設定システムに登録
struct worker_config my_worker_config = {
    .name = "my-worker",
    .worker_func = my_worker_function,
    .init_func = my_worker_init,
    .cleanup_func = my_worker_cleanup
};
```

## トラブルシューティング

### 一般的な問題
- **ワーカーが開始しない**: lcore割り当てと設定を確認
- **パケットドロップ**: キュー設定とバッファサイズを確認
- **パフォーマンス問題**: CPUアフィニティとNUMA設定を確認
- **CLIコマンドが動作しない**: 適切なコマンド登録を確認

### デバッグ技法
- sdplaneのデバッグログシステムを使用
- ワーカー統計とカウンターを監視
- ポートリンク状態と設定を確認
- メモリプール割り当てを検証

詳細な実装例については、コードベース内の既存のワーカーモジュールとそれに対応するCLIコマンド定義を参照してください。