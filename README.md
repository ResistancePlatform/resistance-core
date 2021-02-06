# Resistance 2.1.0-4

## What is Resistance?

[Resistance](https://resistance.io/) Core is a fork of
[Zcash](https://github.com/zcash/zcash).
Based on Bitcoin's code, it intends to offer a far higher standard of privacy
through a sophisticated zero-knowledge proving scheme that preserves
confidentiality of transaction metadata. Some of the differences of Resistance
Core from Zcash are our use of a CPU-optimized Proof-of-Work scheme
([yespower](https://www.openwall.com/yespower/)), shorter block interval
(1 minute), and changes for integration with other Resistance components.

This software is the Resistance client. It downloads and stores the entire
history of Resistance transactions; depending on the speed of your computer and
network connection, the synchronization process can take a long while.

## Security Warnings

See important security warnings on Zcash's
[Security Information page](https://z.cash/support/security/).

**Resistance is experimental and a work in progress.** Use it at your own risk.

## Building

Instructions to compile this yourself:

### Linux

Dependencies (example commands are for Debian or Ubuntu, run them as **root**):

```
sudo apt-get update
sudo apt-get install build-essential automake libtool pkg-config curl wget
```

Compile (do **not** run as **root**)

```
./resutil/build.sh --disable-tests
```

Then you need to get the Resistance params using the following script:

```
./resutil/fetch-params.sh
```

You must create a configuration file. You can create an empty configuration by running:

```
mkdir ~/.resistance
touch ~/.resistance/resistance.conf
```

To start the blockchain daemon run:

```
./src/resistanced
```

**Note**: You may need to wait up to 20 seconds for the daemon to start before it is fully functional.

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
./resutil/build.sh --disable-tests
```

Configuration:

```
touch ~/Library/Application\ Support/Resistance/resistance.conf
./resutil/fetch-params.sh
```

Run Daemon

```
./src/resistanced
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
./resutil/build.sh --disable-tests
HOST=x86_64-w64-mingw32 ./resutil/build.sh --disable-tests
strip src/resistanced.exe src/resistance-cli.exe src/resistance-tx.exe
```

Configuration:

TODO

Run Daemon

```
./src/resistanced
```


### Build Troubleshooting

1. "This is taking forever to build.": You can speed up building for Linux and maybe for macOS by using `./resutil/build.sh --disable-tests -j2`. Depending on your system, you can increase the value of `-j`, i.e. `-j8`. On a fast machine with enough RAM and a fast network (as the build downloads some of its dependencies), build using `-j8` completes in under 10 minutes. Builds without a `-j` option may take 40 minutes or more, but need a lot less RAM. Unfortunately, the `-j` option is known to cause Windows builds to fail, so you'll have to avoid it with those.
2. "I used the `-j` option and my build failed.": This is often caused by running out of RAM. To avoid that, don't set the `-j` value to more than half the number of GB of RAM you have in the system (or VM) - e.g., to use `-j8` safely we recommend having 16 GB RAM or more (in the VM, if applicable).
3. "I make one small change to the source, and I have to rebuild everything?!": Nope! After you have built the first time, you can rebuild quickly by running `make` or `make -j8` (faster) in this repo's `src` directory.
4. "I want to set an rpc password": You can do this by adding the following to the resistance.conf file:

```
rpcuser=CHANGEME
rpcpassword=CHANGEME
```

*Note*: If you run into build problems, and then find a solution please don't keep the solution to yourself. It helps everyone if you add your solution to this troubleshooting section.

## CLI Client

Then you can interact with the daemon locally using the cli tool:

```
./src/resistance-cli getinfo
./src/resistance-cli getblockchaininfo
```

## License

For license information see the file [COPYING](COPYING).

## Code of Conduct

Participation in the Resistance project is subject to a
[Code of Conduct](code_of_conduct.md).
