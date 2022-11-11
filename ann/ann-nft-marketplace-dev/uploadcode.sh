#!/usr/bin/env bash

# set -eu

s="2022-10-03T06:09:50.781751Z  INFO cargo_contract::crate_metadata: Fetching cargo metadata for Cargo.toml
2022-10-03T06:09:50.883024Z  INFO contract_transcode::transcoder: No matching type in registry for path PathKey(["ink_env", "types", "Hash"]).    
2022-10-03T06:09:50.886804Z  INFO cargo_contract::cmd::extrinsics::upload: Contract code path: /Users/lisheng/mygit/rust0x0/sub-art-nft-marketplace/sub_token_registry/target/ink/sub_token_registry.wasm
2022-10-03T06:09:50.889629Z  INFO jsonrpsee_client_transport::ws: Connection established to target: Target { sockaddrs: [], host: "127.0.0.1", host_header: "127.0.0.1:9944", _mode: Plain, path_and_query: "/" }

       Event Balances ➜ Withdraw
         who: 5HGjWAeFDfFCWPsjFQdVV2Msvz2XtMktvgocEZcCj68kUMaw
         amount: 86339202
       Event Balances ➜ Reserved
         who: 5HGjWAeFDfFCWPsjFQdVV2Msvz2XtMktvgocEZcCj68kUMaw
         amount: 738045000000
       Event Contracts ➜ CodeStored
         code_hash: 0x3365d5841ab184c7cb21bba44db2530e759ba05c511e58a903a15861dfa534c5
       Event TransactionPayment ➜ TransactionFeePaid
         who: 5HGjWAeFDfFCWPsjFQdVV2Msvz2XtMktvgocEZcCj68kUMaw
         actual_fee: 86339202
         tip: 0
       Event System ➜ ExtrinsicSuccess
         dispatch_info: DispatchInfo { weight: 2348993000, class: Normal, pays_fee: Yes }

   Code hash 0x3365d5841ab184c7cb21bba44db2530e759ba05c511e58a903a15861dfa534c5"
echo ${s##*Code hash}

basepath=$(pwd)
# srcpath=./release
# destpath=../sub-art-front-end/src/abi/
# cp -f ${basepath}/${srcpath}/erc20_v0.1.json ${basepath}/${destpath}
# --url ws://127.0.0.1:9944"
modules=($(ls -d sub* erc20))
addresses=""
for module in ${modules[@]}; do
    m_dir=${basepath}/${module}
    echo "upload code ${m_dir}"
    cd ${m_dir}
    # echo $(pwd)
    # output=($("cargo contract upload --suri //Alice  --manifest-path "$m_dir"/Cargo.toml"))
    # cargo contract upload --suri //Alice  --manifest-path $m_dir"/Cargo.toml"
    # output=`cargo contract upload --suri //Alice  --manifest-path $m_dir"/Cargo.toml"`
    output='2022-10-03T06:09:50.781751Z  INFO cargo_contract::crate_metadata: Fetching cargo metadata for Cargo.toml
2022-10-03T06:09:50.883024Z  INFO contract_transcode::transcoder: No matching type in registry for path PathKey(["ink_env", "types", "Hash"]).    
2022-10-03T06:09:50.886804Z  INFO cargo_contract::cmd::extrinsics::upload: Contract code path: /Users/lisheng/mygit/rust0x0/sub-art-nft-marketplace/sub_token_registry/target/ink/sub_token_registry.wasm
2022-10-03T06:09:50.889629Z  INFO jsonrpsee_client_transport::ws: Connection established to target: Target { sockaddrs: [], host: "127.0.0.1", host_header: "127.0.0.1:9944", _mode: Plain, path_and_query: "/" }

       Event Balances ➜ Withdraw
         who: 5HGjWAeFDfFCWPsjFQdVV2Msvz2XtMktvgocEZcCj68kUMaw
         amount: 86339202
       Event Balances ➜ Reserved
         who: 5HGjWAeFDfFCWPsjFQdVV2Msvz2XtMktvgocEZcCj68kUMaw
         amount: 738045000000
       Event Contracts ➜ CodeStored
         code_hash: 0x3365d5841ab184c7cb21bba44db2530e759ba05c511e58a903a15861dfa534c5
       Event TransactionPayment ➜ TransactionFeePaid
         who: 5HGjWAeFDfFCWPsjFQdVV2Msvz2XtMktvgocEZcCj68kUMaw
         actual_fee: 86339202
         tip: 0
       Event System ➜ ExtrinsicSuccess
         dispatch_info: DispatchInfo { weight: 2348993000, class: Normal, pays_fee: Yes }

   Code hash 0x3365d5841ab184c7cb21bba44db2530e759ba05c511e58a903a15861dfa534c5
2022-10-03T06:09:50.902856Z  WARN jsonrpsee_core::client::async_client: Custom("[backend]: frontend dropped; terminate client")'
    # echo $output
    o=${output##*Code hash}
    # o='0x3365d5841ab184c7cb21bba44db2530e759ba05c511e58a903a15861dfa534c5 2022-10-03T10:31:03.004003Z  WARN jsonrpsee_core::client::async_client: Custom("[backend]: frontend dropped; terminate client")'
    oo=${o:0:66}
    addresses=$addresses'"'${module}'":"'$oo'",'
    cd -
done
if [ -n addresses ]; then
    echo $addresses
    addresses=${addresses%,*}
    echo '{"address":{'$addresses'},"exclude":["sub_contract_management","sub_art_tradable","sub_art_tradable_private","sub_nft_tradable","sub_nft_tradable_private"]}' >art.json
fi
