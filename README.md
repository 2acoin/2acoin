### About 2ACoin ![](./2acoin_logo.png)
2ACoin is an innovative crypto-currency that is designed/developed to protect American's Second Amendment Rights. 2ACoin provides the benefits of a crypto-currency utilizing the CryptoNote algorithm and also supports the NRA with annual crypto payments. 

#### Master Build Status
[![Build Status](https://travis-ci.org/2acoin/2acoin.svg?branch=master)](https://travis-ci.org/2acoin/2acoin) [![Build status](https://ci.appveyor.com/api/projects/status/github/2acoin/2acoin?branch=master&svg=true)](https://ci.appveyor.com/project/2acoin/2acoin)

#### Development Build Status
[![Build Status](https://travis-ci.org/2acoin/2acoin.svg?branch=development)](https://travis-ci.org/2acoin/2acoin) [![Build status](https://ci.appveyor.com/api/projects/status/github/2acoin/2acoin?branch=development&svg=true)](https://ci.appveyor.com/project/2acoin/2acoin)


### Installing

We offer binary images of the latest releases here: [https://github.com/2acoin/2acoin/releases/latest](https://github.com/2acoin/2acoin/releases/latest)

If you would like to compile yourself, read on.

### How To Compile

#### Build Optimization

The CMake build system will, by default, create optimized native builds for your particular system type when you build the software. Using this method, the binaries created provide a better experience and all together faster performance.

However, if you wish to create portable binaries that can be shared between systems, specify -DARCH=default in your CMake arguments during the build process. Note that portable binaries will have a noticable difference in performance than native binaries. For this reason, it is always best to build for your particular system if possible.

#### Linux

##### Prerequisites

You will need the following packages: [Boost](https://www.boost.org/), [OpenSSL](https://www.openssl.org/), cmake (3.8 or higher), make, and git.

You will also need either GCC/G++, or Clang.

If you are using GCC, you will need GCC-7.0 or higher.

If you are using Clang, you will need Clang 6.0 or higher. You will also need libstdc++\-6.0 or higher.

##### GCC setup, on Ubuntu

    sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y
    sudo apt-get update
    sudo apt-get install aptitude -y
    sudo aptitude install -y build-essential g++-8 gcc-8 git libboost-all-dev python-pip
    sudo pip install cmake
    export CC=gcc-8
    export CXX=g++-8
    git clone -b master --single-branch https://github.com/2acoin/2acoin
    cd 2acoin
    mkdir build
    cd build
    cmake ..
    make
    
The binaries will be in the `src` folder when you are complete.

    cd src
    ./2ACoind --version
    
##### Clang setup, on Ubuntu

    sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y
    wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -
    
You need to modify the below command for your version of ubuntu - see https://apt.llvm.org/

* Ubuntu 14.04 (Trusty)  
`sudo add-apt-repository "deb https://apt.llvm.org/trusty/ llvm-toolchain-trusty 6.0 main"`

* Ubuntu 16.04 (Xenial)  
`sudo add-apt-repository "deb https://apt.llvm.org/xenial/ llvm-toolchain-xenial 6.0 main"`

* Ubuntu 18.04 (Bionic)  
`sudo add-apt-repository "deb https://apt.llvm.org/bionic/ llvm-toolchain-bionic 6.0 main"`

Then perform the following;

    sudo apt-get update
    sudo apt-get install aptitude -y
    sudo aptitude install -y -o Aptitude::ProblemResolver::SolutionCost='100*canceled-actions,200*removals' build-essential clang-6.0 libstdc++-7-dev git libboost-all-dev python-pip
    sudo pip install cmake
    export CC=clang-6.0
    export CXX=clang++-6.0
    git clone -b master --single-branch https://github.com/2acoin/2acoin
    cd 2acoin
    mkdir build
    cd build
    cmake ..
    make
    
The binaries will be in the `src` folder when you are complete.

    cd src
    ./2ACoind --version
    
##### Generic Linux

Ensure you have the dependencies listed above.

If you want to use clang, ensure you set the environment variables `CC` and `CXX`.
See the Ubuntu instructions for an example.

    git clone -b master --single-branch https://github.com/2acoin/2acoin
    cd 2acoin
    mkdir build
    cd build
    cmake ..
    make
    
The binaries will be in the `src` folder when you are complete.

    cd src
    ./2ACoind --version
    
#### OSX/Apple, using Clang

##### Prerequisites

- Install XCode and Developer Tools.


##### Building

    which brew || /usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
    brew install --force cmake boost llvm@8 openssl
    brew link --force llvm@8
    ln -s /usr/local/opt/llvm@8 /usr/local/opt/llvm
    export CC=/usr/local/opt/llvm@8/bin/clang
    export CXX=/usr/local/opt/llvm@8/bin/clang++
    git clone -b master --single-branch https://github.com/2acoin/2acoin
    cd 2acoin
    mkdir build
    cd build
    cmake ..
    make
        
The binaries will be in the `src` folder when you are complete.

    cd src
    ./2ACoind --version
    
    
#### Windows

##### Prerequisites

You can build for 32-bit or 64-bit Windows. **If you're not sure, pick 64-bit.**

- Download the [Build Tools for Visual Studio 2019](https://visualstudio.microsoft.com/thank-you-downloading-visual-studio/?sku=BuildTools&rel=16) Installer
- When it opens up select **C++ build tools**, it automatically selects the needed parts
- Install Boost (1.69 works the latest is 1.70 and doesn't work). Select the appropriate version for your system:
	- [Boost 64-bit](https://bintray.com/boostorg/release/download_file?file_path=1.69.0%2Fbinaries%2Fboost_1_69_0-msvc-14.1-64.exe)
	- [Boost 32-bit](https://bintray.com/boostorg/release/download_file?file_path=1.69.0%2Fbinaries%2Fboost_1_69_0-msvc-14.1-32.exe)
- Install the latest full LTS version of OpenSSL (currently OpenSSL 1.1.0L). Select the appropriate version for your system:
	- [OpenSSL 64-bit](https://slproweb.com/download/Win64OpenSSL-1_1_0L.exe)
	- [OpenSSL 32-bit](https://slproweb.com/download/Win32OpenSSL-1_1_0L.exe)


##### Building

**For 64-bit:**

From the start menu, open 'x64 Native Tools Command Prompt for VS 2019'.

    cd <your_accoin_directory>
    mkdir build
    cd build
    cmake -G "Visual Studio 16 2019" -A x64 .. -DBOOST_ROOT=C:/local/boost_1_69_0
    MSBuild 2ACoin.sln /p:Configuration=Release /m or MSBuild src\cli.vcxproj /p:Configuration=Release /m

**For 32-bit:**

From the start menu, open 'x86 Native Tools Command Prompt for VS 2019'.

    cd <your_2acoin_directory>
    mkdir build
    cd build
    cmake -G "Visual Studio 16 2019" -A Win32 .. -DBOOST_ROOT=C:/local/boost_1_69_0
    MSBuild 2ACoin.sln /p:Configuration=Release /p:Platform=Win32 /m
    
The binaries will be in the src/Release folder when you are complete.

    cd src
    cd Release
    2ACoind.exe --version
    

    
#### Raspberry Pi 3 B+
The following images are known to work. Your operation system image **MUST** be 64 bit.

##### Known working images

- https://github.com/Crazyhead90/pi64/releases
- https://fedoraproject.org/wiki/Architectures/ARM/Raspberry_Pi#aarch64_supported_images_for_Raspberry_Pi_3
- https://archlinuxarm.org/platforms/armv8/broadcom/raspberry-pi-3

Once you have a 64 bit image installed, setup proceeds the same as any Linux distribution. Ensure you have at least 2GB of ram, or the build is likely to fail. You may need to setup swap space.

##### Building

    git clone -b master --single-branch https://github.com/2acoin/2acoin
    cd 2acoin
    mkdir build
    cd build
    cmake ..
    make
    
The binaries will be in the `src` folder when you're compile is done.

    cd src
    ./2ACoind --version
    
#### Thanks
Cryptonote Developers, Bytecoin Developers, Monero Developers, Forknote Project, 2ACoin Community

### Copypasta for license when editing files

Hi 2ACoin contributor, thanks for forking and sending back Pull Requests. Extensive docs about contributing are in the works or elsewhere. For now this is the bit we need to get into all the files we touch. Please add it to the top of the files, see [src/CryptoNoteConfig.h](https://github.com/2acoin/2acoin/blob/master/src/CryptoNoteConfig.h) for an example.

    
    // Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers  
    // Copyright (c) 2014-2018, The Monero Project  
    // Copyright (c) 2018-2019, The TurtleCoin Developers  
    // Copyright (c) 2018-2019, The 2ACoin Developers  
    // 
    // Please see the included LICENSE file for more information.
    
