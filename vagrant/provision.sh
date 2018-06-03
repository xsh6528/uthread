#!/bin/bash

echo "Provisioning VM..."

sudo apt-get update

# Setup core packages.
sudo DEBIAN_FRONTEND=noninteractive apt-get install -y \
    build-essential \
    cmake           \
    git             \
    manpages-dev    \
    python3-pip     \
    tmux            \
    tree            \
    zsh

sudo pip3 install --prefix /usr/local cpplint

# Setup ZSH.
git clone https://github.com/robbyrussell/oh-my-zsh.git /home/vagrant/.oh-my-zsh
sudo chsh -s /bin/zsh vagrant

echo "Done!"
