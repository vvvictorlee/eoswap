#!/usr/bin/env bash

# set -eu
basepath=$(pwd)
packagespath=./packages/abi
destpath=../sub-art-front-end/src/abi/
cp -f ${basepath}/${packagespath}/art.json ${basepath}/${destpath} 


