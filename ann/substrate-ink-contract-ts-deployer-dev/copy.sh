#!/usr/bin/env bash

# set -eu
basepath=$(pwd)
srcpath=./utils
destpath=../sub-art-front-end/src/abi/
cp -f ${basepath}/${srcpath}/art.json ${basepath}/${destpath} 


