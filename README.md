# Xbyak on AAarch64 ハンズオン

## はじめに

これはAArch64アーキテクチャでXbyakを使ってみるためのハンズオンの資料である。DockerでArchLinuxのイメージを作り、その中で`aarch64-linux-gnu-gcc`を使ってクロスコンパイル、QEMUで実行する。

## Dockerfileについて

`/etc/pacman.d/mirrorlist`の先頭に`Server = https://ftp.jaist.ac.jp/pub/Linux/ArchLinux/$repo/os/$arch`を追加し、ミラーとしてJAISTを指定。これをしないとダウンロード中にpacmanがタイムアウトしてDockerファイルのビルドに失敗することがある。

## Licence

MIT
