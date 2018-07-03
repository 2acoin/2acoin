#!/usr/bin/env bash

# Copyright (c) 2018, The TurtleCoin Developers
# Copyright (c) 2018, 2ACoin Developers
# 
# Please see the included LICENSE file for more information.
# nightly.sh - pull a copy of 2acoin-master 
# and put it inside a timestamped folder.
# rock made this

sourcefolder=~/Source/2acoin-dev-build-$(date +%F)

mkdir ~/Source
mkdir ~/Binaries
echo -e "\n MOVING TO ~/Source"
cd ~/Source/
ls -al

echo -e "\n CLONING 2ACoin in $sourcefolder"
git clone https://github.com/2acoin/2acoin $sourcefolder
cd $sourcefolder
mkdir -p $sourcefolder/build && cd $sourcefolder/build
ls -al

echo -e "\n BUILDING 2ACoin"
cmake .. && make -j8 # remove -j8 for single core
cd src
ls -al

echo -e "\n\n COMPRESSING BINARIES"
zip 2acoin-dev-bin-$(date +%F)-linux.zip miner poolwallet simplewallet 2ACoind walletd
mv *.zip ~/Binaries/
cd ~/Binaries
ls -al

echo -e "\n COMPLETE!"
