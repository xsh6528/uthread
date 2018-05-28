#!/bin/bash

echo "Provisioning VM..."

sudo apt-get update

# Setup ZSH.
sudo apt-get install git zsh -y
git clone https://github.com/robbyrussell/oh-my-zsh.git /home/vagrant/.oh-my-zsh
sudo chsh -s /bin/zsh vagrant

# Setup core packages.
sudo DEBIAN_FRONTEND=noninteractive apt-get install -y \
    build-essential \
    cmake           \
    libffi-dev      \
    libssl-dev      \
    manpages-dev    \
    tmux            \
    tree            \

# GTest
GTEST=googletest-release-1.8.0
curl -sSL -o $GTEST.tar.gz https://github.com/google/googletest/archive/release-1.8.0.tar.gz
tar -xf $GTEST.tar.gz
mkdir -p $GTEST/googletest/build
cd $GTEST/googletest/build
cmake ..
make -j8
sudo cp -r ../include/gtest /usr/local/include
sudo cp lib*.a /usr/local/lib

# GLog
GLOG=glog-0.3.5
curl -sSL -o $GLOG.tar.gz https://github.com/google/glog/archive/v0.3.5.tar.gz
tar -xf $GLOG.tar.gz
mkdir -p $GLOG/build
cd $GLOG/build
cmake -DGFLAGS_NAMESPACE=google -DCMAKE_CXX_FLAGS=-fPIC ..
make -j8
sudo make install

# GBench
GBENCH=benchmark-1.3.0
curl -sSL -o $GBENCH.tar.gz https://github.com/google/benchmark/archive/v1.3.0.tar.gz
tar -xf $GBENCH.tar.gz
mkdir -p $GBENCH/build
cd $GBENCH/build
cmake .. -DCMAKE_BUILD_TYPE=RELEASE
make -j8
sudo make install

# libevent
LIBEVENT=libevent-release-2.1.8-stable
curl -sSL -o $LIBEVENT.tar.gz https://github.com/libevent/libevent/archive/release-2.1.8-stable.tar.gz
tar -xf $LIBEVENT.tar.gz
mkdir -p $LIBEVENT/build
cd $LIBEVENT/build
cmake .. -DCMAKE_BUILD_TYPE=RELEASE
make -j8
sudo make install

echo "Done!"
