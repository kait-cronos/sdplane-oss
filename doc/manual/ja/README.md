# sdplane-oss ユーザーガイド

**言語 / Language:** [English](../en/README.md) | **日本語** | [Français](../fr/README.md) | [中文](../zh/README.md) | [Deutsch](../de/README.md) | [Italiano](../it/README.md) | [한국어](../ko/README.md) | [ไทย](../th/README.md) | [Español](../es/README.md)

sdplane-ossは高性能なDPDKベースのソフトウェアルーターです。このユーザーガイドでは、sdplaneの全コマンドと機能について説明します。

## 目次

1. [ポート管理・統計](port-management.md) - DPDKポートの管理と統計情報
2. [ワーカー・lcore管理・スレッド情報](worker-lcore-thread-management.md) - ワーカースレッド、lcore、スレッド情報の管理

3. [デバッグ・ログ](debug-logging.md) - デバッグとログ機能
4. [VTY・シェル管理](vty-shell.md) - VTYとシェルの管理
5. [システム情報・監視](system-monitoring.md) - システム情報と監視機能
6. [RIB・ルーティング](routing.md) - RIBとルーティング機能
7. [キュー設定](queue-configuration.md) - キューの設定と管理
8. [パケット生成](packet-generation.md) - PKTGENを使用したパケット生成

9. [TAPインターフェース](tap-interface.md) - TAPインターフェースの管理
10. [lthread管理](lthread-management.md) - lthreadの管理
11. [デバイス管理](device-management.md) - デバイスとドライバーの管理

## 基本的な使い方

### 接続方法

sdplaneに接続するには：

```bash
# sdplaneを起動
sudo ./sdplane/sdplane

# 別のターミナルからCLIに接続
telnet localhost 9882
```

### ヘルプの表示

各コマンドでは `?` を使用してヘルプを表示できます：

```
sdplane# ?
sdplane# show ?
sdplane# set ?
```

### 基本的なコマンド

- `show version` - バージョン情報の表示
- `show port` - ポート情報の表示
- `show worker` - ワーカー情報の表示
- `exit` - CLIからの終了

## コマンドの分類

sdplaneには79個のコマンドが定義されており、以下の13の機能カテゴリに分類されています：

1. **ポート管理・統計** (10コマンド) - DPDKポートの制御と統計
2. **ワーカー・lcore管理** (6コマンド) - ワーカースレッドとlcoreの管理
3. **デバッグ・ログ** (2コマンド) - デバッグとログ機能
4. **VTY・シェル管理** (4コマンド) - VTYとシェルの制御
5. **システム情報・監視** (10コマンド) - システム情報と監視
6. **RIB・ルーティング** (1コマンド) - ルーティング情報の管理
7. **キュー設定** (3コマンド) - キューの設定
8. **パケット生成** (3コマンド) - PKTGENによるパケット生成

10. **TAPインターフェース** (2コマンド) - TAPインターフェースの管理
11. **lthread管理** (3コマンド) - lthreadの管理
12. **デバイス管理** (2コマンド) - デバイスとドライバーの管理

詳細な使用方法については、各カテゴリのドキュメントを参照してください。