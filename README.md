# macos-opensh

## Overview

"macos-opensh" is the first example application of the
GNU-Zebra/Quagga/FRR-like command-line shell library.
The "opensh" is a Cisco-like shell for "open"-ing files and directories
in MacOS. It shows a powerful ability when the name of the
file or directory is in the multi-byte language, e.g.,
the Jananese UTF-8 filenames.

"macos-opensh" is a derivative of a proprietary program,
and in the hope for me to freely dual-licensing the proprietary program,
(and because I haven't prepared the necessary preparation yet,)
for now I don't accept pull-requests.

## License

Proprietary.

## Installation

- Homebrew's gcc and automake/autoconf are the prerequisites.
- Below will install the opensh in $HOME/macos/bin/.

```
% aclocal
% autoheader
% automake --add-missing -c
% autoconf
% ./configure
% make && make install
```

% build-x86_64.sh
% build-arm64.sh
   or
% build-universal.sh

app/opensh.arm64:
macOS 11.0 Big Sur 以上で動作する Apple Silicon 向けバイナリ
app/opensh.x86_64:
macOS 10.5 Leopard 以上で動作する Intel CPU 向けバイナリ
app/opensh.universal:
Universal Binary.

## Usage

'?' provides a list of command candidates.
When your shell is in the <FILENAME> command node,
'?' provides a list of file candidates.
'TAB' will complete the command argument
as long as it is common among the candidates.

If you type '>' in the <FILENAME> command node, the shell enters
the ''file select mode'', where vi or emacs cursor move is accepted.
If you 'enter' in the file select mode, the candidate the cursor was on
is expanded in the command line.
'q' in the file select mode cancels the file select mode.

```
yasu@yasuhironets-MBP ~ % ~/macos/bin/opensh
[/Users/yasu]> <'?'>
  cd               change current working directory.
  chdir            change current working directory.
  pwd              print current working directory.
  terminal         open a new terminal window.
  show             show information.
  exit             exit
  no               disable command.
  debug            debug command.
  vi               edit a file with vi.
  vim              edit a file with vim.
  quit             quit
  ls               list files in the directory.
  list             list files in the directory.
  open             open.
  launch           launch a command.
  logout           logout
[/Users/yasu]> cd <'?'>
  <cr>             change current working directory.
  <FILENAME>       filename

  devel/                paper/                Music/
  nttcom.txt            my-number-card/       macos/
  Creative Cloud Files/ Pictures/             Desktop/
  Library/              wish-list.txt         Public/
  Movies/               Applications/         Documents/
  memo.txt              game-server/          Downloads/
  tmp/                  event/                membership/
  src/
[/Users/yasu]> cd dev<tab>
[/Users/yasu]> cd devel/<ret>
chdir: devel/
[/Users/yasu/devel]> cd <'?'>
  <cr>             change current working directory.
  <FILENAME>       filename

  simrouting/                  bgpdump2/
  macos-opensh/                zcmdsh/
  テストディレクトリ/
[/Users/yasu/devel]> cd <'>'>
 [simrouting/]                 bgpdump2/
  macos-opensh/                zcmdsh/
  テストディレクトリ/
[/Users/yasu/devel]> cd <Ctrl-N>
  simrouting/                  bgpdump2/
 [macos-opensh/]               zcmdsh/
  テストディレクトリ/
[/Users/yasu/devel]> cd <ret>
  simrouting/                  bgpdump2/
  macos-opensh/                zcmdsh/
 [テストディレクトリ/]
[/Users/yasu/devel]> cd テストディレクトリ/<ret>
chdir: テストディレクトリ/
[/Users/yasu/devel/テストディレクトリ]> ls<ret>
dir: /Users/yasu/devel/テストディレクトリ/

  ランダム 空白入り日本語ファイル名.pdf

[/Users/yasu/devel/テストディレクトリ]> open <'>'>
 [ランダム 空白入り日本語ファイル名.pdf]
[/Users/yasu/devel/テストディレクトリ]> open <ret>
[/Users/yasu/devel/テストディレクトリ]> open ランダム 空白入り日本語ファイル名.pdf<ret>
(Opens the PDF.)
```

## Known Issues

- Removing the UTF-8 filename backward from the end does not work yet.
  (The shell removes it one-byte at a time in (e.g.,) UTF-8 four encoded bytes.
- The spaces in directory names are not handled correctly.
- The more-than-one spaces in file name is not handled correctly.
  (It is canonicalized as just one space in the command matching process.)

## TODO

- multi-match in command match process.

