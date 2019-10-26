# Resistance 2.1.0-rc1

## What is Resistance?

[Resistance](https://resistance.io/) is fork of Zcash.
Based on Bitcoin's code, it intends to offer a far higher standard of privacy
through a sophisticated zero-knowledge proving scheme that preserves
confidentiality of transaction metadata.

This software is the Resistance client. It downloads and stores the entire history
of Resistance transactions; depending on the speed of your computer and network
connection, the synchronization process could take a day or more once the
blockchain has reached a significant size.

## Security Warnings

See important security warnings on Zcash's
[Security Information page](https://z.cash/support/security/).

**Resistance is experimental and a work in progress.** Use it at your own risk.

## Deprecation Policy

This release is considered deprecated 16 weeks after the release day. There
is an automatic deprecation shutdown feature which will halt the node some
time after this 16-week period. The automatic feature is based on block
height.

## Getting Started

We have a guide for joining the main Resistance network:
https://github.com/ResistancePlatform/resistance-core/wiki/1.0-User-Guide

### Need Help?

(We'll list our user community resources such as wiki, forum, etc. here.)

Participation in the Resistance project is subject to a
[Code of Conduct](code_of_conduct.md).

## Building

Initial development release of the Resistance tools.

Instructions to compile this yourself:

### Linux

Dependencies (run as **root**):

```
sudo apt-get update
sudo apt-get install build-essential automake libtool pkg-config curl
```

Compile (do **not** run as **root**)

```
./resutil/build.sh
```

You must first create a configuration file. You can create an empty configuration by running:

```
mkdir ~/.resistance
touch ~/.resistance/resistance.conf
```

Then you need to get the Resistance params using the following script (make sure you have `ipfs` and `wget` installed):

```
./resutil/fetch-params.sh
```

To start the blockchain daemon run:

```
./src/resistanced -regtest -daemon
```

**Note**: You may need to wait up to 20 seconds for the daemon to start before full the daemon is fully functional.

### macOS

Prerequisite:

- Homebrew: https://brew.sh/
- XCode Command Line Tools (make sure these are up to date): https://itunes.apple.com/us/app/xcode/id497799835?mt=12 and http://railsapps.github.io/xcode-command-line-tools.html

Dependencies:

```
brew update
brew upgrade
brew install autoconf autogen automake binutils protobuf coreutils wget curl
```

Build:

```
./resutil/build.sh
```

Configuration:

```
touch ~/Library/Application\ Support/Resistance/resistance.conf
./resutil/fetch-params.sh
```

Run Daemon

```
./src/resistanced -regtest
```

### Windows

**Note**: This must be cross-compiled from a Linux box. In this example, we will use Ubuntu 18.04.2 LTS (Bionic Beaver).

Upgrade your system:

```
sudo apt-get update
sudo apt-get upgrade
```

Install dependencies:

```
sudo apt-get install build-essential pkg-config libc6-dev m4 g++-multilib autoconf libtool ncurses-dev unzip git python python-zmq zlib1g-dev wget curl bsdmainutils automake
sudo apt-get install mingw-w64
git clone git@github.com:ResistancePlatform/resistance-core.git
cd resistance-core
```

Then run the following commands (**select the option containing POSIX when prompted**):

```
sudo update-alternatives --config x86_64-w64-mingw32-gcc
sudo update-alternatives --config x86_64-w64-mingw32-g++
```

Build the executable:

```
./resutil/build.sh
HOST=x86_64-w64-mingw32 ./resutil/build.sh -j$(nproc)
strip src/resistanced.exe src/resistance-cli.exe src/resistance-tx.exe
```

Configuration:

TODO

Run Daemon

```
./src/resistanced -regtest
```


### Build Troubleshooting

1. "This is taking forever to build.": You can speed up the build by using `./resutil/build.sh -j2`. Depending on your system, you can increase the value of `-j`, i.e. `-j8`.
2. "I make one small change to the source, and I have to rebuild everything?!": Nope! After you have built the first time, you can rebuild quickly by running `make` or `make -j8` (faster) in this repo's `src` directory.

*Note*: If you run into build problems, and then find a solution please don't keep the solution to yourself. It helps everyone if you add your solution to this troubleshooting section.
4. "I want to set an rpc password": You can do this by adding the following to the resistance.conf file

```
rpcuser=CHANGEME
rpcpassword=CHANGEME
```

## CLI Client

Then you can interact with the daemon locally using the cli tool:

```
./src/resistance-cli -regtest getinfo
./src/resistance-cli -regtest getblockchaininfo
```

You can even mine a block:

```
./src/resistance-cli -regtest generate 1
```

Or you can mine 101 blocks at once:

```
./src/resistance-cli -regtest generate 101
```

**Note**: 101 Confirmations have to occur before you can spend the reward from the first block. So you have to run the generate command until 100 blocks are there.

More info can be found here: https://bitcoin.org/en/developer-examples#regtest-mode


## Docker

**This is for testing only. It uses weak rpc username, password, and ip whitelist**. Those parameters must be changed in the Dockerfile before deploying to anything real.

To make things extremely simple, you can build and run the project using docker.

Once you have docker installed and running on your system:

1. Clone this repo
2. `cd` into this repo
3. Run `docker build -t "resistance" .`
4. Run `docker run -it -p 18432:18432 resistance /home/resistance/src/resistanced -regtest`
5. Now you can communicate with the RPC via:

```
curl -u test123:test123 --data-binary '{" jsonrpc": "1.0", "id":" curltest", "method": "getblockchaininfo", "params": [] }' -H 'content-type: text/plain;' http://127.0.0.1:18432/
```

## License

For license information see the file [COPYING](COPYING).
