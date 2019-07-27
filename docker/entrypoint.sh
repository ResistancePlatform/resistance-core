#!/usr/bin/env bash
set -eo pipefail

export RES_HOME=/home/resuser
export RES_BIN=/home/resuser/resistance

# Init steps when running as root
if [ "$(id -u)" = "0" ]; then
    mkdir -p ${RES_HOME}/.resistance
    mkdir -p ${RES_HOME}/.resistance-params
    mkdir -p ${RES_BIN}
    cp /fetch-params.sh ${RES_BIN}
    cp /resistanced ${RES_BIN}
    cp /resistance-cli ${RES_BIN}
    chown resuser:resuser -R $RES_HOME/{.resistance,.resistance-params,resistance}
    exec gosu resuser "$BASH_SOURCE" "$@"
fi

# Fetch the parameter files, no-op if they're already in place
cd ${RES_BIN}
./fetch-params.sh

# Init resistance.conf if it doesn't exist
if [[ ! -f ${RES_HOME}/.resistance/resistance.conf ]]; then
    touch ${RES_HOME}/.resistance/resistance.conf
    # Testnet configs
    echo "testnet=1" >> ${RES_HOME}/.resistance/resistance.conf
    echo "rpcport=18132" >> ${RES_HOME}/.resistance/resistance.conf
    echo "rpcport=18133" >> ${RES_HOME}/.resistance/resistance.conf

    # Mainnet configs
    # echo "rpcport=8132" >> ${RES_HOME}/.resistance/resistance.conf
    # echo "rpcport=8133" >> ${RES_HOME}/.resistance/resistance.conf

    echo "rpcallowip=127.0.0.1" >> ${RES_HOME}/.resistance/resistance.conf
    echo "rpcworkqueue=512" >> ${RES_HOME}/.resistance/resistance.conf
    echo "server=1" >> ${RES_HOME}/.resistance/resistance.conf
    echo "listen=1" >> ${RES_HOME}/.resistance/resistance.conf
    echo "txindex=1" >> ${RES_HOME}/.resistance/resistance.conf
    echo "logtimestamps=1" >> ${RES_HOME}/.resistance/resistance.conf
    echo "rpcuser=resnode" >> ${RES_HOME}/.resistance/resistance.conf
    pw=$(head -c 32 /dev/urandom | sha256sum | head -c 64)
    test ${#pw} -eq 64
    echo "rpcpassword=${pw}" >> ${RES_HOME}/.resistance/resistance.conf
fi

# Start the resistance daemon
exec ./resistanced
