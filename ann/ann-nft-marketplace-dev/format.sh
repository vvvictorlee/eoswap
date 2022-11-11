#!/usr/bin/env bash

VERSION=0.1
WORK_DIR=$(cd $(dirname $0); pwd)

function format_module() {
    m_name=$1
    m_dir=${WORK_DIR}/${m_name}
    echo "format module ${m_dir}"
    cd ${m_dir}
    cargo fmt
    if [ $? -ne 0 ];then
      echo "format module  ${m_dir} failed"
      exit 1
    fi
    cd -
}

modules=($(ls -d erc*))
for module in ${modules[@]}; do
        format_module $module
done

modules=($(ls -d sub*))
for module in ${modules[@]}; do
        format_module $module
done