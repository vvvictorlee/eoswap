* [ ] mongod --dbpath /usr/local/mongodb/mongodb-macos-x86_64-5.0.1/data --logpath /usr/local/mongodb/mongodb-macos-x86_64-5.0.1/log/mongod.log
  cd /Users/lisheng/mygit/vlbos/orderbook-backend
  yarn dev

cd /Users/lisheng/mygit/vlbos/pacificstore-node
RUST_LOG=debug RUST_BACKTRACE=1 cargo run -- --dev --tmp  -lerror,runtime::contracts=debug


RUSTFLAGS='--cfg procmacro2_semver_exempt'

-lerror,runtime::contracts=debug