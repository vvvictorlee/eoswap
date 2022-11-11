// Copyright 2017-2021 @polkadot/api authors & contributors
// SPDX-License-Identifier: Apache-2.0

const config = require('@polkadot/dev/config/jest.cjs');

module.exports = {
testTimeout: 500000,
  ...config,
 verbose: true,

  moduleNameMapper: {
    
  },
  modulePathIgnorePatterns: [
    
  ],
  transformIgnorePatterns: ['/node_modules/(?!@polkadot|@babel/runtime/helpers/esm/)']
};
