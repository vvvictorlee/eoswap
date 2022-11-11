#!/usr/bin/env bash

# set -eu
basepath=$(pwd)
srcpath=./release
destpath=../sub-art-rpc-test/packages/abi
cp -f ${basepath}/${srcpath}/*.json ${basepath}/${destpath} 



