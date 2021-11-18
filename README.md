# Xbyak on AAarch64 ハンズオン

## はじめに

これはAArch64アーキテクチャでXbyakを使ってみるためのハンズオンの資料である。DockerでArchLinuxのイメージを作り、その中で`aarch64-linux-gnu-gcc`を使ってクロスコンパイル、QEMUで実行する環境を構築している。Docker Imageができたら、その中でAArch64アーキテクチャの、特にSVEと呼ばれる特徴的なSIMD命令セットについて体験する。

なお、Dockerの環境構築とC++言語の基礎的な知識については前提とする。

## 基礎知識

### なぜSIMDなのか

SIMDとはいわゆるフリンの4分類の一つで、Single Instruction Multiple Dataの略である。幅の広いレジスタに複数のデータ(Multiple Data)をつめて、「せーの」でその全てに同じ演算(Single Instruction)を実施する。ベクトル同士の和や差のような計算をするため、ベクトル演算とも呼ばれる。コードがうまくSIMD命令を使えるように修正することを「SIMDベクトル化」もしくは「SIMD化」と呼ぶ。一般にSIMD化は面倒な作業であり、できるならやりたくはない。ではなぜSIMD化について考えなければならないのか。それは、現代のCPUコアの性能を引き出すには、ほぼSIMD化が必須だからだ。

CPUは、簡単にいえばメモリからデータをとってきて、演算器に放り込み、演算結果をメモリに書き戻す作業を高速に繰り返す機械だ。CPUは「サイクル」という単位で動いており、一秒間に何サイクル回せるかが動作周波数と呼ばれるものだ。一般に演算器がデータを受け取ってから結果を返すまでに数サイクルかかるが、パイプライン処理という手法により、事実上「1サイクルに1演算」できるようになった。したがって、このまま性能を向上させるためには動作周波数を上げる必要があるが、これは2000年代に発熱により破綻した。

1サイクルに1演算できて、動作周波数を上げることができないのだから、性能向上のためには1サイクルに複数の演算をするしかない。その手段がSIMDである。コンパイラによる自動SIMD化は、「できる時はできる」し、「できない時はできない」。「できる時」はラッキーと思えば良いし、「どうしたってできない」時はあきらめればよいが、問題は「理論上はできるはずなのにできない」時だ。その時には手でSIMD化をするしかない。本ハンズオンはそんな不幸な人のために書かれたものである。

### SVEとは何か

TODO: SVEの説明

SVE命令を使うにはいくつかの方法がある。

1. 「コンパイラに完全に任せる」最も基本的な方法である。コンパイラオプションで「可能な限り最適化せよ」と指示すると、ほとんどの場合においてSIMD命令を積極的に使ってくれる。最近のコンパイラは賢くなっており、比較的単純なコードであればSIMDベクトル化もかなり良い感じにやってくれる。
2. 「コンパイラに指示を与える」コンパイラには判断できない条件によりSIMDベクトル化ができない場合、人間がそれをコンパイラにOpenMPのsimdディレクティブ等で教えてやることでうまくSIMDベクトル化ができる場合がある。また、多重ループの内側と外側のどちらをSIMD化した方が良いかはコードによって異なるが、コンパイラの判断とは異なるループをSIMD化したい場合なども人間が適宜指示することで性能が向上する場合がある。また、コンパイラがSIMD化しやすいようにデータ構造を変えるのも有効だ。このレベルでは、コンパイラの出力するレポートをにらみながら指示やオプションを変更し、性能評価を繰り返す。
3. 「組み込み関数を使う」どうしてもコンパイラが思うようなコードを吐いてくれなかった場合、組み込み関数を使って人間が直接SIMD命令を書く。特にシャッフル命令を多用する必要がある場合などはコンパイラはSIMD化できないため、人間が手で書く必要がある。原則として組み込み関数はアセンブリと一対一対応しているため、ほぼほぼアセンブリで書くのと同じような感覚になる。ただし、フルアセンブリで書くよりは組み込み関数で書いた方がいろいろ楽。
4. 「Xbyakを使う」。JITアセンブリであるXbyakを使ってコードを書く。やはりアセンブリと一対一対応した関数を呼び出すことで、アセンブリを「実行時に組み立てて」いくイメージとなる。組み込み関数と異なり、ABIの知識が必要となる。
5. 「フルアセンブリ/インラインアセンブリで書く」。数値計算でここまでやる人は少ないと思われる。Xbyakが使えるなら同じことができるはずなので、現在はアセンブリで直接書くメリットはあまりないであろう。

人間が手を入れるレベルが「コンパイラオプション」「ソースコードにディレクティブを入れる」「組み込み関数を使う」「アセンブリを書く」と、徐々に低くなっているのがわかるであろう。個人的な意見としては、人間がアセンブリを書くのは大変なのでそこまでやらなくても、という気はするが、アセンブリ自体は知っておいて方が良いと考える。少なくともコンパイラのレポートを詳細に読み込むよりも、コンパイラが吐いたアセンブリを読む方が早いことが多いからだ。

## Xbyakの概要

## ハンズオン

### Dockerイメージのビルドと動作確認

SVE命令を使うクロスコンパイラや、QEMUで実行できる環境がDockerファイルとして用意してある。

適当な場所でこのリポジトリをクローンしよう。

```sh
git clone https://github.com/kaityo256/xbyak_aarch64_handson.git
cd xbyak_aarch64_handson
```

`docker`というディレクトリに入って`make`する。

```sh
cd docker
make
```

すると、`kaityo256/xbyak_aarch64_handson`というイメージができる。イメージができたら`make run`でDockerイメージの中に入ることができる。

```sh
$ make run
docker run -e GIT_USER= -e GIT_TOKEN= -it kaityo256/xbyak_aarch64_handson
[user@22e98a6d9601 ~]$
```

以下はDockerコンテナの中での作業である。

環境変数`GIT_USER`や`GIT_TOKEN`は開発用に渡しているものなので気にしないで欲しい。デフォルトで`user`というアカウントでログインするが、`root`というパスワードで`su -`できるので、必要なパッケージがあれば適宜`pacman`でインストールすること。

Dockerイメージの中に入ったら、`xbyak_aarch64_handson/sample`の中にサンプルコードがある。最初に、ARM SVE向けの組み込み関数`svcntb`をコンパイル、実行できるか確認してみよう。コードは`01_sve_length`の中にあり、`make`すればビルド、`make run`すれば実行できる。

```sh
$ cd xbyak_aarch64_handson
$ cd sample
$ cd 01_sve_length
$ make
aarch64-linux-gnu-g++ -static -march=armv8-a+sve -O2 sve_length.cpp
$ make run
qemu-aarch64 ./a.out
SVE is available. The length is 512 bits
```

上記のように「SVE is available. The length is 512 bits」という実行結果が出たら成功だ。

ついでに、SVEがScalableであること、すなわち、同じ実行バイナリで、異なるSIMDベクトル長のハードウェアに対応できることも確認しておこう。QEMUに`-cpu`オプションを指定することでどのようなCPUをエミュレートするか指示できる。例えば`-cpu max,sve128=on`を指定すると、SVEのベクトル長として128ビットのハードウェアを指定したことになる。

```sh
$ qemu-aarch64 -cpu max,sve128=on ./a.out
SVE is available. The length is 128 bits
```

同様に、256ビットや512ビットも指定できる。


```sh
$ qemu-aarch64 -cpu max,sve256=on ./a.out
SVE is available. The length is 256 bits

$ qemu-aarch64 -cpu max,sve512=on ./a.out
SVE is available. The length is 512 bits
```

TODO: Xbyakの動作確認

### 組み込み関数

### Xbyak_aarch64

## Dockerfileについて

Dockerfileの中身について簡単に説明しておく。

### ディストリビューション

```Dockerfile
FROM archlinux
```

ある程度新しいGCCでないとARM SVE組み込み関数に対応しておらず、パッケージマネージャで簡単にインストールできるディストリビューションが(少なくとも試した当時は)ArchLinuxしかなかったため、ArchLinuxを使っている。

### 環境変数の設定

あとで使う環境変数をいくつか設定してある。

```Dockerfile
ENV USER user
ENV HOME /home/${USER}
ENV SHELL /bin/bash
ENV GIT_REPOSITORY kaityo256/xbyak_aarch64_handson
```

### ユーザ追加

```Dockerfile
RUN useradd -m ${USER}
RUN echo 'user:userpass' | chpasswd
RUN echo 'root:root' | chpasswd
```

デフォルトユーザである`user`と、後で必要になった時のために`root`パスワードを`root`に設定してある。

### パッケージのインストール

```Dockerfile
RUN sed -i '1iServer = https://ftp.jaist.ac.jp/pub/Linux/ArchLinux/$repo/os/$arch' /etc/pacman.d/mirrorlist
RUN pacman -Syyu --noconfirm && \
  pacman -S --noconfirm \
  aarch64-linux-gnu-gcc \
  git \
  make \
  vim \
  qemu \
  qemu-arch-extra
```

ArchLinuxのパッケージマネージャは`pacman`だ。例えばパッケージのアップデート(`yum update`や`apt update && apt upgrade`にあたるもの)は`pacman -Syyu`である。通常は「インストールしますか？」と聞いてくるので、それを防ぐために`--noconfirm`をつけている。

パッケージのアップデート、インストールの前に、`/etc/pacman.d/mirrorlist`の先頭に`Server = https://ftp.jaist.ac.jp/pub/Linux/ArchLinux/$repo/os/$arch`を追加することでミラーとしてJAISTを指定している。これをしないとダウンロード中にpacmanがタイムアウトしてDockerファイルのビルドに失敗することがある。

### デフォルトユーザの設定

```Dockerfile
USER ${USER}
WORKDIR /home/${USER}
```

ずっとrootで作業するのも気持ち悪いので、デフォルトユーザを設定している。

### 開発環境の準備

```sh
RUN echo 'alias vi=vim' >> /home/${USER}/.bashrc
RUN echo 'alias ag++="aarch64-linux-gnu-g++ -static -march=armv8-a+sve -O2"' >> /home/${USER}/.bashrc
RUN echo 'alias gp="git push https://${GIT_USER}:${GIT_TOKEN}@github.com/${GIT_REPOSITORY}"' >> /home/${USER}/.bashrc
COPY dot.vimrc /home/${USER}/.vimrc
COPY dot.gitconfig /home/${USER}/.gitconfig
```

開発に必要な設定。特にクロスコンパイラはコマンドも長いしオプションも長いので、`ag++`にエイリアスを設定してある。`gp`はDocker内部から`git push`するための設定なので無視してかまわない。Vimの設定にこだわりがある人は`dot.vimrc`を修正すれば、コンテナの中に持ち込むことができる。`emacs`が使いたい人は適宜`Dockerfile`を書き換えるなり、`su -`してから`pacman -S --noconfirm emacs`すること。

## Licence

MIT
