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
    chown resuser:resuser -R $RES_HOME/{.resistance,.resistance-params,resistance}
    exec gosu resuser "$BASH_SOURCE" "$@"
fi

# Fetch the parameter files, no-op if they're already in place
cd ${RES_BIN}
./fetch-params.sh

# Init resistance.conf if it doesn't exist
if [[ ! -f ${RES_HOME}/.resistance/resistance.conf ]]; then
    touch ${RES_HOME}/.resistance/resistance.conf
    echo "testnet=1" >> ${RES_HOME}/.resistance/resistance.conf
    echo "rpcuser=resnode" >> ${RES_HOME}/.resistance/resistance.conf
    pw=$(head -c 32 /dev/urandom | sha256sum | head -c 64)
    test ${#pw} -eq 64
    echo "rpcpassword=${pw}" >> ${RES_HOME}/.resistance/resistance.conf
fi

# Start the resistance daemon
exec ./resistanced
