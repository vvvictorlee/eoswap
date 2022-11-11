#!/usr/bin/env bash

# set -eu

case "$1" in
"five")  cargo  contract build --manifest-path five_degrees/Cargo.toml ;;
"erc1155") cargo  contract build --manifest-path erc1155/Cargo.toml ;;
*) cargo  contract build --manifest-path erc1155/Cargo.toml && \
cargo  contract build --manifest-path five_degrees/Cargo.toml ;;
esac

