#!/usr/bin/env bash

VERSION=0.1
WORK_DIR=$(cd $(dirname $0); pwd)

function build_module() {
    m_name=$1
    m_dir=${WORK_DIR}/${m_name}
    echo "build module ${m_dir}"
    cd ${m_dir}
    cargo +nightly contract build
    if [ $? -ne 0 ];then
      echo "build module failed"
      exit 1
    fi
    echo "copy to ../release"
    cp -f ${m_dir}/target/ink/${m_name}.wasm ../release/${m_name}_v$VERSION.wasm
    cp -f ${m_dir}/target/ink/${m_name}.contract ../release/${m_name}_v$VERSION.contract
    cp -f ${m_dir}/target/ink/metadata.json ../release/${m_name}_v$VERSION.json
    cd -
}

# echo "clean release"
# rm -rf ${WORK_DIR}/release
# mkdir -p ${WORK_DIR}/release

# sub_auction sub_marketplace sub_bundle_marketplace sub_art_factory sub_nft_factory
# modules=(sub_contract_management sub_art_factory sub_art_factory_private sub_nft_factory sub_nft_factory_private)
# modules=(erc20 sub_contract_management sub_art_factory sub_art_factory_private sub_nft_factory sub_nft_factory_private sub_art_tradable sub_art_tradable_private sub_nft_tradable sub_nft_tradable_private)
modules=(sub_price_seed)
for module in ${modules[@]}; do
        build_module $module
done
