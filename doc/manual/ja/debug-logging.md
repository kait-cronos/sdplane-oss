# デバッグ・ログ

**言語 / Language:** [English](../debug-logging.md) | **日本語** | [Français](../fr/debug-logging.md) | [中文](../zh/debug-logging.md) | [Deutsch](../de/debug-logging.md) | [Italiano](../it/debug-logging.md) | [한국어](../ko/debug-logging.md) | [ไทย](../th/debug-logging.md) | [Español](../es/debug-logging.md)

sdplaneのデバッグとログ機能を制御するコマンドです。

## コマンド一覧

### log_file - ログファイル出力設定
```
log file <ファイルパス>
```

ログをファイルに出力するよう設定します。

**パラメータ：**
- `<ファイルパス>` - ログを出力するファイルのパス

**使用例：**
```bash
# ログを指定ファイルに出力
log file /var/log/sdplane.log

# デバッグ用ログファイル
log file /tmp/sdplane-debug.log
```

### log_stdout - 標準出力ログ設定
```
log stdout
```

ログを標準出力に出力するよう設定します。

**使用例：**
```bash
# 標準出力にログを表示
log stdout
```

**注意：** `log file`と`log stdout`は同時に設定可能で、両方にログが出力されます。

### debug - デバッグ設定
```
debug <カテゴリ> <ターゲット>
```

指定されたカテゴリの特定のターゲットに対してデバッグログを有効化します。

**カテゴリ：**
- `sdplane` - sdplaneメインカテゴリ
- `zcmdsh` - コマンドシェルカテゴリ

**sdplaneターゲット一覧：**
- `lthread` - 軽量スレッド
- `console` - コンソール
- `tap-handler` - TAPハンドラー
- `l2fwd` - L2転送
- `l3fwd` - L3転送
- `vty-server` - VTYサーバー
- `vty-shell` - VTYシェル
- `stat-collector` - 統計コレクタ
- `packet` - パケット処理
- `fdb` - FDB（転送データベース）
- `fdb-change` - FDB変更
- `rib` - RIB（ルーティング情報ベース）
- `vswitch` - 仮想スイッチ
- `vlan-switch` - VLANスイッチ
- `pktgen` - パケットジェネレーター
- `enhanced-repeater` - 拡張リピーター
- `netlink` - Netlinkインターフェース
- `neighbor` - 近隣管理
- `all` - すべてのターゲット

**使用例：**
```bash
# 特定のターゲットのデバッグを有効化
debug sdplane rib
debug sdplane fdb-change
debug sdplane pktgen

# すべてのsdplaneデバッグを有効化
debug sdplane all

# zcmdshカテゴリのデバッグ
debug zcmdsh shell
debug zcmdsh command
```

### no debug - デバッグ無効化
```
no debug <カテゴリ> <ターゲット>
```

指定されたカテゴリの特定のターゲットに対してデバッグログを無効化します。

**使用例：**
```bash
# 特定のターゲットのデバッグを無効化
no debug sdplane rib
no debug sdplane fdb-change

# すべてのsdplaneデバッグを無効化（推奨）
no debug sdplane all

# zcmdshカテゴリのデバッグ無効化
no debug zcmdsh all
```

### show_debug_sdplane - sdplaneデバッグ情報表示
```
show debugging sdplane
```

現在のsdplaneデバッグ設定を表示します。

**使用例：**
```bash
show debugging sdplane
```

このコマンドは以下の情報を表示します：
- 現在有効なデバッグカテゴリ
- 各カテゴリのデバッグレベル
- 利用可能なデバッグオプション

## デバッグシステムの概要

sdplaneのデバッグシステムは、以下の特徴を持っています：

### カテゴリベースのデバッグ
- 異なる機能モジュールごとにデバッグカテゴリが分かれています
- 必要な機能のみデバッグログを有効化できます

### レベルベースの制御
- デバッグメッセージは重要度に応じてレベル分けされています
- 適切なレベルを設定することで、必要な情報のみを表示できます

### 動的設定
- システム稼働中にデバッグ設定を変更できます
- 再起動なしでデバッグレベルを調整可能です

## 使用方法

### 1. ログ出力先の設定
```bash
# ログファイルへの出力を設定（推奨）
log file /var/log/sdplane.log

# 標準出力への出力を設定
log stdout

# 両方を有効化（デバッグ時に便利）
log file /var/log/sdplane.log
log stdout
```

### 2. 現在のデバッグ設定確認
```bash
show debugging sdplane
```

### 3. デバッグターゲットの確認
`show debugging sdplane`コマンドで利用可能なターゲットとその状態を確認してください。

### 4. デバッグ設定の変更
```bash
# 特定のターゲットのデバッグを有効化
debug sdplane rib
debug sdplane fdb-change

# すべてのターゲットを一括有効化
debug sdplane all
```

### 5. デバッグログの確認
デバッグログは設定した出力先（ファイルまたは標準出力）に出力されます。

## トラブルシューティング

### デバッグログが出力されない場合
1. ログ出力先が設定されているか確認（`log file`または`log stdout`）
2. デバッグターゲットが正しく設定されているか確認（`debug sdplane <ターゲット>`）
3. 現在のデバッグ状態を確認（`show debugging sdplane`）
4. ログファイルのディスクサイズや権限を確認

### ログファイル管理
```bash
# ログファイルのサイズ確認
ls -lh /var/log/sdplane.log

# ログファイルのテール表示
tail -f /var/log/sdplane.log

# ログファイルの場所確認（設定ファイルでの例）
grep "log file" /etc/sdplane/sdplane.conf
```

### パフォーマンスへの影響
- デバッグログの有効化はパフォーマンスに影響を与える可能性があります
- 本番環境では必要最小限のデバッグのみを有効化することを推奨します
- ログファイルサイズが大きくなりすぎないよう定期的にローテーションしてください

## 設定例

### 基本的なログ設定
```bash
# 設定ファイルでの例（/etc/sdplane/sdplane.conf）
log file /var/log/sdplane.log
log stdout

# システム起動時のデバッグ有効化
debug sdplane rib
debug sdplane fdb-change
```

### デバッグ時の推奨設定
```bash
# デバッグ用詳細ログ設定
log file /tmp/sdplane-debug.log
log stdout

# 全ターゲットのデバッグを有効化（開発時のみ）
debug sdplane all

# 特定ターゲットのみ有効化する場合
debug sdplane rib
debug sdplane fdb-change
debug sdplane vswitch
```

### 本番環境での推奨設定
```bash
# 本番環境では標準ログのみ
log file /var/log/sdplane.log

# 必要に応じて重要なターゲットのみ有効化
# debug sdplane fdb-change
# debug sdplane rib
```

### デバッグ終了時の推奨操作
```bash
# すべてのデバッグを無効化
no debug sdplane all
no debug zcmdsh all
```

## 定義場所

これらのコマンドは以下のファイルで定義されています：
- `sdplane/debug_sdplane.c`

## 関連項目

- [システム情報・監視](system-monitoring.md)
- [VTY・シェル管理](vty-shell.md)