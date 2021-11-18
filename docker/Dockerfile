FROM archlinux
MAINTAINER kaityo256

ENV USER user
ENV HOME /home/${USER}
ENV SHELL /bin/bash
ENV GIT_REPOSITORY kaityo256/xbyak_aarch64_handson

RUN useradd -m ${USER}
RUN echo 'user:userpass' | chpasswd
RUN echo 'root:root' | chpasswd

RUN sed -i '1iServer = https://ftp.jaist.ac.jp/pub/Linux/ArchLinux/$repo/os/$arch' /etc/pacman.d/mirrorlist
RUN pacman -Syyu --noconfirm && \
  pacman -S --noconfirm \
  aarch64-linux-gnu-gcc \
  git \
  make \
  vim \
  qemu \
  qemu-arch-extra

USER ${USER}
WORKDIR /home/${USER}

RUN echo 'alias vi=vim' >> /home/${USER}/.bashrc
RUN echo 'alias ag++="aarch64-linux-gnu-g++ -static -march=armv8-a+sve -O2"' >> /home/${USER}/.bashrc
RUN echo 'alias gp="git push https://${GIT_USER}:${GIT_TOKEN}@github.com/${GIT_REPOSITORY}"' >> /home/${USER}/.bashrc
COPY dot.vimrc /home/${USER}/.vimrc
COPY dot.gitconfig /home/${USER}/.gitconfig

RUN git clone https://github.com/kaityo256/xbyak_aarch64_handson.git
