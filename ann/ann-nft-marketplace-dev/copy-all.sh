#!/usr/bin/env bash

# set -eu
basepath=$(pwd)
srcpath=./release
destpath=../sub-art-front-end/src/abi/
cp -f ${basepath}/${srcpath}/*.json ${basepath}/${destpath} 



