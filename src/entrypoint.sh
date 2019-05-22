#!/usr/bin/env bash
set -e

if [ "$(id -u)" = "0" ]; then
    mkdir -p /home/resuser/.resistance
    mkdir -p /home/resuser/.resistance-params
    cp /resutil/fetch-params.sh /home/resuser
    chown resuser:resuser /home/resuser/.resistance
    chown resuser:resuser /home/resuser/.resistance-params
    chown resuser:resuser /home/resuser/fetch-params.sh
    exec gosu resuser "$BASH_SOURCE" "$@"
fi

# Fetch the parameter files, no-op if they're already in place
cd /home/resuser
./fetch-params.sh

# Init resistance.conf if it doesn't exist
if [[ ! -f /home/resuser/.resistance/resistance.conf ]]; then
    touch /home/resuser/.resistance/resistance.conf
    echo "testnet=1" >> /home/resuser/.resistance/resistance.conf
    echo "rpcuser=resnode" >> /home/resuser/.resistance/resistance.conf
    pw=$(pwgen 60)
    echo "rpcpassword=${pw}" >> /home/resuser/.resistance/resistance.conf
fi

# Start the resistance daemon
exec ./resistanced
