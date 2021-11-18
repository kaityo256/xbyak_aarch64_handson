# Xbyak on AAarch64 ハンズオン

## はじめに

これは[高性能計算物理勉強会(HPC-Phys)](https://hpc-phys.kek.jp/)における、[Dockerで体験する富岳のアーキテクチャ「AArch64」ハンズオン](https://hpc-phys.kek.jp/workshop/workshop211125.html)のための資料である。DockerでArchLinuxのイメージを作り、その中で`aarch64-linux-gnu-gcc`を使ってクロスコンパイル、QEMUで実行する環境を構築している。Docker Imageができたら、その中でAArch64アーキテクチャの、特にSVEと呼ばれる特徴的なSIMD命令セットについて体験する。

なお、Dockerの環境構築とC++言語の基礎的な知識については前提とする。

**注意** 本資料は現在鋭意執筆中であり、以下の内容については変更される可能性が高いです。特にDockerはキャッシュの問題で、最新版のイメージが作成されない可能性があります。その場合の更新方法については当日お知らせいたします。

## 事前準備：Dockerのインストール

本ハンズオンではDockerを使うため、あらかじめDockerをインストールしておく必要がある。Windows、MacともにDockerのコミュニティ版であるDocker for Desktopをインストールして使うことになるだろう。Macならターミナルを、WindowsならWSL2にUbuntuをインストールして使うのが良いと思われる。Docker for Desktopのインストール後、ターミナルから

```sh
docker ps
```

を実行してエラーが出なければ正しくインストールができている。

```sh
$ docker ps
Cannot connect to the Docker daemon at unix:///var/run/docker.sock. Is the docker daemon running?
```

といったエラーが出た場合、正しくインストールされていないか、うまくDockerデーモンと接続できていない。

Dockerデーモンが動いていることがわかったら、イメージをダウンロードして実行できるか確認しよう。

```sh
$ docker run hello-world
Unable to find image 'hello-world:latest' locally
latest: Pulling from library/hello-world
2db29710123e: Pull complete
Digest: sha256:cc15c5b292d8525effc0f89cb299f1804f3a725c8d05e158653a563f15e4f685
Status: Downloaded newer image for hello-world:latest

Hello from Docker!
This message shows that your installation appears to be working correctly.
(以下略)
```

最初に`Unable to find image`と出てくるのは、ローカルにダウンロード済みのイメージが無いのでダウンロードするよ、という意味なので気にしなくて良い。最後に「Hello from Docker!」が表示されたら正しく実行できている。二度目以降の実行では、ローカルにイメージがキャッシュされているのでダウンロードされずに実行される。

```sh
$ docker run hello-world

Hello from Docker!
This message shows that your installation appears to be working correctly.
(以下略)
```

以下では、Dockerが正しくインストールされ、実行できることを前提とする。

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

#### Dockerイメージのビルド

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

すると、`kaityo256/xbyak_aarch64_handson`というイメージができる。なお、キャッシュの問題で最新版のイメージが作成されない場合がある。その場合は`make no-cache`とするとキャッシュを使わずにゼロからビルドしなおす。

イメージができたら`make run`でDockerイメージの中に入ることができる。

```sh
$ make run
docker run -e GIT_USER= -e GIT_TOKEN= -it kaityo256/xbyak_aarch64_handson
[user@22e98a6d9601 ~]$
```

以下はDockerコンテナの中での作業である。

環境変数`GIT_USER`や`GIT_TOKEN`は開発用に渡しているものなので気にしないで欲しい。デフォルトで`user`というアカウントでログインするが、`root`というパスワードで`su -`できるので、必要なパッケージがあれば適宜`pacman`でインストールすること。

#### 組み込み関数のテスト

Dockerイメージの中に入ったら、`xbyak_aarch64_handson/sample`の中にサンプルコードがある。最初に、ARM SVE向けの組み込み関数`svcntb`をコンパイル、実行できるか確認してみよう。コードは`intrinsic/01_sve_length`の中にあり、`make`すればビルド、`make run`すれば実行できる。

```sh
$ cd xbyak_aarch64_handson
$ cd sample
$ cd intrinsic
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

#### Xbyak_aarch64のテスト

次に、Xbyak_aarch64の動作テストをしてみよう。Xbyakのサンプルコードはリポジトリの`sample/xbyak`以下にある。まずはテストコードをコンパイル、実行してみよう。

```sh
$ cd xbyak_aarch64_handson
$ cd sample
$ cd xbyak
$ cd 01_test
$ make
aarch64-linux-gnu-g++ -static test.cpp -L/home/user/xbyak_aarch64_handson/xbyak_aarch64/lib -lxbyak_aarch64
$ ./a.out
1
```

`a.out`を実行して`1`と表示されたら成功だ。なお、実行時にQEMUにオプションを指定する必要がない場合、このように直接`a.out`を実行して構わない。もちろん`a.out`はARM向けバイナリなので、QEMUを通じて実行されている。

さて、ソースを見てみよう。

```cpp
#include <cstdio>
#include <xbyak_aarch64/xbyak_aarch64.h>

struct Code : Xbyak_aarch64::CodeGenerator {
  Code() {
    mov(w0, 1);
    ret();
  }
};

int main() {
  Code c;
  auto f = c.getCode<int (*)()>();
  c.ready();
  printf("%d\n", f());
}
```

ここで`mov(w0, 1)`の部分が関数の返り値を代入しているところだ。これを適当な値、例えば`mov(w0, 42)`にして、もう一度コンパイル、実行してみよう。

```sh
$ make
aarch64-linux-gnu-g++ -static test.cpp -L/home/user/xbyak_aarch64_handson/xbyak_aarch64/lib -lxbyak_aarch64
$ ./a.out
42
```

ちゃんと42が表示された。

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

```Dockerfile
RUN ln -s /usr/sbin/aarch64-linux-gnu-ar /usr/sbin/ar
RUN ln -s /usr/sbin/aarch64-linux-gnu-g++ /usr/sbin/g++
```

後でXbyakのライブラリをビルドするのに、Xbyakが`g++`や`ar`をネイテイブであることを前提に呼び出しているので、それに合わせてクロスコンパイラ`aarch64-linux-gnu-g++`を`g++`に、クロスアーカイバ`aarch64-linux-gnu-ar`を`ar`にそれぞれシンボリックリンクをはっている。ad hocな対応だが、ネイテイブなg++は使わないことと、Makefileをsedで書き換えるのもad hocさでは似たようなものだと思ってこの対応とした。

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
RUN echo 'export CPLUS_INCLUDE_PATH=/home/user/xbyak_aarch64_handson/xbyak_aarch64' >> /home/${USER}/.bashrc
COPY dot.vimrc /home/${USER}/.vimrc
COPY dot.gitconfig /home/${USER}/.gitconfig
```

開発に必要な設定。特にクロスコンパイラはコマンドも長いしオプションも長いので、`ag++`にエイリアスを設定してある。`CPLUS_INCLUDE_PATH`にXbyakのヘッダを探すパスを設定している。`gp`はDocker内部から`git push`するための設定なので無視してかまわない。Vimの設定にこだわりがある人は`dot.vimrc`を修正すれば、コンテナの中に持ち込むことができる。`emacs`が使いたい人は適宜`Dockerfile`を書き換えるなり、`su -`してから`pacman -S --noconfirm emacs`すること。

```Dockerfile
RUN git clone --recursive https://github.com/kaityo256/xbyak_aarch64_handson.git
RUN cd xbyak_aarch64_handson/xbyak_aarch64;make
```

最後に、コンテナ内でこのリポジトリをクローンしている。サブモジュールとしてxbyak_aarch64を登録しているので、`--recursive`オプションをつけている。そして、`libxbyak_aarch64.a`をビルドするために`make`している。

## Licence

MIT
